#ifndef TSETTINGS_H
#define TSETTINGS_H

#include <QtWidgets>
#include "fhssmodem.h"
#include "fhssslot.h"
#include "tconnect.h"
#include "tbestfreq.h"

class ModemChart;
class TConnect;
class TAdaptSpeed;
class TAdaptFreq;

class DceSettings : public QWidget
{
    Q_OBJECT
public:
    explicit DceSettings(QWidget *parent = nullptr);
    ~DceSettings();

    bool isActive = false;
    short_complex* getOutput();
    int getSlotLength(); // возвращает длину слота(размер массива)
    bool modulate();
    void demodulate(bool rcv);
    QByteArray* slotExitData(int* rs, int* bf );
    QList <fft_complex>* getSlotSpectr();
    float getSlotSNR();
    int getSlotMsgLen();
    int getSkkNumber();
    int getFhssFreqsNumber();
    void setSamplingRate(int vars, int sample);
    void setRR1(bool rnd);
    void set_psp_clock(quint32 tm);   //установка tm для генератора ПСП
    void psp_next_number(int shift=1);
    int get_channel_number(int psp);

private:

    int developmentPhase=4;  // параметр, определяющий набор реализованных вариантов в соответствии с этапом разработки
    int Number=0;                      // номер модема
    quint8 byteIndex=0;              // указывает текущее положение передаваемой тестовой последовательности
    QCheckBox *turnOn;

    QLabel *lbChannel;
    QComboBox *cbVariants;  // выбор варианта
    QComboBox *cbSampleRate;  // выбор частоты дискретизации
    QComboBox *cbStage;  // Этап разработки (1,2,3 и все)
    QTextEdit *infoBox; // отображение информационной панели состояния модема

    QGroupBox *variantsGroup;
    QTimer *timer;            // управление циклом обмена
    QTimer *stat_timer;     // управление отображением статистики обмена
    QCheckBox *startTest;
    QCheckBox *cbBroadCast;
    QCheckBox *cbLoop; 
    QCheckBox *berMe;
    QCheckBox *plotters;
    QCheckBox *shiftCompensation; // вкл/выкл компенсации частотного сдвига
    QCheckBox *cbAdapt_s;   // вкл/выкл адаптацию по скорости
    QCheckBox *cbAdapt_f;   // вкл/выкл адаптацию по частоте
    QCheckBox *cbAuto50;   // вкл/выкл разрыв соединения если 50 слотов подряд не декодированы

    FhssSlot rxFslots[32];
    FhssSlot txFslots[32];    // массивы по ЧК

    FhssSlot *rxFslot;
    FhssSlot *txFslot;    // для режима передачи
    ModemChart *dcePlot;
    QComboBox *cbSynchrNumber;  // номер модема для синхронизации
    QPushButton *pbSynchr;
    QComboBox *cbConnectNumber;  // номер модема для соединения
    QComboBox *cbConnectVariant;  // предлагаемый для соединения номер варианта
    QComboBox *cbDuplex;  // дуплекс-полудуплекс
    QPushButton *pbConnect;
    QPushButton *pbBreak;
    QLabel *lbSyncConnect;
    QLabel *chvm;
    QComboBox *cbActiveFreq;  // число активных частот
    QComboBox *cbAllFreq;  // число всех(активные плюс резервные) частот

    int timerCycle = -1;
    bool processSuspended = false;
    bool messageOutput = true;
    Modem_stream ber={0,0,0,0,0,0,"",0,0,-2,0};

    bool firsts=true;
    bool readFileMode=false;  // режим подачи на вход модема сигнала из файла
    CLogger tlog;
    QLineEdit *timeBox; // отображение значения текущего внутреннего времени модема
    QGroupBox *timeGroup;
    TConnect *mdmConnect=nullptr;
    quint8* sended;
    QByteArray* decoded;  // декодированные данные

    SLOT_VARIANT variants[VARS_NUMBER]  {
        { 1,tr("ФРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_15,sub_29,sbits,ibits_240,54,32,duplex_and_half,3,8,1},
        { 2,tr("ФРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),2,packs_15,sub_29,sbits,ibits_480,108,62,duplex_and_half,3,8,1},
        { 3,tr("ФРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),3,packs_15,sub_29,sbits,ibits_720,162,92,duplex_and_half,3,8,1},
        { 4,tr("ФРЧ"), 3,slot200,fBand_3100,fd_96,tr("АФМ"),4,packs_15,sub_29,sbits,ibits_960,217,123,duplex_and_half,3,8,1},
        { 5,tr("НРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_15,sub_29,sbits,ibits_240,54,32,duplex_and_half,3,8,2},
        { 6,tr("НРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),2,packs_15,sub_29,sbits,ibits_480,108,62,duplex_and_half,3,8,2},
        { 7,tr("НРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),3,packs_15,sub_29,sbits,ibits_720,162,92,duplex_and_half,3,8,2},
        { 8,tr("НРЧ"), 3,slot200,fBand_3100,fd_96,tr("АФМ"),4,packs_15,sub_29,sbits,ibits_960,217,123,duplex_and_half,3,8,2},
        { 9,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_9,sub_29,sbits,ibits_120,32,18,duplex_and_half,1,8,3},
        {10,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),2,packs_9,sub_29,sbits,ibits_240,65,33,duplex_and_half,1,8,3},
        {11,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),3,packs_9,sub_29,sbits,ibits_360,97,47,duplex_and_half,1,8,3},
        {12,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),3,packs_9,sub_29,sbits,ibits_480,97,63,duplex_and_half,1,8,3},
        {13,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("АФМ"),4,packs_9,sub_29,sbits,ibits_480,130,62,duplex_and_half,2,8,3},
        {14,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("АФМ"),4,packs_9,sub_29,sbits,ibits_720,130,92,duplex_and_half,2,8,3},
        {15,tr("НРЧ"),75,slot200,fBand_4500,fd_96,tr("ОФМ"),1,packs_9,sub_44,sbits,ibits_240,49,33,duplex_and_half,1,8,4},
        {16,tr("НРЧ"),75,slot200,fBand_4500,fd_96,tr("ОФМ"),2,packs_9,sub_44,sbits,ibits_480,99,63,duplex_and_half,1,8,4},
        {17,tr("НРЧ"),75,slot200,fBand_4500,fd_96,tr("ОФМ"),3,packs_9,sub_44,sbits,ibits_720,148,92,duplex_and_half,1,8,4},
        {18,tr("НРЧ"),75,slot200,fBand_4500,fd_96,tr("АФМ"),4,packs_9,sub_44,sbits,ibits_960,198,122,duplex_and_half,2,8,4},
        {19,tr("ППРЧ"),3,slot50,fBand_4500,fd_96,tr("ОФМ"),1,packs_3,sub_44,sbits,ibits_060,16,10,duplex_and_half,2,8,5},
        {20,tr("ППРЧ"),3,slot50,fBand_4500,fd_96,tr("ОФМ"),2,packs_3,sub_44,sbits,ibits_120,33,17,duplex_and_half,2,8,5},
        {21,tr("ППРЧ"),3,slot50,fBand_4500,fd_96,tr("ОФМ"),3,packs_3,sub_44,sbits,ibits_240,49,33,duplex_and_half,2,8,5},
        {22,tr("ППРЧ"),3,slot50,fBand_4500,fd_96,tr("АФМ"),4,packs_3,sub_44,sbits,ibits_480,66,62,duplex_and_half,2,8,5},
        {23,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("ОФМ"),1,packs_3,sub_89,sbits,ibits_060,33,11,duplex_and_half,2,8,6},
        {24,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("ОФМ"),1,packs_3,sub_89,sbits,ibits_120,33,17,duplex_and_half,2,8,6},
        {25,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("ОФМ"),2,packs_3,sub_89,sbits,ibits_240,66,32,duplex_and_half,2,8,6},
        {26,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("ОФМ"),3,packs_3,sub_89,sbits,ibits_480,100,62,duplex_and_half,2,8,6},
        {27,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("АФМ"),4,packs_3,sub_89,sbits,ibits_960,133,123,duplex_and_half,2,8,6},
        {28,tr("ППРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_9,sub_29,sbits28,ibits_0,32,22,serv_package,1,8,7},
        {29,tr("ППРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_9,sub_29,sbits29,ibits_120,32,22,simplex,1,8,7}
    };
    QList <int> stageVariants; // список вариантов с учетом стадии разработки
    QList<int>* message;
    bool isSynhro=false;
    bool writeMode=false;
    int snrIndex;
    int curVariant=0; // номер текущего варианта, выбранный в комбобоксе cbVariants
    quint32 freqs_number=4;  // число активных рабочих частот(основных)
    QList<int>* mess_out;
    QList<int>* head_in;
    bool silent_mode=false;
    short_complex* noSignal;    // для режима молчания в служебном канале
    bool servMode=false; // режим служебных кадров
    int freqIndex=0;   // номер индекса частотного канала  для текущего слота

    int frequencyNumber=16;   // число активных частот в ППРЧ
    int frequencyNumberAll=128;   // число всех частот в ППРЧ

    int txPspChannelBase=0;
    int rxPspChannelBase=0;

    connect_Pars_T* connectParams;  // параметры соединения
    TAdaptSpeed* adapt_v=nullptr;
    int procentRec=-1; // процент принятых слотов за цикл адаптации
    state_aT     adapt_action;          // состояние действия по изменению СКК в адаптации
    QByteArray msgToSend;
    int indexToSend;

    TAdaptFreq* adapt_f=nullptr;
    TBestFreq* best_f;
    bool loopMode=false;

    void initInterface();
    void createWidgets();

    void setTextDefinitions();
    QString showArray(quint8 *arr, int len);
    QString showByteArray(QByteArray arr, int len);

    void writeSettings();
    void readSettings();
    void ber_evaluate(QList<int>* sendMes, QList<int>* recvMes);
    int  bit_difference(int num1, int num2);
    void changeVariant();
    void changeShift(bool change);
    void bit_errors(QList<int>* recvMes);
    void the_total_spectrum_of_the_slot(QList <fft_complex>* sozv);  // сложение спектр.составл.в пределах слота по подканалам
    void	show_modem_time(); // отображение тек.времени в модемной панели
    void testSequence(QList<int>* mess, int len);  // формирование тестовых  символов для слота
    void logMessage(QList<int>* mess,int mode);   // вывод сообщения слота  в логгер
    void initConnectParams();
    void setSampleRateItem(int jj);  //  в комбобоксе частоты дискретизации установить пункт,соответствующий варианту СКК
    void setVariant(int jj);
    int chanNumber_recv_plus(int* index); // функция для режима адаптации по частоте
    int chanNumber_send_plus(int* index); // функция для режима адаптации по частоте
    void setTxVariant(int ii);  // установка варианта на передачу
    void setRxVariant(int ii);  // установка варианта на прием
    void logX();

signals:
    void symbolsSended(QString title,  int* codes, int len);
    void slotMessageShow(QString title, QList<int>* mess);
    void timerIsStarted(int cycle);
    void send_ber(Modem_stream* ber);
    void  to_work_the_channel(short_complex* inChan, int num);
    void  to_file_writing(short_complex* inChan, int num);
    void pass_over_variant(SLOT_VARIANT *vars);
    void fix_current_test(bool mode,Modem_stream* ber);
    void changeSnr(int index);
    void anotherSlotFromFile_Please(int num);
    void turnOnState(bool);
    void transmitConnectState(int addr1,int addr2);
    void sendProcentInfo(QString str);
    void loop_on_off(int);
    void allFreqsNumber(int num);  // изменение числа доступных для ППРЧ частот
    void freq_adapt_permition(bool perm); // разрешение на адаптацию по частоте

public slots:
    void  setModemNumber(int num); // присваивает модему номер
    int  getModemNumber() ;   // возвращает номер модема
    void stopGoTimer(bool mode);
    void dataOutputMode(bool mode);
    void setTimerCycle(int index);
    void clearBer();
    void setVariantIndex(int jj);
    void sendPlotData();
    bool setInputSignal(QVector<int_complex>*signal);
    void setInputSignalTest(QVector<int_complex>*signal);
    void setReadFileMode(bool state);
    void setWriteMode(bool state);
    bool fromFileToDemod();
    void setTestMode(bool state);
    void appendToConnectionList(int);   // добавить модем в списки для соединения и синхронизации
    int chanNumber_send();
    int chanNumber_recv();
    void startConnection();
    void set_turnOn_State(bool state);
    void set_turnOn_Plots(bool state);
    void setFrchFrequencies(int f1,int f2);
    void setPspChannels(int txCh,int rxCh);

    void fixProcentValue(int);
    void set_cbVariantIndex(int var);
    void send_data(char *data, int length);
    void define_modem_properties(MODEM_PROP *mProp);
    void set_cbAllFreq(const QString& str);  // переключение общего числа используемых ППРЧ частот
    void set_cbActiveFreq(const QString& str);// переключение числа активных ППРЧ частот
    void setTxChanVariant(int ch,int ii);  // установка в канал варианта на передачу
    void setRxChanVariant(int ch,int ii);  // установка в канал варианта на прием
    void modemReset();

private slots:
    void showVariantsInfo(int ii);
    void createVariantList(int var);
    void testFunctions();
    void modemChannelDemod();

    void startTimer(int state);
    void onTick();
    void showStatus(QString msg);
    void berMeInit(int state);
    void showChart(int sh);
    void shiftComp(int state);
    void OnOffState(int state);
    void initSynchro(int num=0);
    void setConnectParams(int psp_chan1, int psp_chan2);
    void initConnect(bool visa);
    void setVariantParams(int var);
    void setFrchParams();
    void convertVariantToParams(int var,connect_Pars_T* par);
    void showChvm(quint16 addr1,quint16 addr2,quint32 tChan,quint32 rChan);
    void setConnectMode(connect_Pars_T* pars);
    void setDuplex(int num);
    bool slotModulate();
    void slotDemodulate(bool rcv);
    void breakConnection();
    void switchToBestFrequency(int num);
    void setNewSampleRate();   // установить для текущего варианта СКК значение freq_distr из комбобокса cbSampleRate
    void autoBreakdown(int state);
    void speed_adapt_init(int state);
    void cb50(int state);  // поставить-убрать флаг у cbAuto50
    void showDecodProcent();
    void adapt_refresh();
    void sendData();
    void setActiveFreqsNumber(const QString& str);  // переключение числа активных частот
    void setAllFreqsNumber(const QString& str);  // переключение общего числа используемых ППРЧ частот
    void setCbActiveFreqIndex(int def);  // установка комбобокса CbActiveFreq в значение по умолчанию
    void setCbAllFreqIndex(int def);  // установка комбобокса CbAllFreq в значение по умолчанию
    void setLoopMode(bool state);
    void maybe_set_freq_adapt(bool perm); // если есть флаг и сигнал от переключения варианта
};


#endif // TSETTINGS_H
