#include "modempanel.h"
#include "tconsole.h"
#include "tstream.h"
#include "tsettings.h"

#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QMetaType>
#include <QTextOption>
#include <QProgressBar>

ModemPanel::ModemPanel(QWidget *parent)
    : QWidget(parent)
{

    tabWidget = new QTabWidget();
    settings = new DceSettings(this);
    streams = new TStream(this);
    files = new TDataFile(this);
    mprops = new MODEM_PROP();
    mprops->snd = false;
    mprops->data_exchange = false;
    define_modem_properties(mprops);

    tabWidget->addTab(settings, tr("Параметры"));
    tabWidget->addTab(streams, tr("Потоки"));
    tabWidget->addTab(files, tr("Резерв"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);

    modemGroupBox = new QGroupBox(this);
    modemGroupBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    modemGroupBox->setLayout(mainLayout);

    connect(settings, &DceSettings::symbolsSended, streams, &TStream::reflectData);
    connect(settings, &DceSettings::slotMessageShow, streams, &TStream::reflectMessage);
    connect(this,  QOverload<bool>::of(&ModemPanel::setPause), settings, &DceSettings::stopGoTimer);
    connect(this,  QOverload<bool>::of(&ModemPanel::setDataOutput), settings, &DceSettings::dataOutputMode);
    connect(this,  &ModemPanel::setCycle, settings, &DceSettings::setTimerCycle);
    connect(settings, &DceSettings::timerIsStarted, this, &ModemPanel::changeTabWidget);
    connect(settings, &DceSettings::send_ber, streams, &TStream::showBer);
    connect(streams, &TStream::clearFromStreams, settings, &DceSettings::clearBer);
    connect(settings, &DceSettings::to_work_the_channel, this, &ModemPanel::to_work_the_channel,Qt::DirectConnection);
    connect(settings, &DceSettings::pass_over_variant, this, &ModemPanel::pass_over_variant);
    connect(settings, &DceSettings::fix_current_test, this, &ModemPanel::setPauseFlag);
    connect(settings, &DceSettings::fix_current_test, files, &TDataFile::showBerMeasure);
    connect(settings, &DceSettings::changeSnr, this, &ModemPanel::changeSnr);
    connect(this, &ModemPanel::passSnrMetric, files, &TDataFile::setSnrMetric);
    connect(settings, &DceSettings::to_work_the_channel, files, &TDataFile::writeSlotToFile,Qt::DirectConnection);
    connect(files,&TDataFile::readyReadFile, settings, &DceSettings::setReadFileMode);
    connect(files,&TDataFile::writeMode, settings, &DceSettings::setWriteMode);
    connect(settings, &DceSettings::anotherSlotFromFile_Please, files, &TDataFile::buildInputSignal);
    connect(files, &TDataFile::chan_exit_sendf, settings, &DceSettings::setInputSignalTest);
    connect(settings, &DceSettings::pass_over_variant, files, &TDataFile::fsetOptions);
    connect(settings, &DceSettings::turnOnState, this, &ModemPanel::setActiveSign);
    connect(settings, &DceSettings::to_file_writing, files, &TDataFile::writeSlotToFile);
    connect(settings, &DceSettings::transmitConnectState, this, &ModemPanel::transmitConnectState);
    connect(settings, &DceSettings::loop_on_off, this, &ModemPanel::loop_on_off);
    connect(settings, &DceSettings::allFreqsNumber, this, &ModemPanel::allFreqsNumber);
}

void ModemPanel::setPanelVariant(int vars)
{
    settings->setVariantIndex(vars);
}

void ModemPanel::setSamplingRate(int vars, int sampling)
{
    settings->setSamplingRate(vars,sampling);
}

ModemPanel::~ModemPanel()
{
    //onClose();
}

int  ModemPanel::getModemNumber()    // возвращает номер модема
{
    return settings->getModemNumber();
}

void  ModemPanel::setModemNumber(int num) // присваивает модему номер
{
     Number = num;
     QString title= QString("Модем %1").arg(Number);
     modemGroupBox->setTitle(title);
     settings->setModemNumber(num);
}

 void ModemPanel::setTexts()
 {
     tabWidget->setTabText(0,tr("Settings"));
     tabWidget->setTabText(1,tr("Stream"));
     tabWidget->setTabText(2,tr("Резерв"));
 }

 void ModemPanel::changeEvent(QEvent *event)
 {
     if (event->type() == QEvent::LanguageChange) {
         //setTexts();
     } else
         QWidget::changeEvent(event);
 }

 void ModemPanel::changeTabWidget()
 {
      tabWidget->setCurrentWidget(streams);
 }

short_complex* ModemPanel::getOutput()
 {
     return settings->getOutput();
 }

int ModemPanel::getSlotLength()
{
    return settings->getSlotLength();
}

int ModemPanel::getSlotMsgLen()
{
    if (isActive)
        return settings->getSlotMsgLen();
    else
        return 0;
}

bool ModemPanel::getChanExit(QVector<int_complex>* signal)
{
    //qDebug() << "chan_signal "<<signal->count();
    return settings->setInputSignal(signal);
}

void ModemPanel::getChanExitTest(QVector<int_complex>* signal)
{
    settings->setInputSignalTest(signal);
}

void ModemPanel::setActiveSign(bool state)
{
    isActive = state;
}

int ModemPanel::getSkkNumber()
{
    return settings->getSkkNumber();
}

bool ModemPanel::modulate()
{
    return settings->modulate();
}

void ModemPanel::demodulate(bool rcv)
{
    settings->demodulate(rcv);
}

void ModemPanel::setPauseThread(bool mode)
{
    settings->setTestMode(mode);
}

int ModemPanel::chanNumber_send()
{
    return settings->chanNumber_send();
}

int ModemPanel::chanNumber_recv()
{
    return settings->chanNumber_recv();
}

void ModemPanel::set_turnOn_State(bool state)
{
    settings->set_turnOn_State(state);
}

void ModemPanel::set_turnOn_Plots(bool state)
{
    settings->set_turnOn_Plots(state);
}

QByteArray *ModemPanel::slotExitData(int* rs, int* bf )
{
    return settings->slotExitData(rs,bf);
}

QList <fft_complex>* ModemPanel::getSlotSpectr()
{
    return settings->getSlotSpectr();
}
float ModemPanel::getSlotSNR()
{
    return settings->getSlotSNR();
}

void ModemPanel::setFrchFrequencies(int f1,int f2)
{
    settings->setFrchFrequencies( f1, f2);
}

void ModemPanel::setPspChannels(int txCh, int rxCh)
{
    settings->setPspChannels(txCh,rxCh);
}

void ModemPanel::send_data(char *data, int length)
{
    settings->send_data(data, length);
}

void ModemPanel::define_modem_properties(MODEM_PROP *mProp)
{
    settings->define_modem_properties(mProp);
}

void ModemPanel::setChannelLevel(bool state)
{
    mprops->snd = state;
    mprops->data_exchange = state;
    define_modem_properties(mprops);
}

bool ModemPanel::fromFile()
{
    return settings->fromFileToDemod();
}

void ModemPanel::setModemReset()
{
    settings->modemReset();
}


int ModemPanel::getAllFrequenciesNumber()
{
    return settings->getFhssFreqsNumber();
}
void ModemPanel::setActiveFrNum(const QString freq)
{
    settings->set_cbActiveFreq(freq);
}

void ModemPanel::setAllFrNum(const QString allF)
{
    settings->set_cbAllFreq(allF);
}

void ModemPanel::setRandomReference(bool rnd)
{
    settings->setRR1(rnd);
}

void ModemPanel::set_psp_clock(quint32 tm)
{
    initialTime = tm;
    settings->set_psp_clock(tm);
}

void ModemPanel::time_to_psp(int t_go)
{
    int shift = t_go - lastSlotTime;
    settings->psp_next_number(shift);
    lastSlotTime = t_go;
}

int ModemPanel::get_psp_channel(int t_go,int psp)
{
    time_to_psp(t_go);
    return settings->get_channel_number(psp);
}
