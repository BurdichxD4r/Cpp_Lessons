#include "chanmodel.h"
#include "colorgrid.h"

ChanModel::ChanModel(QWidget *parent) : QWidget(parent)
{
    createWidgets();
    initInterface();

    setTextDefinitions();
    heavens = new TFadings(this);
    setChannelsParams(-1, 25.0f, 0.0f);

    //connect(cbSnr, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),this, &ChanModel::setSnrValue);
    connect(cbSnrScale, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),this, &ChanModel::passSnrMetric);
    connect(fadingRateBox, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &ChanModel::applyFadingSwitching);
    connect(applyButton,&QPushButton::clicked, this, &ChanModel::applyChanges);

    //qDebug() << "setOptions " <<  fft_dim << n_to_diap << afm4;;
    connect (m_activBut, &QAbstractButton::clicked, this, &ChanModel::activeChange);
    connect(this, &ChanModel::activeChSig, colorGrd, &ColorGrid::checkChngSlot );

    connect(this, &ChanModel::gridTimeOutSig, colorGrd, &ColorGrid::availCellDescSlot);
    connect(this, &ChanModel::flashToCell, colorGrd, &ColorGrid::reFreshAvailCellSlot);

    connect (m_colorBut, &QAbstractButton::clicked, this, &ChanModel::colorChange);
    connect(this, &ChanModel::colorChSig, colorGrd, &ColorGrid::colorChngSlot );

    connect (m_emulBut, &QAbstractButton::clicked, this, &ChanModel::setSnrDistributionNext);
    connect(colorGrd, &ColorGrid::numberCellSig, m_cellSp, &QSpinBox::setValue);
    connect(colorGrd, &ColorGrid::colorCellSig, m_colorSp, &QSpinBox::setValue);
    //connect(colorGrd, &ColorGrid::numberCellSig, this, &ChanModel::showSelectedChannelInfo);
    connect(m_cellSp ,QOverload<int>::of(&QSpinBox::valueChanged),this, &ChanModel::showSelectedChannelInfo);
    connect(freqShiftSet,&QLineEdit::textChanged, this, &ChanModel::setShiftValue);
    connect(bfShiftSet,&QLineEdit::textChanged, this, &ChanModel::setBfShiftValue);
    cbSnr->setCurrentIndex(25);
    setSnrValue(cbSnr->currentText());
    setBfShiftValue();


    for (int ii=0;ii<num_chan;ii++) {
        chanSignal[ii] = new QVector<fft_complex>(SLOT_LEN_MAX,fft_complex(0,0));
        chanExit[ii] = new QVector<int_complex>(SLOT_LEN_MAX,fft_complex(0,0));
        nWavesSlot[ii] = new QVector<fft_complex>(SLOT_LEN_MAX,fft_complex(0,0));
    }
    doubleSlt = new QVector<fft_complex>();
    setSnrDistribution();
}

ChanModel::~ChanModel()
{

}

void ChanModel::initInterface()
{

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(lbSnr);
    layout1->addWidget(cbSnr);
    layout1->addWidget(cbSnrScale);

    layout1->addWidget(freqShiftSet);
    layout1->addStretch();
    layout1->addWidget(bfShiftSet);
    layout1->addStretch();
    layout1->addWidget(fadingRateBox);
    layout1->addWidget(waveDelay);
    layout1->addWidget(fadingRate);
    layout1->addWidget(applyButton);

    QHBoxLayout *layout_but = new QHBoxLayout;
    layout_but->addWidget(m_colorBut);
    layout_but->addStretch();
    layout_but->addWidget(m_activBut);
    layout_but->addStretch();
    layout_but->addWidget(m_emulBut);

    QLabel *colorlb = new QLabel("Номер цвета", this);
    QLabel *celllb = new QLabel("Номер ячейки", this);
    colorlb->setAlignment(Qt::AlignRight);
    celllb->setAlignment(Qt::AlignRight);
    colorlb ->setBuddy ( m_colorSp );
    celllb ->setBuddy ( m_cellSp );

    QHBoxLayout *layout_spin = new QHBoxLayout;
    layout_spin->addWidget(celllb);
    layout_spin->addWidget(m_cellSp);
    layout_spin->addStretch();
    layout_spin->addWidget(colorlb);
    layout_spin->addWidget(m_colorSp);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(layout1);
    layout->addWidget(colorGrd);
    layout->addLayout(layout_but);
    layout->addLayout(layout_spin);
    //layout->addStretch();

    channelGroup = new QGroupBox(this);
    channelGroup->setLayout(layout);

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->addWidget(channelGroup);
    setLayout(layout2);
}

void ChanModel::createWidgets()
{
    QString str;
    lbSnr = new QLabel(this);
    lbSnr->adjustSize();

    cbSnr = new QComboBox(this);
    for(int ii=0;ii<snr_index_max;ii++){
        str = QString("%1" ).arg(ii);
        cbSnr->addItem(str);
    }
    str = QString("100" );
    cbSnr->addItem(str);

    cbSnrScale = new QComboBox(this);
    str = QString("EbN0" );
    cbSnrScale->addItem(str);
    str = QString("SNR" );
    cbSnrScale->addItem(str);
    str = QString("EsN0" );
    cbSnrScale->addItem(str);

    freqShiftSet = new QLineEdit;
    freqShiftSet->setMaximumWidth(30);
    QDoubleValidator* dValid = new QDoubleValidator();
    dValid->setRange(-10,10,1);dValid->setLocale(QLocale::English);
    freqShiftSet->setValidator(dValid);
    freqShiftSet->setText("0.0");

    bfShiftSet = new QLineEdit;
    bfShiftSet->setMaximumWidth(30);
    str = QString("%1").arg(bf_drift_velocity, 2, 'f', 1);
    QDoubleValidator* dVal = new QDoubleValidator();
    dVal->setRange(-10,10,2);dVal->setLocale(QLocale::English);
    bfShiftSet->setValidator(dVal);
    bfShiftSet->setText(str);

    fadingRateBox = new QComboBox(this);

    for(int ii=0;ii<11;ii++){
        str = QString("Вариант %1" ).arg(ii);
        fadingRateBox->addItem(str);
    }
    fadingRateBox->setMinimumWidth(120);
    fadingRateBox->setMaximumWidth(140);
    fadingRateBox->setCurrentIndex(0);
    listView = new QListView(fadingRateBox);
    fadingRateBox->setView(listView);
    listView->setRowHidden(3, true);
    listView->setRowHidden(9, true);
    listView->setRowHidden(10, true);

    waveDelay = new QLineEdit(this);
    waveDelay->setAlignment(Qt::AlignRight);
    waveDelay->setInputMask("00.0");
    waveDelay->setMaximumWidth(30);
    waveDelay->setText("0.0");

    fadingRate = new QLineEdit(this);
    fadingRate->setAlignment(Qt::AlignRight);
    fadingRate->setInputMask("00.0");
    fadingRate->setMaximumWidth(30);
    fadingRate->setText("0.0");

    applyButton = new QPushButton(this); // кнопка "Применить"
    applyButton->setEnabled(true);
    applyButton->setMinimumWidth(65);

    colorGrd = new ColorGrid(this,ROW_NUM,COL_NUM);
    colorGrd->setMinimumHeight(300);
    colorGrd->setMaximumHeight(300);
    //colorGrd->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
//    colorGrd->setMinHightCell(20);
//    colorGrd->setMinWidthCell(10);

    m_emulBut = new QPushButton("Массив ОСШ", this );
    m_activBut = new QPushButton("Изменить активность", this );
    m_colorBut = new QPushButton("Изменить цвет", this);
    m_colorSp = new QSpinBox(this);
    m_cellSp = new QSpinBox(this);

    m_cell_num = ROW_NUM * COL_NUM - 1;
    m_cellSp->setMinimum(0);
    m_cellSp->setMaximum(m_cell_num);
    m_colorSp->setMinimum(COLOR_MIN);
    m_colorSp->setMaximum(COLOR_MAX);

}

void ChanModel::setTextDefinitions()
{
    cbSnr->setToolTip(tr("Значение отношения сигнал/шум в АБГШ-канале"));
    lbSnr->setText(tr("ОСШ"));
    channelGroup->setTitle(tr("Модель канала"));
    freqShiftSet->setToolTip("Смещение частоты в канале, Гц");
    bfShiftSet->setToolTip("Скорость cмещения BestFrame");
    m_emulBut->setToolTip("Установить распределение отношения \nсигнал/шум по частотным каналам");

    fadingRateBox->setToolTip(tr("вариант замираний"));

    waveDelay->setToolTip(tr("задержка волны в ионосфере, мс"));
    fadingRate->setToolTip(tr("разброс частот, Гц"));
    applyButton->setText(tr("Применить"));
    applyButton->setToolTip(tr("Применить значения измененных параметров"));

    fadingRateBox->setItemText(0,tr("Без замираний"));
    fadingRateBox->setItemText(1,tr("Спокойные НШ"));
    fadingRateBox->setItemText(2,tr("Умеренные НШ"));
    fadingRateBox->setItemText(3,tr("Нарушенные НШ"));
    fadingRateBox->setItemText(4,tr("Спокойные СШ"));
    fadingRateBox->setItemText(5,tr("Умеренные СШ"));
    fadingRateBox->setItemText(6,tr("Нарушенные СШ"));
    fadingRateBox->setItemText(7,tr("Возмущенные СШ"));
    fadingRateBox->setItemText(8,tr("Спокойные ВШ"));
    fadingRateBox->setItemText(9,tr("Умеренные ВШ"));
    fadingRateBox->setItemText(10,tr("Нарушенные ВШ"));

}

void  ChanModel::calculateChannelTest(short_complex* inChan)
{
    chanSignal[0]->clear();
    for (int ii=0;ii<slotLength;ii++) {
         chanSignal[0]->append(fft_complex((*(inChan+ii)).real(),(*(inChan+ii)).imag()));
    }
    nWavesSlot[0]->clear();
    if (heavens->getgWave(m_cellSp->value()) != 0){
        heavens->form_gwave_output(nWavesSlot[0],chanSignal[0],0,freqShift);
    }
    QVector<fft_complex>* doubleSlot = doubleSlt;
    for (int ii = 0;ii < heavens->getSkyWaves(m_cellSp->value());ii++) {
        heavens->form_skywave_output(doubleSlot,chanSignal[0],m_cellSp->value(),ii, freqShift);
        wavesAddition(doubleSlot,nWavesSlot[0]);
    }

    chanExit[0]->clear();
    for (int ii=0;ii<nWavesSlot[0]->length();ii++) {
         nWavesSlot[0]->replace(ii, nWavesSlot[0]->at(ii) + awgnSigma * awgn_noise());
         int_complex sig = int_complex(goToInt24(nWavesSlot[0]->at(ii).real()), goToInt24(nWavesSlot[0]->at(ii).imag()));
         chanExit[0]->append(sig);
    }
    emit chan_exit_send(chanExit[0]);
    //snrMeasure(chanSignal);
}

void  ChanModel::setSnrValue(const QString& str)
{
    bool ok;
    int snr = str.toInt(&ok, 10);
    if (ok){
        snr_db = static_cast<float>(snr);
        awgnSigma =sigma_awgn();
        setSnrIndexToGrid(snr,m_cellSp->value());
        //emit activeChSig(m_cellSp->value());
        qDebug() << "snr_db=" << snr_db << "sigma=" <<awgnSigma;
    }
}

void  ChanModel::setOptions(SLOT_VARIANT *vars)
{
    subchan_number = vars->subchan_number;
    fft_dim = vars->freq_discr/100;
    slot_mc = vars->slot_time;
    modulationRate = vars->rate;
    bandNumber = 1;
    if (vars->freq_band==fBand_4500) bandNumber = 2;
    if (vars->freq_band==fBand_9000) bandNumber = 3;
    referenceParcel = getReferentParcel(vars);
    if (modulationRate == 4) afm_adjust =afm4;
    else afm_adjust =1;
    slotLength = fft_dim*(vars->slot_time/10);
    heavens->setSlotLength(slotLength);
    koef_to_band = sqrtf((float)fft_dim/subchan_number/modulationRate);
    awgnSigma =sigma_awgn();
   qDebug() << "setOptions" <<vars->num<< subchan_number <<  fft_dim << modulationRate
            <<bandNumber <<afm_adjust << "sigma="<<awgnSigma<<koef_to_band<<referenceParcel<<slotLength;
   setSnrIndexToGrid(cbSnr->currentIndex(),m_cellSp->value());
   setShiftValue();
   applyChanges();
}

void ChanModel::setSnrIndexToGrid(int index,int cell)
{
    int snr = COLOR_ZERO_SNR + index*SNR_SCALE;
    if (snr>COLOR_MAX) snr=COLOR_MAX;
    emit colorChSig(cell, snr);
}

void ChanModel::setSnrIndex(int index)
{
    if (index == -1){
        setSnrIndexToGrid(cbSnr->currentIndex(),m_cellSp->value());
    }else{
        if ((index >= 0)&&(index<cbSnr->count())){
            cbSnr->setCurrentIndex(index);
        }else
            cbSnr->setCurrentIndex(0);
    }
}

float ChanModel::sigma_awgn()
{
    float sigma =sqrtf(fft_dim)*scaleFactor[bandNumber-1]*afm_adjust;
    if (cbSnrScale->currentText() == "EbN0"){
        float to_the_bit = sqrtf(modulationRate);
        sigma /= to_the_bit;
    }
    if (cbSnrScale->currentText() == "EsN0"){
    }
    sigma *= expf(-ln10_20*snr_db);
    return sigma;
}

float ChanModel::sigma_awgn_chan(int chan)
{
    float sigma =sqrtf(fft_dim)*scaleFactor[bandNumber-1]*afm_adjust;
    if (cbSnrScale->currentText() == "EbN0"){
        float to_the_bit = sqrtf(modulationRate);
        sigma /= to_the_bit;
    }
    if (cbSnrScale->currentText() == "EsN0"){
    }
    sigma *= heavens->getSNR(chan);
    return sigma;
}

void  ChanModel::snrMeasure(fft_complex* chanS)
{
    float x1=0;float x2=0;float y1,y2,y3;
    int N1=fft_dim*referenceParcel*1.25;
    int N2 = slotLength; //18
    //int N1=240;int N2 = slotLength; //27
    for (int ii=0;ii<slotLength;ii++) {
        fft_complex x = awgnSigma * awgn_noise();
        if ((ii >=N1)&&(ii < N2))
        {
            x1 = x1 +powf(abs(chanS[ii]),2)*0.0001;
            x2 = x2 +powf(abs(x),2)*0.0001;
        }
        if (ii == N2-1){
            y1 = sqrtf(x1/(N2-N1-1));
            y2 = sqrtf(x2/(N2-N1-1));
            y3 = y1/y2;
            qDebug() << N1<<N2<<" SNR,dB "<< 20*log10f(y3*koef_to_band) <<y1*100 << y2*100<<awgnSigma << y3 ;
        }
    }
}

int ChanModel::getReferentParcel(SLOT_VARIANT *vars)
{
    int ref;
    if (vars->f_change_time == 3) {
        ref = 1;
    }else if (vars->f_change_time == 75) {
        ref = 7;
    }else ref = 0;
    return ref;
}

qint32 ChanModel::goToInt24(float value)
{
    qint32 res=0;
    qint32 lim = 0x7fffff;
    if (fabs(value) < lim) res = static_cast<qint32>(value);
    else {
        if (value > 0) res = lim;
        else res = -lim;
    }
    return res;
}

void ChanModel::activeChange()
{
    emit activeChSig(m_cellSp->value());
}

void ChanModel::colorChange()
{
    emit colorChSig(m_cellSp->value(), m_colorSp->value());
}

void   ChanModel::setSingleSnr(int cell,float snr)
{
    float snr1 = snr;
    if (cell >= freqAll) snr1 = -29;
    setChannelsParams(cell,snr1,0.0f);
    setSnrIndexToGrid(snr1,cell);
}

void ChanModel::setSnrDistribution()
{
    float xx;
    float snr_min = 3.0f;
    float snr_max = 20.0f;
    if (countDistribution==0){
        for (int i = 0; i <= m_cell_num; ++i){
            xx = (snr_max-snr_min)* fabs(sinf(pi*(i-7.0)/35))+snr_min;
            setSingleSnr(i,xx);
        }
    }
    if (countDistribution==1){
        for (int i = 0; i <= m_cell_num; ++i){
            if (i < 128) xx = static_cast<float>(snr1[i]);
            else xx = 25.0f;
            setSingleSnr(i,xx);
        }
    }
    if (countDistribution==2){
       xx = 25.0f;
       for (int i = 0; i <= m_cell_num; ++i){
           setSingleSnr(i,xx);
       }
    }
    showSelectedChannelInfo(0);
}

void ChanModel::setSnrDistributionNext()
{
    countDistribution = (countDistribution+1)%3;
    setSnrDistribution();
}

void ChanModel::setShiftValue()
{
    QString str = freqShiftSet->text();
    float fShift = 2*pi*str.toFloat();
    freqShift = fShift/(fft_dim*100);
    //qDebug() << "freq="<<freqShift;
}

int ChanModel::replaceOffset()
{
    static float offset = (float)(slotLength/2-1); // смещение слота в интервале входного массива двойной длины
    offset = offset + bf_drift_velocity;
    int nset = (int)(offset) % slotLength;
    if (nset<0) nset = slotLength - nset;
    if (nset>slotLength) nset = nset - slotLength;
    //qDebug() << "offset="<<nset;
    return nset;
}

void ChanModel::setBfShiftValue()
{
    QString str = bfShiftSet->text();
    bf_drift_velocity = str.toFloat();
    heavens->setBfDrift(bf_drift_velocity);
    //qDebug() << "bfShiftValue="<<bf_drift_velocity;
}

void ChanModel::applyFadingSwitching(int index)
{
    QString str;
    str.setNum(fSpread[index],'f',1);
    fadingRate->setText(str);
    str.setNum(fDelays[index],'f',1);
    waveDelay->setText(str);
}

void ChanModel::setChannelsParams(int chanNum,float snr, float freqShift)
{
    ChannelParams	*cp = new ChannelParams	;

    cp->Length = slotLength;
    cp->SNR	 = snr;
    cp->Shift	 = freqShift;
    cp->orthInt = fft_dim;
    cp->slot_mc = slot_mc;

    int index = fadingRateBox->currentIndex();
    cp->frequency_spreading	= fSpread[index];
    cp->var_number = index;
    cp->t1			= 0.0f;
    cp->t2			= fDelays[index];
    cp->t3			= 0.0f;
    cp->gwave		= 0.0f;
    cp->n_sky		= 2;
    switch( index )
    {
    case 0:       
        cp->gwave		= 1.0f;
        cp->n_sky		= 0;
        cp->ih_number = -1;  // без замираний
        break;
    case 1:
    case 5:
    case 8:
        cp->ih_number = 1;  // 0.5ms
        break;
    case 4:
        cp->ih_number = 0; // 0.1ms
        break;
    case 6:
    case 7:
        cp->ih_number = 2;  // 1.0ms
        break;
    case 2:
        cp->ih_number = 3;  // 1.5ms
        break;
    case 3:
    case 9:
        cp->ih_number = 4;  // 10.0ms
        break;
    case 10:
        cp->ih_number = 5;  // 30.0ms
        break;
    default:
        cp->gwave		= 1.0f;
        cp->n_sky		= 0;
    }

    //qDebug() << "cp->fading_rate=" << cp->fading_rate << "cp->t2=" << cp->t2;
    if(chanNum == -1){
        for(int ii=0; ii < num_chan; ii++ ){
          heavens->ctrl_channel_init( *cp, ii );   // инициализация 128 частотных каналов
        }
    }
    else{
        if ((chanNum >= 0)&&(chanNum < num_chan)){
            heavens->ctrl_channel_init( *cp, chanNum );
        }
        //chanPlot->setChannelDataPointer(chanNum, &cs[chanNum]->Received[offset]);
    }
    delete cp;
}

void ChanModel::applyChanges()
{
    QString str;
    int  snr;
    float freqShift;
    str = cbSnr->currentText();
    setSnrValue(str);
    snr = str.toInt();
    str = freqShiftSet->text();
    freqShift = str.toFloat();

    setChannelsParams(m_cellSp->value(),snr,freqShift);

    setIndexFadingBox();
}

void ChanModel::showSelectedChannelInfo(int chNum)
{
    int row =  chNum/16;
    int col =    chNum - row*16;
    colorGrd->setFocus();
    colorGrd->setCurrentCell(row,col);
    QString  str;
    ChannelParams	*cp = new ChannelParams;
    heavens->getChannelParams(chNum, cp);
    str = str.setNum(cp->SNR,'f',0);
//    qDebug() << "back" << cp->Shift << cp->SNR << cp->fading_rate << "t2="<<cp->t2
//             << "var_num="<<cp->var_number;
    if (cp->SNR<30) cbSnr->setCurrentIndex(cp->SNR);
    else cbSnr->setCurrentIndex(30);
    setSnrValue(cbSnr->currentText());
    str = str.setNum(cp->Shift,'f',1);
    freqShiftSet->setText(str);
    str= str.setNum(cp->frequency_spreading,'f',1);
    fadingRate->setText(str);
    str= str.setNum(cp->t2,'f',1);
    waveDelay->setText(str);

    setIndexFadingBox();
}

void ChanModel::setIndexFadingBox()
{
    ChannelParams	*cp = new ChannelParams();
    heavens->getChannelParams(m_cellSp->value(), cp);
    disconnect(fadingRateBox, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &ChanModel::applyFadingSwitching);
    qDebug() << "cp->var_number="<<cp->var_number << "cp->ih_number="<<cp->ih_number;
    fadingRateBox->setCurrentIndex(cp->var_number);
    connect(fadingRateBox, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &ChanModel::applyFadingSwitching);
}

void ChanModel::wavesAddition(QVector<fft_complex>* doubleSlot, QVector<fft_complex> *nW)
{
    if(nW->count() == 0){
        nW->append(*doubleSlot);
    }else{
        int len = nW->count();
        if (doubleSlot->count() < len) len = doubleSlot->count();
        for (int ii = 0;ii < len;ii++) {
            nW->replace(ii,nW->at(ii)+doubleSlot->at(ii));
        }
    }
}

void ChanModel::setSlotToChannel(short_complex* inChan,int chan)
{
    chanSignal[chan]->clear();
    for (int ii=0;ii<slotLength;ii++) {
         chanSignal[chan]->append(fft_complex((*(inChan+ii)).real(),(*(inChan+ii)).imag()));
    }
}

void ChanModel::renewCellsCounters()
{
    for(int k = 0; k < m_cell_num; ++k){
        emit gridTimeOutSig(k);
    }
}

void  ChanModel::calculateChannels(QList<int>* flist)
{
    int jj;
    renewCellsCounters();
    if (!flist->isEmpty()){
        for (int ii=0;ii<flist->count();ii++) {
            jj = flist->at(ii);
            emit flashToCell(jj);
            calculateChannel(chanSignal[jj],chanExit[jj],jj);
        }
    }
}

void  ChanModel::cleanChannels(QList<int>* flist)
{
    int jj;
    if (!flist->isEmpty()){
        int_complex sig = int_complex(0,0);
        for (int ii=0;ii<flist->count();ii++) {
            jj = flist->at(ii);
            chanExit[jj]->fill(sig,SLOT_LEN_MAX);
        }
    }
}

void  ChanModel::calculateChannel(QVector<fft_complex> *chanSignal1,QVector<int_complex>* chanExit1,int chan)
{
    nWavesSlot[chan]->clear();
    if (heavens->getgWave(m_cellSp->value()) != 0){
        heavens->form_gwave_output(nWavesSlot[chan],chanSignal1, chan, freqShift);
    }
    QVector<fft_complex>* doubleSlot = doubleSlt;
    for (int ii = 0;ii < heavens->getSkyWaves(m_cellSp->value());ii++) {
        heavens->form_skywave_output(doubleSlot,chanSignal1,chan,ii, freqShift);
        wavesAddition(doubleSlot,nWavesSlot[chan]);
    }
    chanExit1->clear();
    for (int ii=0;ii<nWavesSlot[chan]->length();ii++) {
         nWavesSlot[chan]->replace(ii, nWavesSlot[chan]->at(ii) + sigma_awgn_chan(chan) * awgn_noise());
         int_complex sig = int_complex(goToInt24(nWavesSlot[chan]->at(ii).real()), goToInt24(nWavesSlot[chan]->at(ii).imag()));
         chanExit1->append(sig);
    }
    chanSignal1->fill(fft_complex(0,0));
}

QVector<int_complex>* ChanModel::getChanOutput(int chan)
{
    return chanExit[chan];
}

int ChanModel::getSlot_msec()
{
    return slot_mc;
}

void ChanModel::setAllFreqsNum(int num)
{
    freqAll = num;
    setSnrDistribution();
}
