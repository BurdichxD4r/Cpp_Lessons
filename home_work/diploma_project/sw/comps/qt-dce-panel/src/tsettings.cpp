#include "modempanel.h"
#include "tconsole.h"
#include "tstream.h"
#include "tsettings.h"
#include "eml.h"
#include "modemchart.h"
#include "adapt_speed.h"
#include "tadaptfreq.h"

#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QMetaType>
#include <QTextOption>
#include <QString>
#include <QDebug>
#include <QRandomGenerator>

//#define		DO_LOG
#ifdef DO_LOG
#include <stdio.h>
#endif

DceSettings::DceSettings(QWidget *parent) : QWidget(parent)
{
    qRegisterMetaType<connect_Pars_T>();
    message = new QList<int>();
    sended = new quint8[22];
    decoded = new QByteArray();
    mess_out= new QList<int>();
    head_in= new QList<int>();
    *head_in << 0 << 0 <<0;

    noSignal	= (short_complex*) malloc(SLOT_LEN_MAX*sizeof(short_complex));
    memset(noSignal, 0, SLOT_LEN_MAX*sizeof(short_complex));
    connectParams = new connect_Pars_T;

    rxFslot = &rxFslots[0];
    txFslot = &txFslots[0];

    timer = new QTimer(this);
    stat_timer = new QTimer(this);

    dcePlot = new ModemChart();
    dcePlot->setModemNumber(Number);
    dcePlot->setAttribute(Qt::WA_QuitOnClose, false);

    mdmConnect = new TConnect(this);

    initInterface();
    setCbActiveFreqIndex(frequencyNumber);
    setCbAllFreqIndex(frequencyNumberAll);

    setTextDefinitions();
   //qDebug() << "slot_var"<<variants[0].num<< variants[0].mode_type;
    //createVariantList(developmentPhase-1);
    shiftCompensation->setChecked(false);

    connect(cbStage, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::createVariantList);

    connect(cbDuplex, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setDuplex);
    connect(stat_timer, &QTimer::timeout, this, &DceSettings::onTick);

    connect(plotters, &QCheckBox::stateChanged, this, &DceSettings::showChart);
    connect(shiftCompensation, &QCheckBox::stateChanged,this, &DceSettings::shiftComp);
    connect(turnOn, &QCheckBox::stateChanged,this, &DceSettings::OnOffState);
    connect(pbSynchr,&QPushButton::clicked, this, &DceSettings::initSynchro);
    connect(pbConnect,&QPushButton::clicked, this, &DceSettings::startConnection);
    connect(pbBreak,&QPushButton::clicked, this, &DceSettings::breakConnection);
    connect(cbAuto50, &QCheckBox::stateChanged,this, &DceSettings::autoBreakdown);
    connect(cbAdapt_s, &QCheckBox::stateChanged,this, &DceSettings::speed_adapt_init);
    connect(cbAdapt_f, &QCheckBox::stateChanged,mdmConnect, &TConnect::init_adapt_f);
    connect(cbAdapt_f, &QCheckBox::stateChanged,this,&DceSettings::cb50);
    connect(mdmConnect, &TConnect::cbAdapt_f_turn,this, [this](bool state){
        cbAdapt_f->setChecked(state);
    });
    connect(mdmConnect, &TConnect::cbAdapt_s_turn,this, [this](bool state){
        cbAdapt_s->setChecked(state);
    });

    connect(mdmConnect, &TConnect::synchrStatus, this, &DceSettings::showStatus);
    connect(mdmConnect, &TConnect::connectStatus, this, &DceSettings::showChvm);
    connect(mdmConnect, &TConnect::connectMode, this, &DceSettings::setConnectMode);
    connect(mdmConnect, &TConnect::synchrFailed,  this, &DceSettings::initSynchro);

    connect(mdmConnect, &TConnect::setWorkingVariant, this, &DceSettings::set_cbVariantIndex);
    connect(mdmConnect,&TConnect::setNewRxSpeed, this,&DceSettings::setRxChanVariant);
    connect(mdmConnect,&TConnect::setNewTxSpeed, this,&DceSettings::setTxChanVariant);

    connect(cbSampleRate, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setNewSampleRate);
    connect(this, &DceSettings::sendProcentInfo, dcePlot, &ModemChart::setProcentInfo);

    connect(cbLoop, &QCheckBox::stateChanged,this, &DceSettings::loop_on_off);
    connect(cbLoop, &QCheckBox::stateChanged,this, &DceSettings::setLoopMode);
    connect(cbBroadCast, &QCheckBox::stateChanged,this, &DceSettings::sendData);
    connect(cbActiveFreq, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                               this, &DceSettings::setActiveFreqsNumber);
    connect(cbAllFreq, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                               this, &DceSettings::setAllFreqsNumber);

    cbStage->setCurrentIndex(developmentPhase-1);

    cbConnectVariant->setCurrentIndex(18);
    cbDuplex->setCurrentIndex(1);
    cbSynchrNumber->setCurrentIndex(1);
    cbConnectNumber->setCurrentIndex(1);

    setTestMode(false);

}

DceSettings::~DceSettings()
{
     delete timer;
     delete stat_timer;
     deleteLogger(tlog);
     delete mdmConnect;
     delete dcePlot;
     delete adapt_v;
     delete connectParams;
}

void DceSettings::setLoopMode(bool state)
{
    loopMode=state;
    mdmConnect->setDataMode(state);
}

void DceSettings::set_turnOn_State(bool state)
{
    turnOn->setChecked(state);
}

void DceSettings::set_turnOn_Plots(bool state)
{
    plotters->setChecked(state);
}

void DceSettings::setTestMode(bool state)
{
    if (!isActive) return;
    if (state){
        //mdmConnect->init_mdm_time();
        connect(timer, &QTimer::timeout, this, &DceSettings::testFunctions);
        connect(startTest, &QCheckBox::stateChanged,this, &DceSettings::startTimer);
        connect(berMe, &QCheckBox::stateChanged,this, &DceSettings::berMeInit);
        // получим работу по старой схеме ( с кнопкой Тест запись/чтение файлов и расчет BER)
    }else{
        disconnect(timer, &QTimer::timeout, this, &DceSettings::testFunctions);
        disconnect(startTest, &QCheckBox::stateChanged,this, &DceSettings::startTimer);
        disconnect(berMe, &QCheckBox::stateChanged,this, &DceSettings::berMeInit);
    }
    startTest->setEnabled(state);
    startTest->setChecked(false);
    berMe->setEnabled(state);
    berMe->setChecked(false);
    turnOn->setEnabled(!state);
}

int  DceSettings::getModemNumber()    // возвращает номер модема
{
    return Number;
}

void  DceSettings::setModemNumber(int num) // присваивает модему номер
{
    Number = num;
    adapt_v = new TAdaptSpeed(this,Number,variants);
    mdmConnect->setAdapt(adapt_v);
    connect(adapt_v, &TAdaptSpeed::procentDecoded, this, &DceSettings::fixProcentValue);

    for (int ii=0;ii<32;ii++) {
        rxFslots[ii].setModemNumber(num);
        txFslots[ii].setModemNumber(num);
    }
    QString fname = QString("settings_log%1.txt").arg(Number);
    qDebug() << "tlog name="<< fname;
    #ifdef	DO_LOG
        tlog = newLogger(fname.toUtf8().data(),1,1);
    #else
        tlog = newLogger(fname.toUtf8().data(),1,0);
    #endif
     strToFile(tlog,1,tr("class DceSettings").toUtf8().data(),0);
     intToFile(tlog, 0, tr(" modem  ").toUtf8().data(),Number, 0, 10,' ',1);
     connect(cbVariants, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setVariant);
     connect(this, &DceSettings::freq_adapt_permition, this,&DceSettings::maybe_set_freq_adapt);
     initConnectParams();
     mdmConnect->setParams(Number,freqs_number,connectParams);
     best_f = new TBestFreq();
     best_f->setLogNuber(Number);
     connect(best_f, &TBestFreq::bestFrequency,  mdmConnect, &TConnect::setRecvNrchFreqNumber);//,Qt::DirectConnection);
     connect(mdmConnect, &TConnect::calcNrch,  best_f, &TBestFreq::bestFrequencyCalculate);//,Qt::DirectConnection);
}

void DceSettings::initInterface()
{

    QFont font1("Times New Roman", 12);
    font1.setBold(true);
    //font1.setStyleHint(QFont::Courier);
    createWidgets();

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(turnOn);
    layout1->addWidget(chvm);
    layout1->addStretch();
    layout1->addWidget(plotters);

    QHBoxLayout *layout1a = new QHBoxLayout;
    layout1a->addWidget(cbVariants);
    layout1a->addWidget(cbSampleRate);
    layout1a->addStretch();
    layout1a->addWidget(cbStage);

    QHBoxLayout *layoutt = new QHBoxLayout;
    layoutt->addWidget(timeBox);
    layoutt->setContentsMargins(0,0,0,0);
    timeGroup = new QGroupBox(this);
    timeGroup->setLayout(layoutt);

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->addLayout(layout1a);
    layout2->addWidget(infoBox);
    layout2->setContentsMargins(0,0,0,0);
    variantsGroup = new QGroupBox(this);
    variantsGroup->setLayout(layout2);

    QHBoxLayout *layout3a = new QHBoxLayout;
    layout3a->addWidget(cbAdapt_s);
    layout3a->addWidget(cbAdapt_f);
    layout3a->addWidget(shiftCompensation);
    layout3a->addStretch();
    layout3a->addWidget(cbAllFreq);
    layout3a->addWidget(cbActiveFreq);

    QHBoxLayout *layout3b = new QHBoxLayout;
    layout3b->addWidget(cbSynchrNumber);
    layout3b->addWidget(pbSynchr);
    layout3b->addStretch();
    layout3b->addWidget(cbConnectNumber);
    layout3b->addWidget(cbConnectVariant);
    layout3b->addWidget(cbDuplex);
    layout3b->addWidget(pbConnect);
    layout3b->addWidget(pbBreak);

    QHBoxLayout *layout3c = new QHBoxLayout;
    layout3c->addWidget(startTest);
    layout3c->addWidget(berMe);
    layout3c->addStretch();
    layout3c->addWidget(cbBroadCast);
    layout3c->addWidget(cbLoop);
    layout3c->addWidget(cbAuto50);

    QHBoxLayout *layout3s = new QHBoxLayout;
    layout3s->addWidget(lbSyncConnect);
    layout3s->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addLayout(layout1);
    layout->addWidget(timeGroup);
    layout->addWidget(variantsGroup);
    layout->addLayout(layout3b);
    layout->addLayout(layout3a);
    layout->addLayout(layout3c);
    layout->addLayout(layout3s);

    layout->addStretch();

    setLayout(layout);
}

void DceSettings::createWidgets()
{
    QString str;
    //QFont font("SansSerif", 10);
    QFont font("Verdana", 10);
    QFont font1("Times New Roman", 12);
    font1.setBold(true);
    font.setBold(true);
    font1.setStyleHint(QFont::Courier);
    int frameStyle = QFrame::Sunken | QFrame::Panel;

    turnOn = new QCheckBox(this);
    //turnOn->setFont(font1);
    turnOn->setChecked(false);
    turnOn->adjustSize();

    chvm = new QLabel(this);
    chvm->setFrameStyle(frameStyle);
    chvm->adjustSize();

    timeBox = new QLineEdit(this);
    timeBox->setReadOnly(true);
    //timeBox->setMaximumWidth(120);

    lbSyncConnect = new QLabel(this);
    //lbSyncConnect->setVisible(false);
    //lbSyncConnect->setFrameStyle(frameStyle);
    lbSyncConnect->adjustSize();

    infoBox = new QTextEdit(this);
    //timeBox->setFont(font1);
    infoBox->setReadOnly(true);
    infoBox->setMaximumHeight(130);
    infoBox->setMinimumHeight(130);

    cbStage = new QComboBox(this);
    for(int ii=1;ii<4;ii++){
        str = QString("Этап %1" ).arg(ii);
        cbStage->addItem(str);
    }
    str = QString("Всё" );
    cbStage->addItem(str);

    cbVariants = new QComboBox(this);
    cbVariants->setMinimumWidth(130);

    cbSampleRate = new QComboBox(this);
    cbSampleRate->addItem(QString("%1" ).arg(fd_80));
    cbSampleRate->addItem(QString("%1" ).arg(fd_96));
    cbSampleRate->addItem(QString("%1" ).arg(fd_128));
    cbSampleRate->addItem(QString("%1" ).arg(fd_192));

    cbBroadCast = new QCheckBox(this);
    //cbBroadCast->setFont(font1);
    cbBroadCast->setChecked(false);

    cbLoop = new QCheckBox(this);
    cbLoop->setEnabled(false);
    cbLoop->setChecked(false);

    startTest = new QCheckBox(this);
    berMe = new QCheckBox(this);
    shiftCompensation = new QCheckBox(this);

    plotters = new QCheckBox(this);
    plotters->setChecked(false);
    //plotters->setEnabled(false);

    cbAdapt_s = new QCheckBox(this);
    cbAdapt_s->setChecked(false);
    //cbAdapt_s->setEnabled(false);
    cbAdapt_f = new QCheckBox(this);
    cbAdapt_f->setChecked(false);
    //cbAdapt_f->setEnabled(false);
    cbAuto50 = new QCheckBox(this);
    cbAuto50->setChecked(false);

    cbSynchrNumber = new QComboBox(this);
    cbSynchrNumber->setDuplicatesEnabled(false);
    cbSynchrNumber->setInsertPolicy(QComboBox::InsertAlphabetically);
    cbSynchrNumber->setMaximumWidth(30);
    cbSynchrNumber->setMaximumHeight(18);

    cbConnectNumber = new QComboBox(this);
    cbConnectNumber->setDuplicatesEnabled(false);
    cbConnectNumber->setInsertPolicy(QComboBox::InsertAlphabetically);
    cbConnectNumber->setMaximumWidth(30);
    cbConnectNumber->setMaximumHeight(18);

    for(int ii=4;ii>0;ii--){
        appendToConnectionList(ii);
    }

    cbConnectVariant = new QComboBox(this);
    cbConnectVariant->setDuplicatesEnabled(false);
    cbConnectVariant->setInsertPolicy(QComboBox::InsertAlphabetically);
    cbConnectVariant->setMaximumWidth(45);
    cbConnectVariant->setMaximumHeight(18);
    for(int ii=0;ii<29;ii++){
        str.setNum(ii+1);
        cbConnectVariant->addItem(str);
    }

    cbActiveFreq = new QComboBox(this);
    cbActiveFreq->setMaximumWidth(50);
    cbActiveFreq->setMaximumHeight(18);
    for(int ii = 2;ii <= 16;ii++){
        str.setNum(ii*2);
        cbActiveFreq->addItem(str);
    }

    cbAllFreq = new QComboBox(this);
    cbAllFreq->setMaximumWidth(50);
    cbAllFreq->setMaximumHeight(18);
    for(int ii = 32;ii <= 256;ii++){
        str.setNum(ii);
        cbAllFreq->addItem(str);
    }

    cbDuplex = new QComboBox(this);
    cbDuplex->setMaximumWidth(45);
    cbDuplex->setMaximumHeight(18);
    str = QString("1/2" );
    cbDuplex->addItem(str);
    str = QString("Дуп" );
    cbDuplex->addItem(str);

    pbSynchr = new QPushButton(this);
    pbSynchr->setEnabled(true);
    pbSynchr->setMaximumWidth(35);
    pbSynchr->setMaximumHeight(20);

    pbConnect = new QPushButton(this);
    pbConnect->setEnabled(true);
    pbConnect->setMaximumWidth(35);
    pbConnect->setMaximumHeight(20);

    pbBreak = new QPushButton(this);
    pbBreak->setEnabled(false);
    pbBreak->setMaximumWidth(25);
    pbBreak->setMaximumHeight(20);
}

void DceSettings::appendToConnectionList(int num)
{
    QString str;
    str.setNum(num);
    //cbConnectNumber->insertItem(0,str);
    cbSynchrNumber->insertItem(0,str);
    cbConnectNumber->insertItem(0,str);
}

void DceSettings::setTextDefinitions()
{
    QString str;
    str.setNum(Number);
    turnOn->setToolTip(tr("Включение/выключение модема установкой/снятием флага"));
    turnOn->setText(tr("Вкл/Выкл"));
    chvm->setText(tr("ЧВМ=")+"0/0-0");
    chvm->setToolTip(tr("Частотно-временная матрица"));

    cbBroadCast->setToolTip(tr("Включение/выключение режима широковещательной передачи"));
    cbBroadCast->setText(tr("Brdcst"));
    cbAdapt_s->setToolTip(tr("Вкл/выкл адаптации по скорости"));
    cbAdapt_s->setText(tr("АдСк"));

    cbActiveFreq->setToolTip(tr("Число активных частот"));
    cbAllFreq->setToolTip(tr("Общее число частот ППРЧ"));

    cbAdapt_f->setToolTip(tr("Вкл/выкл адаптации по частоте"));
    cbAdapt_f->setText(tr("АдЧа"));
    cbAuto50->setToolTip("Вкл/выкл разрыв соединения после 50\nподряд не декодированных слотов");
    cbAuto50->setText(tr("50 "));
    cbLoop->setToolTip(tr("Включение/выключение режима шлейфа"));
    cbLoop->setText(tr("Шлейф"));
    cbStage->setToolTip(tr("Выбор этапа разработки"));
    cbSampleRate->setToolTip(tr("Выбор частоты дискретизации,Гц"));

    variantsGroup->setTitle(tr("Вариант сигнальной конструкции"));

    startTest->setToolTip(tr("Отладка канального уровня"));
    startTest->setText(tr("Тест"));
    berMe->setToolTip(tr("При установленном флаге ТЕСТ запускает измерение BER"));
    berMe->setText(tr("Изм."));
    shiftCompensation->setToolTip("Включение/выключение режима \n компенсации частотного сдвига");
    shiftCompensation->setText(tr("КЧС"));

    plotters->setToolTip(tr("Вкл/выкл отображение окна с графиками"));
    plotters->setText(tr("Графики"));

    timeBox->setToolTip(tr("текущее время модема"));
    timeGroup->setTitle(tr("Время, 128-битный счетчик"));

    cbSynchrNumber->setToolTip("Номер модема для синхронизации");
    pbSynchr->setToolTip(tr("Запуск процедуры синхронизации"));
    pbSynchr->setText(tr("Синх"));

    cbConnectNumber->setToolTip("Номер модема для соединения");
    pbConnect->setToolTip(tr("Передать запрос на соединение"));
    pbConnect->setText(tr("Соед"));
    pbBreak->setToolTip(tr("Передать признак разрыва соединения"));
    pbBreak->setText(tr("РС"));
    cbConnectVariant->setToolTip("Номер варианта для соединения");
    cbDuplex->setToolTip("Выбор дуплекс-полудуплекс");
}

void DceSettings::writeSettings()
{
    QString str;
    str.setNum(Number);
    //QSettings settings(QApplication::applicationDirPath()+QDir::separator()+"fhss.ini", QSettings::IniFormat);
    QSettings settings("inteltech","fhss-standalone");
    settings.setValue("DIRECTORY"+str+"/choose","");
}

void DceSettings::readSettings()
{
    QString str;
    str.setNum(Number);
    //QSettings settings(QApplication::applicationDirPath()+QDir::separator()+"fhss.ini", QSettings::IniFormat);
    QSettings settings("inteltech","fhss-standalone");
    //sendFilesDirDefault =settings.value("DIRECTORY"+str+"/choose","/").toString();
    //qDebug() << Number <<"sendFilesDirDefault= "<< sendFilesDirDefault;
}


QString DceSettings::showArray(quint8 *arr, int len)
{
    QString str,str1;
    for (int ii=0;ii<len;ii++) {
        str1 = QString(" 0x%1").arg(*(arr+ii), 2, 16,QLatin1Char('0'));
        str.append(str1);
    }
    return str;
}

QString DceSettings::showByteArray(QByteArray arr, int len)
{
    QString str,str1;
    for (int ii=0;ii<len;ii++) {
        str1 = QString(" 0x%1").arg(arr.at(ii), 2, 16,QLatin1Char('0'));
        str.append(str1);
    }
    return str;
}

void DceSettings::createVariantList(int var)
{
    QString str,str1;
    cbVariants->clear();
    stageVariants.clear();
    for(int ii=0;ii<VARS_NUMBER;ii++){
        if ((variants[ii].stage == var+1)||(var+1 == cbStage->count())){
            stageVariants.append(ii);
            str = QString(" %1  %2 %3 %4%5" ).arg(variants[ii].num).arg(variants[ii].freq_mode).arg(variants[ii].freq_band)
                    .arg(variants[ii].mode_type).arg(variants[ii].rate);
            cbVariants->addItem(str);
        }
    }
}

void DceSettings::setActiveFreqsNumber(const QString& str)
{
    bool ok;
    int num=str.toInt(&ok);
    if (ok){
        frequencyNumber=num;
        connectParams->startFreqNumber = num;
        if (adapt_f != nullptr)
           adapt_f->set_activeFreqNumber(num);
    }
}

void DceSettings::setAllFreqsNumber(const QString& str)
{
    bool ok;
    int num=str.toInt(&ok);
    if (ok){
        frequencyNumberAll=num;
        connectParams->allFreqNumber = num;
        emit allFreqsNumber(num);
        if (adapt_f != nullptr)
           adapt_f->set_allFreqNumber(num);
    }
     qDebug()<<"DceSettings::setAllFreqsNumber num="<<num<<ok;

}

void DceSettings::set_cbAllFreq(const QString& str)
{
    bool ok;
    int num=str.toInt(&ok);
    if (ok){
       setCbAllFreqIndex(num);
    }
    qDebug()<<"DceSettings::set_cbAllFreq num="<<num<<ok;
}

void DceSettings::set_cbActiveFreq(const QString& str)
{
    bool ok;
    int num=str.toInt(&ok);
    if (ok){
       setCbActiveFreqIndex(num);
    }
}

void DceSettings::setCbActiveFreqIndex(int def)
{
    cbActiveFreq->setCurrentIndex((def - 4)/2);
}

void DceSettings::setCbAllFreqIndex(int def)
{
    cbAllFreq->setCurrentIndex(def - 32);
}

void DceSettings::setNewSampleRate()
{
    bool ok;
    int sr = (cbSampleRate->currentText()).toInt(&ok);
    if (ok){
        if (variants[curVariant].freq_discr != sr){
            if ((sr == fd_192)&&(variants[curVariant].freq_band != fBand_9000 )){
                setSampleRateItem(curVariant);
            }else{
                variants[curVariant].freq_discr = sr;
                txFslot->setSlotVariant(&variants[curVariant]);
                rxFslot->setSlotVariant(&variants[curVariant]);

                emit pass_over_variant(&variants[curVariant]);
            }
        }
    }
}

void DceSettings::setSampleRateItem(int jj)
{
    int cbSampleRateIndex;
    switch( variants[jj].freq_discr )
    {
    case fd_96:
        cbSampleRateIndex = 1;
        break;
    case fd_128:
        cbSampleRateIndex = 2;
        break;
    case fd_192:
        cbSampleRateIndex = 3;
        break;
    default:
        cbSampleRateIndex = 0;
    }
    disconnect(cbSampleRate, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setNewSampleRate);
    cbSampleRate->setCurrentIndex(cbSampleRateIndex);
    connect(cbSampleRate, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setNewSampleRate);
}

void DceSettings::setVariant(int jj)
{
    if (jj < 0) return;
    isSynhro = false;
    clearBer();
    message->clear();
    if(stageVariants.isEmpty()) return;
    int ii = stageVariants.at(jj);
    setSampleRateItem(ii);
    curVariant = ii;
    if (variants[ii].num == 28) servMode=true;
    else servMode=false;

    setRxVariant(ii);
    setTxVariant(ii);
//    if(ii==18){
//        for (int kk=0;kk<8;kk++) {
//            setRxChanVariant(kk,21);
//            setTxChanVariant(kk,21);
//        }
//    }

    if (variants[ii].freq_mode == tr("ФРЧ")){
        convertVariantToParams(ii,connectParams);
        setFrchParams();
        emit freq_adapt_permition(false);
    }else if(variants[ii].freq_mode == tr("НРЧ")){
        mdmConnect->setLoop_fixFrequecyMode(true);
        emit freq_adapt_permition(false);
    }else{
        mdmConnect->setLoop_fixFrequecyMode(false);
        emit freq_adapt_permition(true);
    }
    if (adapt_v != nullptr) adapt_v->initAdaptation(jj);
}

void DceSettings::setVariantIndex(int jj)
{
    disconnect(cbVariants, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setVariant);
    cbVariants->setCurrentIndex(jj);
    connect(cbVariants, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setVariant);
    setVariant(jj);
}

void DceSettings::setSamplingRate(int vars,int sample)
{
    variants[vars].freq_discr = sample;
}

void DceSettings::setTxVariant(int ii)  // установка варианта на передачу
{
    for (int jj=0;jj<32;jj++) {
        connectParams->txVariant[jj] = ii;
        txFslots[jj].setSlotVariant(&variants[ii]);
    }

}

void DceSettings::setRxVariant(int ii)  // установка варианта на прием
{
    for (int jj=0;jj<32;jj++) {
        connectParams->rxVariant[jj] = ii;
        rxFslots[jj].setSlotVariant(&variants[ii]);
    }

    double rsmax=static_cast<double>((rxFslot->getVarslot()->N - rxFslot->getVarslot()->K))/2;
    dcePlot->setRSrange(-1,rsmax);
    dcePlot->setBFrange(0,rxFslot->getBestFrameShifts());

    emit pass_over_variant(&variants[ii]);
    showVariantsInfo(ii);
}

void DceSettings::setTxChanVariant(int ch,int ii)
{
    intToFile(tlog, 0, tr(" setTx chan_num= ").toUtf8().data(),ch, 0, 10,' ',0);
    intToFile(tlog, 0, tr(" to variant= ").toUtf8().data(),ii, 0, 10,' ',1);
    txFslots[ch].setSlotVariant(&variants[ii]);
    connectParams->txVariant[ch] = ii;
}

void DceSettings::setRxChanVariant(int ch,int ii)
{
    intToFile(tlog, 0, tr(" chan_num= ").toUtf8().data(),ch, 0, 10,' ',0);
    intToFile(tlog, 0, tr(" rx to variant_num= ").toUtf8().data(),ii, 0, 10,' ',1);
    rxFslots[ch].setSlotVariant(&variants[ii]);
    connectParams->rxVariant[ch] = ii;
}

void DceSettings::convertVariantToParams(int var,connect_Pars_T* par)
{
    par->speed = variants[var].info_bits*1000/variants[var].slot_time;
    switch(variants[var].freq_band){
    case fBand_4500:
        par->band = 1;
        break;
    case fBand_9000:
        par->band = 2;
        break;
    default:
        par->band = 0;
        break;
    }
    int speed = variants[var].info_bits*1000/variants[var].slot_time;
    switch (speed){
    case 600:
        par->speed = 1;
        break;
    case 1200:
        par->speed = 2;
        break;
    case 1800:
        par->speed = 3;
        break;
    case 2400:
        par->speed = 4;
        break;
    case 3600:
        par->speed = 5;
        break;
    case 4800:
        par->speed = 6;
        break;
    case 9600:
        par->speed = 7;
        break;
    case 19200:
        par->speed = 8;
        break;
    default:
        par->speed = 0;
        break;
    }
    if (variants[var].freq_mode == tr("ФРЧ")) par->switch_mode = 0;
    else if (variants[var].freq_mode == tr("НРЧ")) par->switch_mode = 1;
    else par->switch_mode = 2;
    if(variants[var].f_change_time > 3) par->rs1 = 1;
    else par->rs1 =0;
    if (par->switch_mode==1) mdmConnect->setConnectTime(CONNECT_TIME_NRCH);
    else mdmConnect->setConnectTime(CONNECT_TIME_NO_NRCH);
}

void DceSettings::setVariantParams(int var)
{
    convertVariantToParams(var,connectParams);
}

void DceSettings::setFrchParams()
{
    if ((isActive)&&(connectParams->switch_mode==0)){
        mdmConnect->setFrchConnect();
    }
}

void DceSettings::setDuplex(int num)
{
    connectParams->duplex = num;
    qDebug() <<Number<< " duplex="<<connectParams->duplex;
}

 void DceSettings::showVariantsInfo(int ii)
 {
     QString str;
     infoBox->clear();
     str = QString("Вариант %1" ).arg(variants[ii].num);
     infoBox->append(str);
     str = QString("Режим %1" ).arg(variants[ii].freq_mode);
     infoBox->append(str);
     str = QString("Дл-ть слота %2мс" ).arg(variants[ii].slot_time);
     infoBox->append(str);
     if (variants[ii].freq_mode == tr("ППРЧ")){
         str = QString("Смена РЧ <= %2мс" ).arg(variants[ii].f_change_time);
         infoBox->append(str);
     }
     str = QString("Полоса %1 Гц" ).arg(variants[ii].freq_band);
     infoBox->append(str);
     str = QString("Модуляция %1 %2" ).arg(variants[ii].mode_type).arg(variants[ii].rate);
     infoBox->append(str);
     str = QString("Число подканалов %1" ).arg(variants[ii].subchan_number);
     infoBox->append(str);
     str = QString("Посылок в слоте %1" ).arg(variants[ii].pack_number);
     infoBox->append(str);
     str = QString("Информац.скорость %1 бит/с" ).arg(variants[ii].info_bits*1000/variants[ii].slot_time);
     infoBox->append(str);


 }

 void DceSettings::testFunctions()
 {
//     if (!readFileMode){
//         modemChannelDemod();
//     }else{
//         fromFileToDemod();
//     }
     if (readFileMode){
         fromFileToDemod();
     }
 }

void DceSettings::startTimer(int state)
{
    if (state){
        if (timerCycle == -1) {
            timer->setSingleShot(true);
            timer->start();
        } else{
            timer->start(timerCycle);
        }
        byteIndex=0;
        emit timerIsStarted(timerCycle);
    }else{
        timer->stop();
    }
    ber.varnum = 0;
    emit changeSnr(-1);
}

void DceSettings::stopGoTimer(bool mode)
{
    processSuspended = mode;
    if (berMe->isChecked()){
        if (processSuspended){
            ber.vartext = cbVariants->currentText();
            //ber.varnum = cbVariants->currentIndex();
            ber.snr = snrIndex;
            //qDebug() << "QQQ snrIndex="<< snrIndex;
            emit fix_current_test(false, &ber);
            emit changeSnr(++snrIndex);
            if (snrIndex > snr_index_max){
                snrIndex = 0;
                changeShift(SHIFTINGS > 0);
            }
            clearBer();
        }
    }else{
         if (!mode) emit timerIsStarted(timerCycle);
    }
}

void DceSettings::dataOutputMode(bool mode)
{
    messageOutput = mode;
}

void DceSettings::setTimerCycle(int index)
{
    switch (index) {
    case 0:
        timerCycle = -1;
        break;
    case 1:
        timerCycle = 0;
        break;
    case 2:
        timerCycle = 50;
        break;
    case 3:
        timerCycle = 200;
        break;
    case 4:
        timerCycle = 1000;
        break;
    default:
        timerCycle = 0;
        break;
    }
    if (timerCycle == -1){
        timer->setSingleShot(true);
    }else{
        timer->setSingleShot(false);
        timer->setInterval(timerCycle);
    }
    //qDebug() << "setTimerCycle timerCicle="<<timerCycle;
}

void DceSettings::ber_evaluate(QList<int>* sendMes, QList<int>* recvMes)
{
    int recvLength = recvMes->count();
    for (int ii=0;ii<sendMes->count();ii++) {
        ber.send += 8;
        if (ii < recvLength){
            ber.recv += 8;
            ber.error += bit_difference(sendMes->at(ii), recvMes->at(ii));
            if (sendMes->at(ii) != recvMes->at(ii)) ber.byte_error++;
        }else{
            ber.error += bit_difference(sendMes->at(ii), 0);
            ber.byte_error++;
        }
    }
    if (berMe->isChecked()){
        if ((ber.recv > ber_measure)){
            emit fix_current_test(true, &ber);
        }
    }
}

 int  DceSettings::bit_difference(int num1, int num2)
 {
     int res=0;
     res	= Errors[num1 ^ num2];
     return res;
 }

 void DceSettings::onTick()
 {
     if (!processSuspended){
         ber.vartext = cbVariants->currentText();
         emit send_ber(&ber);
         //qDebug() << "onTick ber="<< ber.send << ber.recv << ber.error;
         show_modem_time();
         QString msg;
         quint32 res = mdmConnect->getDoneProcent(msg);
         if (res > 0) lbSyncConnect->setText(msg);
         showDecodProcent();
         //mdmConnect->msg_to_Status();
     }
 }

 void DceSettings::showChvm(quint16 addr1,quint16 addr2,quint32 tChan,quint32 rChan)
 {
     QString sz,sz1,sz2,sz3;
     chvm->setText(tr("ЧВМ=") + sz.setNum(addr1) + "/"+sz1.setNum(addr2)+
                                       "-" +sz2.setNum(tChan)+ "/"+sz3.setNum(rChan));
     if((addr1==0)&&(addr2==0)) {
             adapt_v->active = false;
             cbVariants->setCurrentIndex(27);
             connectParams->search_nrch_mode = true;
             best_f->setStatisticMode(false);
             pbConnect->setEnabled(true);
             pbBreak->setEnabled(false);
             cbAdapt_s->setChecked(false);
             //cbAdapt_s->setEnabled(false);
             cbAdapt_f->setChecked(false);
             //cbAdapt_f->setEnabled(false);
             initConnectParams();
     }
     emit transmitConnectState(addr1,addr2);
 }

 void DceSettings::setConnectMode(connect_Pars_T* pars)
 {
     int var=27;
     quint8 band1=pars->band;
     quint16 speed1=pars->speed;
     quint8 switch_mode1=pars->switch_mode;
     quint8 rs1 = pars->rs1;
     for (int ii=0;ii<VARS_NUMBER;ii++) {
         convertVariantToParams(ii,pars);
         if ((band1 == pars->band)&&
             (speed1==pars->speed)&&
             (switch_mode1==pars->switch_mode)&&
             (rs1==pars->rs1))
         {
             var = ii;
             break;
         }
     }
     intToFile(tlog, 1, tr(" variant_num= ").toUtf8().data(),var+1, 0, 10,' ',1);
     cbVariants->setCurrentIndex(var);
     cbDuplex->setCurrentIndex(pars->duplex);

     connectParams->txPspChannel = pars->chan_number[0];
     connectParams->rxPspChannel = pars->chan_number[1];


     setVariant(var);
     //if (adapt_v != nullptr) adapt_v->initAdaptation(var);
     pbConnect->setEnabled(false);
     pbBreak->setEnabled(true);
     cbAdapt_s->setEnabled(true);
     if (variants[var].slot_time == 50)
        cbAdapt_f->setEnabled(true);
 }

 void DceSettings::showStatus(QString msg)
 {
         lbSyncConnect->setText(msg);
 }

 void DceSettings::clearBer()
 {
     ber=  {0,0,0,0,0,0,"",0,0,-2,0};
 }

 short_complex* DceSettings::getOutput()
 {
     if (!silent_mode){
         return txFslot->getOutput();
     }
     else return noSignal;
 }

int DceSettings::getSlotLength()
{
    return rxFslot->getSlotLength();
}

int DceSettings::getSlotMsgLen()
{
    return rxFslot->getSlotMessageLength();
}

int DceSettings::getSkkNumber()
{
    return variants[curVariant].num;
}

 void DceSettings::berMeInit(int state)
 {
     if (state){
         cbVariants->setCurrentIndex(0);
         snrIndex = 0;
         emit changeSnr(snrIndex);
     }
 }

 void DceSettings::changeVariant()
 {
     int index = cbVariants->currentIndex()+1;
     if (index < cbVariants->count()){
         cbVariants->setCurrentIndex(index);
     }else{
         berMe->setChecked(false);
         startTest->setChecked(false);
     }
 }

 void DceSettings::changeShift(bool change)
 {
     if (change){
         if (!rxFslot->getShiftingState())
         {
             changeVariant();
         }
     }else{
         changeVariant();
     }
 }

 void DceSettings::showChart(int sh)
 {
     QString str;
     str.setNum(Number);
     if(sh){
        dcePlot->setWindowTitle(tr("Графики модема ")+str);
        dcePlot->setModemNumber(Number);
        dcePlot->setWindowFlags(Qt::WindowStaysOnTopHint);
        dcePlot->show();
        //dcePlot->activateWindow();
     }else{
        dcePlot->close();
        //qDebug() <<"atn 1-4=" << atan2f(1,1)<<atan2f(1,-1)<<atan2f(-1,-1)<<atan2f(-1,1);
     }
 }

 void	 DceSettings::sendPlotData()
 {
     if  (!plotters->isChecked()) return;
     QVector<std::complex<float>> cdVals;
     QList <fft_complex>* sozv = rxFslot->getSlotSpectrum();
     //the_total_spectrum_of_the_slot(sozv);
     cdVals = sozv->toVector();
     std::complex<float> z(1.1852260087141653,0.18772135804827703);  // 2 Гц
     cdVals.append(z);
     std::complex<float> z1(1.1668439044772119,0.2801344366270865);  // 3 Гц
     cdVals.append(z1);
     int rxChanNum = chanNumber_recv();
//     intToFile(tlog, 1, tr(" modnumber= ").toUtf8().data(),Number, 0, 10,' ',0);
//     intToFile(tlog, 0, tr(" plot ch_recv= ").toUtf8().data(),rxChanNum, 0, 10,' ',1);
     dcePlot->addData(rxFslot->getBestFrame(),rxFslot->getDoppler() ,rxFslot->getSNR(), rxFslot->getRSerr(), rxChanNum, cdVals);

    // qDebug() << "shift="<< shift << " snr=" <<snr;
 }

 QList <fft_complex>* DceSettings::getSlotSpectr()
 {
     return rxFslot->getSlotSpectrum();
 }
 float DceSettings::getSlotSNR()
 {
     return rxFslot->getSNR();
 }

 void DceSettings::shiftComp(int state)
 {
     rxFslot->setCompensationMode(state);
 }

 bool DceSettings::setInputSignal(QVector<int_complex>*signal)
 {
     bool res = false;
     if (mdmConnect->rcv()){
         rxFslot->setDoubleSlotInput(signal);
         res = true;
     }
     return res;
 }

void DceSettings::setInputSignalTest(QVector<int_complex>*signal)
 {
     rxFslot->setDoubleSlotInput(signal);
 }

 void DceSettings::setReadFileMode(bool state)
 {
     readFileMode=state;
     isSynhro = false;
 }

 void DceSettings::modemChannelDemod()
 {
     if (!processSuspended){
         // формирование тестовых  символов для слота
         QList<int> mess;
         int len = rxFslot->getSlotMessageLength();
         for (int ii=0;ii<len;ii++) {
             mess.append(byteIndex);
             if(!message->isEmpty()) byteIndex++;
         }
         rxFslot->setSlotMessage(&mess);
         if (servMode) silent_mode = true;
         else silent_mode = false;
         emit to_work_the_channel( getOutput(),rxFslot->getSlotLength());
         if (writeMode) {
             emit slotMessageShow("send Message:",message);
             ber.send += message->length() << 3;
         }
         if(isSynhro && !writeMode){
             QList<int>* mes= rxFslot->getSlotMessage();
             if (messageOutput){
                 emit slotMessageShow("send Message:",message);
                 if (!mes->isEmpty()) emit slotMessageShow("recv Message:",mes);
             }
             ber.send += message->length() << 3;
             bit_errors(mes);
             sendPlotData();
         }
         if(!message->isEmpty()){
             isSynhro=true;
         }
         message->clear();
         message->append(mess);
     }

     if (timerCycle == -1) {
         emit send_ber(&ber);
         startTest->setChecked(false);
     }
 }

 bool DceSettings::fromFileToDemod()
 {
     if (readFileMode){
         if (!processSuspended){
             int len = rxFslot->getSlotLength();
             emit anotherSlotFromFile_Please(len);
             if(isSynhro){
                 QList<int>* mes= rxFslot->getSlotMessage();
                 if (mes->isEmpty()) {
                     for (int ii=0;ii<variants[curVariant].K;ii++) {
                         mes->append(0);
                     }
                 }
                 if (messageOutput){
                     if (!mes->isEmpty()) emit slotMessageShow("recv Message:",mes);
                 }
                 bit_errors(mes);
                 sendPlotData();
             }
             if(!message->isEmpty()){
                 isSynhro=true;
             }
             message->clear();
             message->append(1);
         }
     }
     return readFileMode;
 }

 void DceSettings::bit_errors(QList<int> *recvMes)
 {
     quint32   err_bytes, err_bits;
     quint32 synchroLength=6;  // количество правильных символов для установления синхронизации

     if(ber.rxSync == 1)									// Счетчик синхронизирован
     {
         //Счетчик принятых данных увеличиваем когда засинхронизировались
         ber.recv	= ber.recv + (recvMes->length() << 3);
         err_bytes=0;
         for(int ii=0; ii < recvMes->length(); ii++, ber.rxCnt++)
         {
             err_bits	= static_cast<quint32>(Errors[recvMes->at(ii) ^ ber.rxCnt]);
             if( err_bits > 0 )
             {
                 err_bytes++;
             }
             ber.error	+= err_bits;
         }
         ber.byte_error += err_bytes;
         if( err_bytes > synchroLength ) ber.rxSync	= 0;
     }
     else													// Счетчик рассинхронизирован
     {
         err_bytes=0;
         for(int ii=1; ii < recvMes->length(); ii++)
         {
             if( recvMes->at(ii) == recvMes->at(ii-1) + 1 )
                 err_bytes++;
         }
         if( err_bytes > synchroLength )
         {
             ber.rxSync++;
             for(int ii=2; ii < recvMes->length(); ii++)
             {
                 if( recvMes->at(ii-1) == ((recvMes->at(ii-2) + recvMes->at(ii)) >> 1) )
                 {
                     ber.rxCnt = quint8(recvMes->last()+1);
                     //qDebug()<< "BER is Synchro" << ber.rxSync<<"ber.rxCnt="<<ber.rxCnt;
                     break;
                 }
             }
         }
     }
     if (berMe->isChecked()){
         if ((ber.recv > ber_measure)){
             emit fix_current_test(true, &ber);
         }
     }
 }

 void DceSettings::setWriteMode(bool state)
 {
     writeMode=state;
 }

 void DceSettings::the_total_spectrum_of_the_slot(QList <fft_complex>* sozv)
 {
    int subNum = variants[curVariant].subchan_number;
    QVector<float> amplSpectrum(subNum, 0);
    for (int ii=0;ii <sozv->count() ;ii++) {
    //for (int ii=0;ii <subNum ;ii++) {
           int ns = ii % subNum;
           amplSpectrum[ns] += abs(sozv->at(ii));
    }
    floatToFile(tlog,0,tr("").toUtf8().data(),amplSpectrum[0],10,2,'f',0);
    for (int ii=1;ii<subNum;ii++) {
        floatToFile(tlog,0,tr(" ").toUtf8().data(),amplSpectrum[ii],10,2,'f',0);
    }
    strToFile(tlog,0,tr(" ").toUtf8().data(),1);
 }

void DceSettings::initConnectParams()
{
    connectParams->chan_number[0] = txPspChannelBase;
    connectParams->chan_number[1] = rxPspChannelBase;
    connectParams->fixFrequencies[0] = FIX_FREQ_NUMBER2;
    connectParams->fixFrequencies[1] = FIX_FREQ_NUMBER1;
    connectParams->break_enabled=1;
    connectParams->duplex=0;
    connectParams->simplex = true;
    connectParams->fixFrequencyMode=false;
    connectParams->wait_best_frequency_in_transmission=false;
    connectParams->switch_mode = 1;
    connectParams->search_nrch_mode = true;
    connectParams->startup = false;
    connectParams->break_on_50 = false;
    connectParams->slot_counter = 0;
    connectParams->adapt_current = NONE;
    for (int ii=0;ii<32;ii++) {
        connectParams->fChannels[ii] = ii;
    }
    connectParams->cycle_before_switch_adapt = 0;
}

 void DceSettings::OnOffState(int state)
 {
     if (state){
         adapt_f = new TAdaptFreq(this, Number,frequencyNumber,frequencyNumberAll);
         mdmConnect->setAdaptFreq(adapt_f);
         cbVariants->setCurrentIndex(27);
         stat_timer->start(1000);
         mdmConnect->init_mdm_time();
     }else {
         stat_timer->stop();
         delete adapt_f;
         adapt_f = nullptr;
     }
     cbActiveFreq->setEnabled(!state);
     cbAllFreq->setEnabled(!state);
     cbLoop->setEnabled(state);
     isActive = state;
     chvm->setText(tr("ЧВМ=")+"0/0-0");
     emit turnOnState(state);
 }

 void DceSettings::autoBreakdown(int state)
 {
     connectParams->break_on_50 = state;
 }

 void DceSettings::speed_adapt_init(int state)
 {
     mdmConnect->init_adapt_s(state);
     cb50(state);
 }

 void DceSettings::cb50(int state)
 {
     cbAuto50->setChecked(state);
     showStatus(" ");
 }

 void	DceSettings::show_modem_time()
 {
     quint16 time[8];
     QString	sz,sz1;
     //uint128_t *tm=fslot->get_time();
     uint128_t tm=mdmConnect->get_mdm_time();
     for(int ii=0; ii < 8; ii++)
         time[ii]	= (tm).word16[ii];

     for (int ii=0;ii<8;ii++) {
         sz1.setNum(time[7-ii],16);
         sz1 = QString("%1").arg(sz1,4,'0');
         if (ii<7) sz1.append(":");
         sz.append(sz1);
     }
     timeBox->setText(sz);
     timeBox->setAlignment(Qt::AlignCenter);
 }

 bool DceSettings::modulate()
 {
     bool res;
     res = slotModulate();
//     if (res){
//         intToFile(tlog, 1, tr(" modnumber= ").toUtf8().data(),Number, 0, 10,' ',0);
//         intToFile(tlog, 0, tr(" ch_send= ").toUtf8().data(),chanNumber_send(), 0, 10,' ',1);
//     }
     return res;
 }

 void DceSettings::demodulate(bool rcv)
 {
     slotDemodulate(rcv);
 }

 void DceSettings::testSequence(QList<int>* mess,int len)
 {
     if(!servMode){
         if (mdmConnect->snd()){
             int chanIndex=3;
             int chanIndex1=4;
             if (connectParams->fixFrequencyMode){
                     chanNumber_recv_plus(&chanIndex);
                     chanIndex1 = chanIndex;
                     if (loopMode){
                         chanNumber_send_plus(&chanIndex1);
                     }
             }else{
                 chanNumber_send_plus(&chanIndex);
             }
             int head_len=mdmConnect->tx_heading(sended,chanIndex,chanIndex1);
             for (int ii=0;ii<len;ii++) {
                 if (ii<head_len) mess->append(sended[ii]);
                 else{
                     if (msgToSend.isEmpty()){
                         if (Number%2==1) mess->append(byteIndex++);
                         else mess->append(byteIndex--);
                     }else{
                         if (indexToSend < msgToSend.length()){
                            mess->append(static_cast<quint8>(msgToSend.at(indexToSend)));
                            indexToSend++;
                         }
                         if (indexToSend >= msgToSend.length()){
                             msgToSend.clear();
                             indexToSend = 0;
                         }
                     }
                 }
             }
         }
     }
 }

 QByteArray *DceSettings::slotExitData(int* rs, int* bf )
 {
     *rs = rxFslot->getRSerr();
     *bf = rxFslot->getBestFrame();
     return decoded;
 }

 void DceSettings::initSynchro(int num)
 {
     if(isActive){
         int len = rxFslot->getSlotMessageLength();
         if (len>=CODOGRAMM_LEN){  // формирование сервисной кодограммы
             connectParams->server_num = cbSynchrNumber->currentText().toInt();
             connectParams->repeat = num;
             mdmConnect->time_increment();
             mdmConnect->start_synchro(connectParams);
         }
     }
 }

 void DceSettings::setConnectParams(int psp_chan1, int psp_chan2)
 {
     qDebug() <<"psp_chan = "<< psp_chan1<<psp_chan2;
     if(psp_chan1 > 0){
         connectParams->server_num = cbConnectNumber->currentText().toInt();
         connectParams->duplex = cbDuplex->currentIndex();
     }else{
         showStatus(tr("Сеть занята"));
     }
 }

 void DceSettings::initConnect(bool visa)
 {
     if (visa){
         int len = rxFslot->getSlotMessageLength();
         if (len>=CODOGRAMM_LEN){  // формирование сервисной кодограммы
             mdmConnect->start_connect(connectParams);
         }
     }else{
         showStatus(tr("Сеть занята(0)"));
     }
 }

 int DceSettings::chanNumber_send()
 {
     int ch;
     if (connectParams->fixFrequencyMode)
        ch = connectParams->fixFrequencies[0];
     else
        //ch = (freqIndex + (connectParams->txPspChannel*2)) % frequencyNumber;
        ch = (connectParams->futureSendIndexes1.at(0) + (connectParams->txPspChannel*2)) % frequencyNumber;

     //ch = 2*((Number+1) % 2)+1;
     txFslot = &txFslots[ch];
     return connectParams->fChannels[ch];
 }

 int DceSettings::chanNumber_recv()
 {
     int ch;
     if (connectParams->fixFrequencyMode)
        ch = connectParams->fixFrequencies[1];
     else
        //ch =  (freqIndex + (connectParams->rxPspChannel*2)) % frequencyNumber;
         ch = (connectParams->futureRecvIndexes1.at(0) + (connectParams->rxPspChannel*2)) % frequencyNumber;

     //ch = 2*(Number % 2)+1;
     rxFslot = &rxFslots[ch];
     return connectParams->fChannels[ch];
 }

 int DceSettings::chanNumber_send_plus(int *index)
 {
     int ch;
     if (connectParams->fixFrequencyMode)
        ch = connectParams->fixFrequencies[0]; // 3
     else
        //ch =  (freqIndex + (connectParams->rxPspChannel*2)) % frequencyNumber;
        ch = (connectParams->futureSendIndexes1.at(0) + (connectParams->txPspChannel*2)) % frequencyNumber;

     //ch = 2*(Number % 2);
     *index = ch;
     return connectParams->fChannels[ch];
 }

 int DceSettings::chanNumber_recv_plus(int *index)
 {
     int ch;
     if (connectParams->fixFrequencyMode)
        ch = connectParams->fixFrequencies[1];  // 4
     else
        //ch =  (freqIndex + (connectParams->rxPspChannel*2)) % frequencyNumber;
        ch = (connectParams->futureRecvIndexes1.at(0) + (connectParams->rxPspChannel*2)) % frequencyNumber;

     //ch = 2*(Number % 2);
     *index = ch;
     return connectParams->fChannels[ch];
 }

 void DceSettings::startConnection()
 {
     if(isActive){
         initConnectParams();
         if (connectParams->switch_mode !=0 ){
             setVariantParams(cbConnectVariant->currentIndex());
             setConnectParams(connectParams->txPspChannel,connectParams->rxPspChannel);
             if (connectParams->switch_mode ==0){
                 connectParams->fixFrequencies[0] = FIX_FREQ_NUMBER1;
                 connectParams->fixFrequencies[1] = FIX_FREQ_NUMBER2;
             }
             initConnect(true);
         }
     }
 }

void DceSettings::breakConnection()
 {
    qDebug() << "breakConnection";
    mdmConnect->set_info_head_type(CONNECTION_BREAK,1,0);
 }

 bool DceSettings::slotModulate()
 {
     QList<int>* mess= mess_out;
     mess->clear();

     int len = txFslot->getSlotMessageLength();
     if (mdmConnect->serviceKdg()){
         if (len>=CODOGRAMM_LEN){  // формирование сервисной кодограммы
             if (mdmConnect->tx_process(sended, Number)){
                 for (int ii = 0; ii < len; ii++){
                     if (ii<CODOGRAMM_LEN ) mess->append(sended[ii]);
                     else mess->append(0);
                 }
             }
         }
     }
     else{
         testSequence(mess,len);
     }
     if (!mess->isEmpty()){
         logMessage(mess,1);
         txFslot->setSlotMessage(mess);
         silent_mode=false;
         ber.send += mess->length() << 3;
         if (messageOutput){
             //*mess = mess->mid(2);
             emit slotMessageShow("send Message:",mess);
         }
         if (writeMode){
              emit to_work_the_channel( getOutput(),txFslot->getSlotLength());
          }
     }else silent_mode=true;
//     if(!silent_mode)
//     {
//         strToFile(tlog,1,tr(" Fsend=").toUtf8().data(),0);
//         intToFile(tlog, 0, tr(" ").toUtf8().data(),chanNumber_send(), 5, 10,' ',0);
//         strToFile(tlog,0,tr("  ").toUtf8().data(),0);
//     }
     return !silent_mode;
 }

 void DceSettings::slotDemodulate(bool rcv)
  {
      QList<int>* mes;
      int kdg_call, rxFreqChan, chanIndex, err=0,max=1;
      rxFreqChan = chanNumber_recv_plus(&chanIndex);
      max = rxFslot->getmaxRSerr();
      err = max;
      //strToFile(tlog,1,tr(" mess1=").toUtf8().data(),1);
      //qDebug() << "slotDemod="<<Number<< connectParams->fixFrequencyMode<<connectParams->fixFrequencies[0]<<connectParams->fixFrequencies[1];
      if (rcv){
          mes= rxFslot->getSlotMessage();
          err = rxFslot->getRSerr();
          max = rxFslot->getmaxRSerr();//-1;
          if( err>=max) mes->clear();
          decoded->clear();
          if(!mes->isEmpty()){
              logMessage(mes,2);
              mdmConnect->clearBreakdown();
              for (int ii = 0; ii < mes->count(); ii++){
                  decoded->append(static_cast<quint8>(mes->at(ii)));
              }
              if(!decoded->isEmpty()){
                  kdg_call = mdmConnect->rx_process(decoded->data(),rxFslot->getBestFrame());
                  if (connectParams->switch_mode == 1){
                      if (kdg_call==1){
                          best_f->setStatCollectMode(frequencyNumber);
                      }
                      if (connectParams->search_nrch_mode){
                          quint8 mn = 2;
                          if (connectParams->duplex == 1) mn = connectParams->half_duplex_num;
                          best_f->collecting_statistics_on_frequencies(rxFslot,rxFreqChan,mn);
                      }
                  }
                  mdmConnect->rx_heading(decoded->data(),head_in,mes);
              }
              if (messageOutput){
                  if (mes->count()) emit slotMessageShow("recv Message:",mes);
              }
              bit_errors(mes);

          }else{
//              strToFile(tlog,1,tr(" empty mes").toUtf8().data(),0);
//              logX();
              mdmConnect->setConnectBreakdown();   // авария канала
          }
          sendPlotData();
      }else{
//          strToFile(tlog,1,tr(" no rcv").toUtf8().data(),0);
//          logX();
      }

      mdmConnect->addAdaptStat(err,max,chanIndex,rxFreqChan);
      mdmConnect->sendRecvControl();
      mdmConnect->state_refresh();
      if (connectParams->fixFrequencyMode)
          mdmConnect->time_increment();
      else{
          //mdmConnect->chan_refresh(&freqNumber, frequencyNumber/2);
          mdmConnect->chan_refresh1(&freqIndex);
      }
      adapt_refresh();
  }

 void DceSettings::logX()
 {
     int chanIndex1=0;
     int chanIndex2=0;
     chanNumber_send_plus(&chanIndex1);
     chanNumber_recv_plus(&chanIndex2);
     intToFile(tlog, 0, tr(" isn=").toUtf8().data(),chanIndex1, 4, 10,' ',0);
     intToFile(tlog, 0, tr(" chs=").toUtf8().data(),chanNumber_send(), 4, 10,' ',0);
     intToFile(tlog, 0, tr(" chr=").toUtf8().data(),chanNumber_recv(), 4, 10,' ',0);
     intToFile(tlog, 0, tr(" txVariant=").toUtf8().data(),connectParams->txVariant[chanIndex1], 5, 10,' ',0);
     intToFile(tlog, 0, tr(" rxVariant=").toUtf8().data(),connectParams->rxVariant[chanIndex2], 5, 10,' ',1);
 }

 void DceSettings::switchToBestFrequency(int num)
 {
     qDebug() <<"Modem "<< Number << " switched to fChannel "<< num;
     QString str=QString("переключен на канал НРЧ № ");
     QString str1;
     str1 = str1.setNum(num);
     str = str.append(str1);
     lbSyncConnect->setText(str);
 }

 void DceSettings::logMessage(QList<int>* mess, int mode)
 {
     int x0=0,x=0,x2=0;
     if (!mdmConnect->serviceKdg()){
         QByteArray xxx; // logging head values for
         xxx.append(static_cast<quint8>(mess->at(0)));
         xxx.append(static_cast<quint8>(mess->at(1)));
         head_Info *ihead = (head_Info *)xxx.data();
         x0 = ihead->f_adapt.id;
         x = ihead->f_adapt.f_number1;
         x2 = ihead->f_adapt.f_number2;
//         if (x0+x+x2 ==0){
//             return;
//         }
         strToFile(tlog,1,tr(" ").toUtf8().data(),0);
         intToFile(tlog, 0, tr(" id=").toUtf8().data(), x0, 3, 10,' ',0);
         intToFile(tlog, 0, tr(" x1=").toUtf8().data(), x, 3, 10,' ',0);
         intToFile(tlog, 0, tr(" x2=").toUtf8().data(), x2, 3, 10,' ',0);

         strToFile(tlog,0,tr(" msg=").toUtf8().data(),0);
         for (int ii=2;ii<mess->count();ii++) {
             intToFile(tlog, 0, tr(" ").toUtf8().data(),mess->at(ii), 4, 10,' ',0);
         }
         int chanIndex1=0;
         chanNumber_send_plus(&chanIndex1);
         intToFile(tlog, 0, tr(" isn=").toUtf8().data(),chanIndex1, 4, 10,' ',0);
         intToFile(tlog, 0, tr(" chs=").toUtf8().data(),chanNumber_send(), 4, 10,' ',0);
         intToFile(tlog, 0, tr(" chr=").toUtf8().data(),chanNumber_recv(), 4, 10,' ',0);

         intToFile(tlog, 0, tr(" ...").toUtf8().data(),mode, 4, 10,' ',0);
         strToFile(tlog,0,tr(" ").toUtf8().data(),1);
     }else{
         strToFile(tlog,1,tr(" serv_kdg=").toUtf8().data(),0);
         for (int ii=0;ii<mess->count();ii++) {
             intToFile(tlog, 0, tr(" ").toUtf8().data(),mess->at(ii), 4, 10,' ',0);
         }
         strToFile(tlog,0,tr(" ").toUtf8().data(),1);
     }
 }

void DceSettings::setFrchFrequencies(int f1,int f2)
{
    connectParams->fixFrequencies[0] = f1;
    connectParams->fixFrequencies[1] = f2;
}

void DceSettings::setPspChannels(int txCh,int rxCh)
{
    txPspChannelBase = txCh;
    rxPspChannelBase = rxCh;
    connectParams->txPspChannel = txCh;
    connectParams->rxPspChannel = rxCh;
}

void DceSettings::fixProcentValue(int procent)
{
    procentRec = procent;
    showDecodProcent();
}

void DceSettings::showDecodProcent()
{
    QString str;
    if (procentRec >= 0) {
        if (mdmConnect->ballIsMine()){
            str = QString(" Декодировано %1% слотов").arg(procentRec);
        } else str.clear();
        emit sendProcentInfo(str);
    }
}

void DceSettings::set_cbVariantIndex(int var)
{
    setVariant(var);
    mdmConnect->init_adapt_s(false);

    intToFile(tlog, 1, tr(" half_duplex_num=  ").toUtf8().data(),connectParams->half_duplex_num, 0, 10,' ',0);
    intToFile(tlog, 0, tr(" ball=").toUtf8().data(),connectParams->adapt_speed_ball_is_mine, 0, 10,' ',1);
    bool state = cbAdapt_s->isChecked();
    if (state)
        mdmConnect->init_adapt_s(state);

    disconnect(cbVariants, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setVariant);
    cbVariants->setCurrentIndex(var);
    connect(cbVariants, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &DceSettings::setVariant);
}

void DceSettings::adapt_refresh()
{
    if (connectParams->fixFrequencyMode)
    {
        int freqIndex = connectParams->adaptFreqIndex;
        int newRxVar = connectParams->newRxVariant;
        if (connectParams->rxVariant[freqIndex] != newRxVar )
            setRxChanVariant( freqIndex, newRxVar);
    }
}

void DceSettings::send_data(char *data, int length)
{
    msgToSend.clear();
    msgToSend.append(data,length);
    indexToSend  = 0;
}

void DceSettings::define_modem_properties(MODEM_PROP *mProp)
{
    mdmConnect->setModemProp(mProp);
}

void DceSettings::sendData()
{
    QByteArray ba;
    ba.append(0x7f);
    ba.append(0xfe);
    ba.append(0x44);
    ba.append(0xf6);
    ba.append(0xdb);
    ba.append(0xff);
    ba.append(0xff);
    send_data(ba.data(),ba.length());
}

int DceSettings::getFhssFreqsNumber()
{
    return frequencyNumberAll;
}

void DceSettings::setRR1(bool rnd)
{
    txFslot->setRR2(rnd);
    rxFslot->setRR2(rnd);
}

void DceSettings::set_psp_clock(quint32 tm)
{
    mdmConnect->set_mdm_time(tm);
}

void DceSettings::modemReset()
{
    qDebug() << "Reset"<< getModemNumber();
    //initConnectParams();
    setVariant(cbVariants->currentIndex());
    for (int ii=0;ii<32;ii++) {
        connectParams->fChannels[ii] = ii;
    }
    if (adapt_f != nullptr)
        adapt_f->initFreqStat( frequencyNumber,frequencyNumberAll);

}

void DceSettings::maybe_set_freq_adapt(bool perm)
{
    connectParams->freq_adapt = perm && cbAdapt_f->isChecked();
    qDebug() << "freq_adapt="<< connectParams->freq_adapt;
    mdmConnect->chan_refresh(&freqIndex, frequencyNumber/2);
}

void DceSettings::psp_next_number(int shift)
{
    mdmConnect->chan_refresh(&freqIndex, frequencyNumber/2,shift);
}

int DceSettings::get_channel_number(int psp)
{
    int ch = (freqIndex + (psp*2)) % frequencyNumber;
    return connectParams->fChannels[ch];
}

