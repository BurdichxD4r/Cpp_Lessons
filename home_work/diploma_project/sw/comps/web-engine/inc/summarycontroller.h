// Модуль для формирования вкладки SUMMARY
#ifndef SUMMARYCONTROLLER_H
#define SUMMARYCONTROLLER_H

#include <httprequesthandler.h>

using namespace stefanfrings;


struct ModemStruct{  // структура для описания состояния модема
    qint32    DCE_ID{0};         // номер модема
    qint32    port {2323};       // порт
    QString   IP {"127.0.0.1"};  // адрес в сети
    QString   radio_type{"RX"};  // роль в сети (tx- передающий, rx-приемный)
    qint64    creation_time{0};  // время создания
    qint64    enable_time{0};    // время включения
    QString   stage{""};         // стадия работы (init, enable, sync, connect, exch)
    QString   busy{"no"};        // флаг занятости(yes, no)
    qint32    regim{0};          // режим работы

};

struct ClientStruct{     // структура для описания подключенного клинета (служба ПАК АУ)
    qint32   port {2323};       // порт клиента
    qint64   connect_time{0};   // время подключения
    QString  type{"AU"};        // тип подключенного клиента
};

class SummaryController : public HttpRequestHandler
{
    Q_OBJECT
public:
    SummaryController(const QSettings* settings,QObject *parent = nullptr);
    void     service(HttpRequest& request, HttpResponse& response);
private:

    /** Root directory of testing data */
    QString m_testroot;

    /** Encoding of text files */
    QString m_encoding;

     /** Name of file with testing data */
    QString m_sammname;

    QMap<QString, QVariant>  m_test_list;         // список подключенных клиентов, доступных УВС, настроек системы из файла "test_summary_sett.txt"
    QVector<ModemStruct>     m_modem_list;        // массив данных о доступных модемов
    QVector<ClientStruct>    m_client_list;       // массив данных о подключенных клиентов

    void                  createTestSummaryList(); // создание списка полей, считанных из файла.json
    QJsonObject           getObject(const QString objName); // формирование json объекта по списку полей считанных из файла
    QVector<ModemStruct>  getModemList();         // получим список всех доступных модемов
    QVector<ClientStruct> getClientList();        // получим список всех подключенных клиентов
};

#endif // SUMMARYCONTROLLER_H
