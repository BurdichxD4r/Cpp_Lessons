#ifndef WIDGETDTE_H
#define WIDGETDTE_H

#include <QWidget>
#include <QDialog>
#include <QMap>
#include <QTimer>
#include <QTime>

#include "protocolClient.h"
#include "widgetmess.h"
#include "enabledce_dial.h"
#include "disabledce_dial.h"
#include "netsync_dial.h"
#include "netconnect_dial.h"
#include "netdisconnect_dial.h"
#include "exchagedce_dial.h"
#include "stagemarkerbutton.h"         // модуль индикаторов состояния ПАК АУ

//#include "widgetAU.h"
//#include "widgetMAS.h"

const int STATE_BUTTON_WIDTH = 80;      // ширина "кнопки состояния"


struct AUParams{   // структура задающее состояние АУ
    uint32_t           DCE_ID{0};         // позывной привязанного УВС
    uint32_t           DCE_ID_OPPN{0};    // позывной получателя (оппонента)
    QString            create_time{""};   // время создания
    QString            enable_time{""};   // время включения
    PACStatusEnum      status{INIT};      // статус на текущий момент (INIT = 0, ENABLE, NET_SYNC, NET_CONNECT, EXCHENGE, NET_DISCON)
    Version            version;           // версия ПО, загруженная на ПАК АУ/МАС

};
QT_BEGIN_NAMESPACE
namespace Ui { class WidgetDTE; }
QT_END_NAMESPACE

Q_DECLARE_METATYPE(NetConnectS)
Q_DECLARE_METATYPE(ExchangeModeEnum)

class WidgetDTE : public QWidget
{
    Q_OBJECT

public:    
    explicit WidgetDTE(QWidget *parent = nullptr);
    ~WidgetDTE();
    void setPort(int port);
    void setIP(QString ip);

public slots:

    void slotCloseApp();

    void slotGetBitCount(int count);
    void slotPassBitCount(int count); // переданное кол-во бит очередной командой ExchangeDCE
    void slotErrBitCount(int count);
    void slotMsgInQueCount(int count);
    void slotBytesInQueCount(int count);

    void  slotDceInfo();
    void  slotDirInfo();
    void  slotNewConnect();  // создание нового соединения
    void  slotPing();        // создание команды PING
    void  slotTermType();    // создание команды TERMTYPE
    void  slotVersion();     // создание команды VERSION
    void  slotStatus();      // создание команды STATUS
    void  slotTestCmd();     // отправка не сущ. ком. TEST
    void  slotEnable();      // создание команды ENABLE
    void  slotDisable();     // создание команды DISABLE
    void  slotNetSync();     // создание команды NETSYNC
    void  slotNetConnect();  // создание команды NETCONNECT
    void  slotNetDisconnect(); // создание команды NETDISCONN
    void  slotExchange();     // создание команды Exchange

    void  slotPackFromTransport(int, QByteArray &, QDateTime); //  обработка сигнала из Protocol о пришедшем пакете из модуля connector
    void  slotErrorFromTransport(QString &, QTime);        //  обработка сигнала из Protocol о состоянии соединения из модуля connector

    void  slotDiscon();      // обработка разрыва соединения
    void  slotMas();         // обработка того факта что приложение будет работать как ПАК МАС
    void  slotAu();          // обработка того факта что приложение будет работать как ПАК АУ
    void  slotJson();        // обработка того факта что сообщение будет представлено в JSON формате
    void  slotXml();         // обработка того факта что сообщение будет представлено в XML формате
    void  slotNetWorking();  // обработка сознания соединения с ПАК УВС (сервером)
    void  slotPanelOn(StageMarkerButEnum, QString &free_param); // слот включения/ выключения кнопок на паненле состояний

private slots:
     void slotTimerMsgAlarm();            // слот таймера отправки сообщения ПАК УВС
     void slotTitleChange(QString);       // изменение заголовка приложения
     void slotTimerAuAlarm();            // обработка события таймера о завершении времени ожидания ответного сообщения от ПАК УВС
    // void slotTimerErrPerLetterAlarm();   // обработка события таймера отправки сообщений из ПАК МАС в ПАК УВС для расчета P(err/let)
     void slotStopTest();
     void slotInfo();                // обработка состояния rbInfo
     void slotTest();                // обработка состояния rbTest
     void slotJournalView(bool);     // видимость журнала сообщений
     void slotVaryData(bool);        // вариативность последовательности для передачи в ком. ExchangeDCE

private:
    Ui::WidgetDTE *ui;
    bool            m_timer_stop{false};  // флаг остановки таймера передачи инфо пакетов
    int             time_wait = 11000;    // (мс) время ожидания ответа на команду переданную в УВС
    int             time_info_pack{100};  // период между информ. пакетами
    AUParams        m_au_obj;             // основные параметры ПАК АУ (см. структуру выше)
    TypeMsgEnum     m_type_msg = JSON;    // формат сообщения (по умолчанию JSON)
    ClientTypeEnum  m_client_type = AU;   // тип клиента (по умолчанию АУ АСРЦ)
    ConnectTypeEnum m_connect_type = TCP; // тип соединения (по умолчанию TCP)
    QColor          m_text_cl;            // цвет объекта (красный - сервер, синий - клиент)
    QString         m_msg;                // сообщение для передачи
  //  QString         m_uuid_str;         // UUID сеанса связи
    ProtocolClient *m_prtcl_client{nullptr};       // протокол клиентской части взаимодействия
    QDialog        *m_dialog_com{nullptr};         // диалог выбора параметров команды
    QMap <QString, StageMarkerButton*> m_map_stage;// вектор панели индикаторов состояний
    QTimer         *m_autimer;            // таймер ожидания ответа
    QWidget        *m_wgt_cl;
    bool            m_varyable{true};     // флаг для формирования изменяемой последовательности для передачи в ком. ExchangeDCE
    quint64         m_vary_count{0};      // счетчик вариативных сообщений

   // WidgetMAS * m_wgt_mas;
   // WidgetAU   m_wgt_au;


    // ПАК МАС
    ExchangeModeEnum m_exch_mode{INFO_EXCH};    // значение режима работы ПАК МАС (INFO_EXCH - передача информации,
                                                // TEST_EXCH - тест на определение кол- во поврежд. бит)
    QJsonObject      m_object_json;
    double           m_P_err_lett{0.};           // счетчик вероятности ошибок

    int              m_byte_pass{0};              // счетчик переданных байт
    int              m_byte_get{0};               // счетчик принятых байт
    int              m_bit_err{0};                // счетчик бит принятых с ошибкой

    int              m_num_enq{0};                // кол-во бит в очереди при статусе : enqueued
    int              m_num_end{0};                // кол-во бит в очереди при статусе : end

    int              m_msg_queueu{};             //счетчик сообщений в очереди
    int              m_msg_queueu_pre{0};        //счетчик переданных сообщений (пока нет первого ответного сообщение)
    int              m_byte_queueu{0};           // счетчик байт в очереди
    QTimer           *m_msg_timer;               // таймер отправки сообщ. ПАК УВС

    ExchangeParamStruct m_exch_struct;

    void       totalDisconnect();                   // разрыв соединения
    QString    createTitle(QColor &cl, Qt::Alignment &al,int tx);  // функция формирования заголовка для отображения в информационном окне приложения
    void       initMainWind();                      // Настройки внешнего вида MainWindow
    void       panelStateCreate();                  // Создание панели состояний для ПАК АУ
    void       panelStateVisible(bool visible);     // Видимость панеле состояний

    QString    jsonCmd(const QString & str_json);   //  выделение наименования команды json
    QString    xmlCmd(const QString & str_xml);     //  выделение наименования команды xml
    QString    getTypeCom(const QString & str);     //  выделение команды из строки для отображения в таблице

    void       unlockEnableButtons();                   // разблокирование кнопок MW_DTE согласно текущему состоянию
    void       lockAllButtons();                        // блокировка все кнопок для запуска команды
    void       autoStartMAS();                          // автозапуск работы иммитатора в режиме ПАК МАС
    ExchangeParamStruct makeVaryableData(const ExchangeParamStruct & data_struct);  // подготовка вариативной последовательности для передачи в ком. ExchangeDCE

signals:
    void       sigWriteInJourn(int, QDateTime, QString &, const QString &, QColor); // сообщение следует записать в журнал
           //  signals to Protocol
    void       sigSendCommand(QJsonObject &);        // сигнал Protocol об отправке команды через Connector
    void       sigDiscon();                          // сигнал  об отключении от Сервера
    void       sigExchMode(int);//ExchangeModeEnum); // сигнал о режиме работы ПАК МАС (обмен инф. или тест канала)
    void       sigClientType(ClientTypeEnum);        // сигнал о типе подключ. клиента (МАС или АУ)

};

#endif // WIDGETDTE_H
