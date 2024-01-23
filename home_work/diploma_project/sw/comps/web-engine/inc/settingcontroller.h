// модуль для формирования вкладки Settings - настройка работы системы
#ifndef SETTINGCONTROLLER_H
#define SETTINGCONTROLLER_H

#include <httprequesthandler.h>
#include <QJsonObject>

using namespace stefanfrings;

struct SettingStruct{       // структура для описания настроек (ModemManager)
    qint32   DCE_serial_number{0};           // порядковый номер DCE (1-16)
    qint32   ping_period {0};                // период отправки команды PING (сек)
    qint32   version_period{0};              // период отправки команды Version (Сек)
    qint32   version_mess{0};                // выдавать или нет сообщение о разнице в вервии ПО(1-да, 0-нет)
    qint32   time_messeges{0};               // max время нахождения сообщения в очереди (ms)
    qint32   type_protocol{0};               // тип протокола (XML|JSON)
    qint32   DTE_list_ports{0};              // список портов, используемых для подключения DTE
    qint32   DCE_list_ports{0};              // список портов, используемых для подключения DCE
    QString  DCE_list_ip{""};                // список ip, используемых для подключения к DCE
    qint32   DCE_port_number{0};             // номер порта из DCE_list_ports, используемый для подключения к DCE
    qint32   DCE_ip_number{0};               // номер порта из DCE_ip_ports, используемый для подключения к DCE
    qint32   role_in_network{0};             // радио роль в сети (принимающий (RX)/ передающий (TX)) (RX|TX)
    qint32   status_polling_time_period{0};  // период времени опроса состояния (отправки сообщения «Status») DCE (ms)
    qint32   max_connection_attempts_DCE{0}; // max кол-во попыток соединения с DCE за 1 цикл опроса состояния (ед)
    qint32   waiting_time_status_DCE{0};     // время ожидания ответа на сообщение «Status» от DCE (ms)
    qint32   waiting_time_recovery_DCE{0};   // время ожидания ответа на сообщение о восстановлении состояния DCE (ms)
};

class SettingController : public HttpRequestHandler
{
    Q_OBJECT
public:
    SettingController(const QSettings* settings, QObject *parent = nullptr);

    void service(HttpRequest& request, HttpResponse& response);   // обработка всевозможных запростов к старницы настроек

private:

    /** Root directory of testing data */
    QString m_testroot;

    /** Encoding of text files */
    QString m_encoding;

     /** Name of file with testing data */
    QString m_settname;

    QMap<QString, QVariant>  m_test_list;         // список подключенных клиентов и доступных УВС, настроек ММ "test_summary_sett.txt"
    QVector<SettingStruct>   m_sett_list;       // стуктура настройки работы ММ с подключенными модемами и клиентами
    QJsonObject              m_json_obj;          // json объект для хранения настроек

    void                     createTestSettingList();                // процедура создание списка настроек системы
    QVector<SettingStruct>   getSettingList();        // считаем значения настроек из файла
    QJsonObject              getObject(const QString objName);       // формирование json oбъекта из полей считанных из файла
    void                     saveTestSettingList(QJsonObject &jObj); // сохранение измененных настроек в файл
    int                      setSettingList(QVector<SettingStruct> & settings, QJsonObject &jObj); // установка измененных параметров установок в jsonObj
};

#endif // SETTINGCONTROLLER_H
