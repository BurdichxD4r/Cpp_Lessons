#ifndef WIDGET_DCE_H
#define WIDGET_DCE_H

#include <QWidget>
#include <QJsonObject>
#include <QMap>
#include "dte_label.h"

#include "widgetmess.h"
#include "protocolServ.h"
#include "simpleplot.h"
#include <complex>
#include <QVector>
DTE_label

QT_BEGIN_NAMESPACE
namespace Ui { class Widget_DCE; }
QT_END_NAMESPACE

Q_DECLARE_METATYPE(NetConnectS)

typedef  std::complex<short> short_complex;
typedef  std::complex<int> int_complex;

// структура статуса создания файла журнала
struct LogFileStatS{
    bool    log_file_create_status;   // статус создания лог. файла (создавать / не создавать)
    QString log_file_name;            // имя лог. файла
};


const QString STR_ERR_MODEM ="Error, modem is not turnOn";  // ответ клиенту в случае отказа модема при запуске
const QString STR_ERR_RADIO ="Error, radio line is not turnOn"; // ответ клиенту в случае отказа подключ.к радио средствам

const int     NO_USE_PORT = 0;                        // заглушка для неиспользуемого порта (ПРМ<->ПРД)
const QString NO_USE_ADR = "000.000.000.000";         // заглушка для неиспользуемого адреса в сети (ПРМ<->ПРД)

class Widget_DCE : public QWidget
{
    Q_OBJECT

public:

    Widget_DCE(QWidget *parent = nullptr);
    ~Widget_DCE();


    void    setPort(int port);                 // установка порта Сервера
    void    setID_DCE(int id_dce);             // установка DCE_ID
    void    setID_DCE_DST(int id_dce_DST);     // установка DCE_ID_DST
    void    setTypeMsg(QString type_msg);      // установка типа сообщения
 //   void    setModemAndUdpParams(NetS portAdr, int port_bind,
 //                                int index_zpch,int m_start_buf,
 //                                int m_que_depth); // установка параметров UDP соединения
    void    setQueueDepth(int queDepth);

    // установка стартовой буфф., и глуб. очереди перед. слотов
   // void    setRatePass(int rate);             // установка скорости передачи

    QString getMsgPing();                      // доступ к ping-сообщению (поле формы)


    QString getRadioRole();                 // радио роль (RX)
    int     getPort();                      // порт прослушки TCP
    int     getID_DCE();                    // DCE_ID
    int     getServerStatus();              // статус сервера TCp запущен или остановлен(1-запущен, 0 -остановлен)

    void    startDCE();                     // запуск сервера DCE
    void    settAndStartDCE(QString msg_type, QStringList &other_params); // настройка DCE из dce.ini и автозапуск
    void    servDCEStartOnly(int port_tcp, QString msg_type);

    // QJsonObject             getSamplObj(const QString cmd, const QString suffix);                       // получение образца команды по cmd полю из списка command_list.txt
     //QMap<QString, QVariant> getCmdList();
public slots:

    void slotCmdFromClient(MsgDescrS & msg);                    // сигнал от ProtocolServ о пришедшей команде
    void slotCodoToRadio(QJsonObject & obj);                     // сигнал об отправки сообщ. в радио канал
   // void slotDisconnect();
    void slotGetNetAdrFromConnect(NetS &);
    void slotSendPackFromTransport(int, QByteArray &, QDateTime);// получение пакета сообщения из connectorDCE
    void slotSendErrFromConnect( QString & str, QTime tm);       // отправка данных о состоянии соединения (о возникших ошибках (или без ошибок!))
    void slotSendListClientFromConnect(QVector <NetConnectS>);   // список подключенных клиентов из модуля connectorDCE
    void slotSetTitleForm(QString title);                        // задание заголовка формы (по центру формы, крупнм шрифтом)
    void slotSetStatusForm(QString status_line);                 // задание статус-линии формы (внизу формы)
    void slotGetIQFromRadio(QVector<int_complex>*);              // получение IQ сигнала из Радио канала
    void slotDisconRadio();                                      // сигнал из Радио канале о дисконекте
    void slotIQSigToRadio(SlotFieldS &current_slot/*short_complex * signal, int len, int freq_num*/);                 // передача IQ сигнала в радио канал
    void slotGetSlotSize();                                      // обработка запроса радио канала о размере слота модема
    void slotPrepareEndAnsw(MsgDescrS & msg, int msg_in_que);
    void slotPrepareStartAnsw(MsgDescrS & msg);
    void slotPrepareEnqueuedAnsw(MsgDescrS & msg, int msg_in_que);
    void slotCmdExchangeDCEAnsw(QByteArray data_exch);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void slotGetCommandFromProt(MsgDescrS &msg);       // !!!обработка команды полученной из протокола
    void slotCommdReReady(MsgDescrS &msg);             // !!!обработка сигнала из внешнего прил. с командой ответом
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

private slots:

    void slotDCESetBusy(bool busy);      // установка значения busy
    void slotDCESetStage(int index);     // установка значения stage DCE

    void slotTitleChange(QString);       // изменение заголовка приложения
    void slotDCEIdChange(QString);       // изменение номера DCE_ID
    void slotJson();                     // задание типа сообщений как json
    void slotXml();                      // задание типа сообщений как xml
    void slotNetWorking();               // запуск/ остановка сервера
    void slotJournalView(bool);          // видимость журанала
    void slotDCE_ID_DST(bool);           // использование полей DCE_ID_DST, DCE_ID_SRC в команде ExchangeDCE
    void slotDepthChanged(QString  depth);

    void slotLogFileListChanged();       // изменение списка журналов для ведения

private:
    Ui::Widget_DCE *ui;

    int               m_mode_start{0};       // режим запуска модема (1-auto, 0 -cmd EnableDCE)
    int               m_port{0};             // номер порта сервера
    UVSParamS         m_uvs_obj;             // сервер-УВС. Состояние УВС (структура задается выше)
    ProtocolServ *    m_prtcl_serv{nullptr}; // сервер на уровне протокола
    QJsonObject       m_radio_param_obj;     // параметры радио средств
    QJsonObject       m_modem_param_obj;     // параметры модема
    TypeMsgEnum       m_type_msg = JSON;     // тип сообщения
    QColor            m_text_cl;             // цвет использованный в оформлении
    QString           m_ip_adr;              // ip на котором запущен сервер


    QVector <LogFileStatS> *m_vec_log_file; // вектор для хранения данных о статусе создания системы жураналирования

    // CoreDCE включается в проект на уровне дизайнера

    bool        checkDirExist(QString &dir_name_);
    bool         makeDir(QString &dir_name_);
    QJsonObject readSettFromIni();
    QJsonObject defaultSettLoad();
    void        saveSettToIni(QJsonObject &jObj);
    QJsonObject createSettingObj(QString fl_ini_name);
    QString     settFileExist();   // проверка существ. файла с настройками
    void        initWidget();
    void        startServ();        // запуск сервера
    void        stopServ();         // остановка сервера
    void        startRadio();       // запуск радио канала
    void        stopRadio();        // остановка радио канала
    QString     xmlCmd(const QString &);
    QString     jsonCmd(const QString &);
    void        totalDisconnect();
    QString     createTitle(QColor &cl, Qt::Alignment &al,int tx);
    void        addClientSign(QString client, int port, QString adr); // добавление значка подключившегося клиента на панель сервера
    void        clearClientPanel();                     // очистка панели списка подключенных клиентов
    QString     titleAndColorCreate(QColor &cl, Qt::Alignment &al,int tx);
    QString     getTypeCom(const QString &);
    void        msgCmdParse(MsgDescrS  & msg_obj);  // парсинг пришедшей команды
    void        cmdEnableDCEParser(MsgDescrS & msg, QJsonObject & modem, QJsonObject & radio);  // парсинг команды EnableDCE
    void        getCurrentSetting(QJsonObject & params_obj);
    void        turnOnRadio(QJsonObject & params_obj);
    bool        turnOnModem(QJsonObject & params_obj);
    qint32      getMsgSize(QString &data_field);
    int         decodingGettingObject(QJsonObject json_in);
    int         decodingDataLength(QJsonValue val);


   // обработка команды пришедшей от клиента
    void   cmdStatusProc(MsgDescrS & msg);
    void   cmdVerionProc(MsgDescrS & msg);
    void   cmdGetDCEInfoProc(MsgDescrS & msg);
    void   cmdGetRadioDirInfoProc(MsgDescrS & msg);
    void   cmdEnableDCEProc(MsgDescrS & msg);
    void   cmdDisableDCEProc(MsgDescrS & msg);
    void   cmdNetworkSYNCProc(MsgDescrS & msg);
    void   cmdNetworkCONNECTProc(MsgDescrS & msg);
    void   cmdNetworkDISCONProc(MsgDescrS & msg);
    void   cmdExchangeDCEProc(MsgDescrS & msg);
    void   cmdReplyDCEProc(MsgDescrS & msg);
   // формирование ответа клиенту
    void   cmdVerionAnsw(MsgDescrS & msg);
    void   cmdStatusAnsw(MsgDescrS & msg);
    void   cmdGetDCEInfoAnsw(MsgDescrS & msg);
    void   cmdGetRadioDirInfoAnsw(MsgDescrS & msg);
    void   cmdEnableDCEAnsw(MsgDescrS & msg);
    void   cmdDisableDCEAnsw(MsgDescrS & msg);
    void   cmdNetworkSYNCAnsw(MsgDescrS & msg);
    void   cmdNetworkCONNECTAnsw(MsgDescrS & msg);
    void   cmdNetworkDISCONAnsw(MsgDescrS & msg);
    void   cmdExchangeDCEAnsw(QByteArray data_exch);
    void   cmdReplyDCEAnsw(MsgDescrS & msg);
    void   processObject(QJsonObject & obj_in,   // объект принятый, сформированный из входящей последовательности
                                QJsonObject & obj_samp,  // объект эталонный, сформированный по данным m_com_list т.е.из файла command_list
                                ModeEnum mode, QString &err);           // режим обработки (CHECK - проверяется входящий объект на соответствие с эталонным,
                                                             //                  FILL_SAME - после того как входной объект проверен, по некоторым
                                                             //его данным + эталонный объект ->формируется формирутся объект отвертной команды)
signals:


    //////   Сигналы для ModemPanel о пришедших командах от клиента
    void    sigEnableDCE(MsgDescrS & msg);
    void    sigDisableDCE(MsgDescrS & msg);
    void    sigSYNC(MsgDescrS & msg);
    void    sigCONECT(MsgDescrS & msg);
    void    sigDISCON(MsgDescrS & msg);
    void    sigExchDCE(MsgDescrS & msg);
    void    sigCmdToClient(MsgDescrS & msg, QJsonObject & obj_param);  // сиг. в ProtocolServ о готовности данных для формирования ответа клинету
    void    sigCmdToClientFromRadio(QString cmd_name, QJsonObject &param_list_out); // инициативное сообщение из радио канала


    void    sigWriteInJourn(int, QDateTime, QString &, const QString &, QColor);  // необходима запись информации в журнал сообщений
  ///////////////////////////////////////////////////////////////////////////////////////////////
    void    sigDCEChangeStatus();   // сигнал в MainWindow о смене статуса работы сервера (запуск/остановка)
    void    sigStopServ();          // необходима остановка работающего сервера ProtocolServ
    void    sigDCEIdChng();         // изменения номера DCE_ID в MainWindow
    void    sigPortChng(QString);   // изменение номера порта сервера в MainWindow
    void    sigDCETypeChange();     // изменение типа сообщения

  ///////////////////////////////////////////////////////////////////////////////////////////////
    void    sigGetCommd(MsgDescrS & msg);       // !!!!была получена команда от DTE (protocolDCE)
    void    sigCommReToProt(MsgDescrS & msg);   // !!!!была получена команда с ответом(из внешнего прилож.)
 ////////////////////////////////////////////////////////////////////////////////////////////////   
    void    sigUseDST(bool);                    // использование полей DCE_ID_SRC, DCE_ID_DST



    void       sigNetAdr(NetS &);                  // передача Адреса в сети

    void       sigSetBusy(bool);                   // изменение поля BUSY
    void       sigSetStage(int);                   // изменение поля STAGE

    void       sigCodoToRadio(QJsonObject & obj);   // сигнал об отправки сообщения в радио канала
    void       sigIQToRadio( SlotFieldS &current_slot/*short_complex * signal, int len, int freq_num*/); // сигнал с данными в радио канал корреспонденту
    void       sigIQFromCorr(QVector<int_complex>*); // сигнал с данными из радио канала от корреспондента

    void       sigBfSht(qint32 lf_shift, qint32 rt_shift);
    void       sigIQEnergy(qreal energy);
    void       sigErrorTurnOn(QString msgErr, MsgDescrS & msg); // сигнал в ProtocolServ о наличии ошбок при запуске модема или радиосредств
};
#endif // WIDGET_DCE_H
