#ifndef PROTOCOLCLIENT_H
#define PROTOCOLCLIENT_H

#include <QObject>
#include "verificationClient.h"
#include "enums_list.h"
#include "structs_list.h"

//------------------------------------------------------------------
// структура инфообмена
struct InfoExchStruct{
    double  p_err_lett{0.};           // счетчик вероятности ошибок
    int     byte_pass{0};             // счетчик переданных символов
    int     byte_get{0};              // счетчик принятых символов
    int     bit_err{0};               // счетчик символов принятых с ошибкой
};

// структура пакета который сравнивают с эталоном
struct TestPackStruct{
    int  ampl{0};          // заданная ампл. пакета
    int  len {0};          // текущая длина пакета
    int  len_max{0};       // заданная длина пакета
    int  start{2};         // пакет начался (2-старт работы ПО, только начинаем принимать пакеты, 1- продолжение работы пакеты принимаются непрерывно)
    int  parity{2};        // четность текущего пакета
    int  parity_pre{2};    // четность на предыдушем шаге
    int  err{0};           // ошибок // кол-во ошибок
};

//-------------------------------------------------------------------

class ProtocolClient:public QObject
{
    Q_OBJECT
public:
    ProtocolClient(NetS &net_adrs,
                   ClientTypeEnum type_client = AU,   // тип подкл. клиента
                   TypeMsgEnum type_msg = JSON,      // тип сообщения
                   bool format_msg = false           // формат сообщение
            );
    ~ProtocolClient();
public slots:
    void slotPackFromTransport(int,  QByteArray &, QDateTime );
    void slotErrorFromTransport(QString &, QTime );
   // void slotPanelOn(QJsonObject &);
    void slotDiscon();
    void slotSendCommandDTE(QJsonObject &);
    void slotExchMode(int);//ExchangeModeEnum);
    void slotParityLenGet(int);
    void slotNoParityLenGet(int);
    void slotParityAmplGet(int);
    void slotNoParityAmplGet(int);

    bool dceInfoCmdSend();
    bool dirInfoCmdSend();
    bool termTypeCmdSend();
    bool pingCmdSend();
    bool versionCmdSend();
    bool statusCmdSend();
    bool testCmdSend();
    bool syncCmdSend(NetSYNCParamStruct * data_struct);
    bool connectCmdSend(NetCONNECTParamStruct *data_struct);
    bool disconCmdSend(NetDISCONParamStruct * data_struct);
    quint32 enableCmdSend(EnableParamStruct * data_struct);
    bool disableCmdSend(DisableParamStruct * data_struct);
    bool exchCmdSend(ExchangeParamStruct *data_struct);

    void fillCommandEnable(QJsonObject &jObj, EnableParamStruct *data_struct);
    void fillCommandDisable(QJsonObject &jObj , DisableParamStruct *data_struct);
    void fillCommandSynchro(QJsonObject &jObj, NetSYNCParamStruct *data_struct);
    void fillCommandConnect(QJsonObject &jObj, NetCONNECTParamStruct *data_struct);
    void fillCommandDiscon(QJsonObject &jObj, NetDISCONParamStruct *data_struct);
    void fillCommandExchange(QJsonObject &jObj, ExchangeParamStruct *data_struct);

private slots:
    void slotAnalyseMsgFromServer(MsgDescrS &msg);

private:
    VerificationClient *m_verify_client;        // объект верификации команды для отправки серверу
    ClientTypeEnum      m_client_type = AU;     // тип клиента (по умолчанию АУ АСРЦ)
    InfoExchStruct      m_info_exch;            // структура для хранения параметров инфообмена
    int                 m_msg_queue{0};
    int                 m_byte_queue{0};
    ExchangeModeEnum    m_mode_exch{INFO_EXCH};  // режим работы ПАК МАС

    TestPackStruct      m_test_pack;            // реестр для оценки ошибок в принятой последовательности в сравнении с ожидаемой послед.

    int                 m_parity_A{50};         // ампл. четной последовательности задается на форме
    int                 m_noparity_A{77};       // ампл. нечетной последовательности задается на форме
    int                 m_parity_L{32};         // длит. четной последовательности задается на форме
    int                 m_noparity_L{32};       // длит нечетной последовательности задается на форме
    bool                m_test_parity{true};    // чётность тестовой последовательности 0 пак.
    bool                m_parity_count{false};      // счетчик четности отправленного пакета (0й пак. - нечетный, т.к.~ MSG_ID=1)

    bool                m_par_i_1{true};     // четность. послед на пердыд. шаге
    bool                m_curr_start{false};  // начался очередной пакет
    bool                m_curr_end{false};    // закончился очередной пакет
    int                 m_curr_par{2};        // четность текущего пакета 0 - чет,  1 - нечет

    int         verifyImitsInst(const QJsonObject  & obj);           // проверка иммитовставки на стороне ПАК АУ
    void        analyseMsgFromServer(const QByteArray & jsonPack);   // пакет пришел от сервера
    QJsonObject prepareJsonComVerify(QJsonObject &jObj_in);
    QJsonObject prepareJsonComReceiveData(QJsonObject &jObj_in);     // подготовка ответа на команду ExchangeDCE от ПАК МАС

    bool        checkPreAnsw(QJsonObject & jsonObj);            // проверка принятой команды на то, что она является предварительной
    bool        checkPositiveAnsw(QJsonObject & jsonObj);       // проверка что полученный ответ соотв. штатной работе системы
    bool        checkStartAnsw(QJsonObject & jsonObj);          // проверка что полученный ответ соотв. команде со status: start
    bool        checkNetBusyAnsw(QJsonObject & jsonObj);        // проверка что полученный ответ соотв. команде со status: net busy
    bool        checkUserBusyAnsw(QJsonObject & jsonObj);       // проверка что полученный ответ соотв. команде со status: user busy
    bool        checkRecvAnsw(QJsonObject & jsonObj);           // проверка что полученный ответ соотв. команде со status: recv
    bool        checkEndAnsw(QJsonObject & jsonObj);            // проверка что полученный ответ соотв. команде со status: end
    bool        checkEnqueuedAnsw(QJsonObject & jsonObj);       // проверка что полученный ответ соотв. команде со status: enqueued(ПАК МАС)
    bool        checkDataAnsw(QJsonObject & jsonObj);           // проверка что полученный ответ соотв. команде со data.size > 0(ПАК МАС)
    bool        checkDisconAnsw(QJsonObject & jsonObj);

    bool       isDecSeq(QByteArray & ba);
    bool       isIncSeq(QByteArray & ba);
    int        getBitErr(QByteArray & ba_sampl);   // нахождение кол-ва ошибочных бит в ошибочном байте
    int        getErrorInObjParityNew(QByteArray & ba);
    int        getErrorInObj(QByteArray & ba);            // подсчет кол-во ошибок в принятой последовательности для P(err/letter)
    int        getErrorInObjParity(QByteArray & ba, int delta);      // четная послед.{0,2,4,}m_test_parity{true}
    int        getErrorInObjNoParity(QByteArray & ba);    // не четная послед.{1,3,5,...} m_test_parity{false}
    QBitArray  byteToBit(QByteArray & ba);                // преобразование байтов в биты
    QString    getChannel(QJsonObject & jsonObj);         // выделение из ответной команды NetworkConnect предлагаемых каналов приёма/передачи
    int        decodingGettingObject(QJsonObject json_in);// декодирование принятого от клиента сообщения
    int        decodingDataLength(QJsonValue val);        // размер передаваемых данных после декодирования
    void       cutFirstSeq(QByteArray & ba);
signals:
    void sigSendCommand(QJsonObject &json_obj);
    void sigDiscon();
    void sigAddMsgToJournal(int tx, QByteArray & ba, QDateTime tm);
    void sigChangeWorkStatus(QString & str, QTime tm);
    //void sigPanelOnDTE(QJsonObject & jsonObj);
    void sigPanelOnDTE(StageMarkerButEnum, QString &free_val);
    //  void        sigMsgReadyDTE(QByteArray &);       // сигнал о том что  пакет собран, для MW
    void sigPassBit(int num);
    void sigGetBit(int num);
    void sigErrBit(int num);
    void sigBytesInQue(int num);
    void sigMsgInQue(int num);
};

#endif // PROTOCOLCLIENT_H
