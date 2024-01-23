// вкладка Настройка системы
#include "settingcontroller.h"
#include "global.h"
#include <QTime>
#include <QDir>

#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include "template.h"

//---------------------------------------------------------------------------
SettingController::SettingController(const QSettings* settings,QObject *parent)
    : HttpRequestHandler{parent}
{
    m_encoding=settings->value("encoding","UTF-8").toString();
    m_testroot=settings->value("path",".").toString();
    m_settname=settings->value("settName",".").toString();

    if(!(m_testroot.startsWith(":/") || m_testroot.startsWith("qrc://")))
    {
        // Convert relative path to absolute, based on the directory of the config file.
        #ifdef Q_OS_WIN32
            if (QDir::isRelativePath(m_testroot) && settings->format()!=QSettings::NativeFormat)
        #else
            if (QDir::isRelativePath(m_testroot))
        #endif
        {
            QFileInfo configFile(settings->fileName());
            m_testroot=QFileInfo(configFile.absolutePath(), m_testroot).absoluteFilePath();
        }
    }
    qDebug("SettingController: testroot=%s, encoding=%s filename=%s",qPrintable(m_testroot),qPrintable(m_encoding),qPrintable(m_settname));

}
//---------------------------------------------------------------------------
void SettingController::service(HttpRequest &request, HttpResponse &response)
{
    HttpSession session = sessionStore->getSession(request,response,true);
    qDebug()<<session.getId();

    createTestSettingList();         // создаем список параметров настройки системы
    m_sett_list = getSettingList();   // заполняем структуру настроки системы из файла

    response.setHeader("Content-Type", "text/html; charset=UTF-8");

    for (int i = 0; i < m_sett_list.size(); i++){
        qDebug("ping_period=%i",    m_sett_list.at(i).ping_period);
        qDebug("version_period=%i", m_sett_list.at(i).version_period);
    }

    QByteArray language = request.getHeader("Accept-Language");
    Template t = templateCache->getTemplate("sett_templ",language);

    t.loop("sett", m_sett_list.size());
    if (session.contains("username")) {           // аутентификация пользователя пройдена
        if (!session.contains("ping_period")){     // на вкладку Settings заходим впервые за сессию
            QByteArray username=session.get("username").toByteArray();
            QTime logintime=session.get("logintime").toTime();
            t.setVariable("username",username);
            t.setVariable("logintime",logintime.toString("HH:mm:ss"));
            for (int i = 0; i < m_sett_list.count(); i++){
                QString number = QString::number(i);
                QString numb = QString::number(i + 1);

                t.setVariable("sett" + number + ".DCE_serial_number",           QString::number(m_sett_list.at(i).DCE_serial_number));
                t.setVariable("sett" + number + ".ping_period",                 QString::number(m_sett_list.at(i).ping_period));
                t.setVariable("sett" + number + ".version_period",              QString::number(m_sett_list.at(i).version_period));
                t.setVariable("version_mess" + numb,                QString::number(m_sett_list.at(i).version_mess));
                t.setVariable("sett" + number + ".time_messeges",               QString::number(m_sett_list.at(i).time_messeges));
                t.setVariable("sett" + number + ".type_protocol",               QString::number(m_sett_list.at(i).type_protocol));
                t.setVariable("sett" + number + ".DTE_list_ports",              QString::number(m_sett_list.at(i).DTE_list_ports));
                t.setVariable("sett" + number + ".DCE_list_ports",              QString::number(m_sett_list.at(i).DCE_list_ports));
                t.setVariable("sett" + number + ".DCE_list_ip",                 m_sett_list.at(i).DCE_list_ip);
                t.setVariable("sett" + number + ".DCE_port_number",             QString::number(m_sett_list.at(i).DCE_port_number));
                t.setVariable("sett" + number + ".DCE_ip_number",               QString::number(m_sett_list.at(i).DCE_ip_number));
                t.setVariable("sett" + number + ".role_in_network",             QString::number(m_sett_list.at(i).role_in_network));
                t.setVariable("sett" + number + ".status_polling_time_period",  QString::number(m_sett_list.at(i).status_polling_time_period));
                t.setVariable("sett" + number + ".max_connection_attempts_DCE", QString::number(m_sett_list.at(i).max_connection_attempts_DCE));
                t.setVariable("sett" + number + ".waiting_time_status_DCE",     QString::number(m_sett_list.at(i).waiting_time_status_DCE));
                t.setVariable("sett" + number + ".waiting_time_recovery_DCE",   QString::number(m_sett_list.at(i).waiting_time_recovery_DCE));

                session.set("DCE_serial_number",            m_sett_list.at(i).DCE_serial_number);
                session.set("ping_period",                  m_sett_list.at(i).ping_period);
                session.set("version_period",               m_sett_list.at(i).version_period);
                session.set("version_mess",                 m_sett_list.at(i).version_mess);
                session.set("time_messeges",                m_sett_list.at(i).time_messeges);
                session.set("type_protocol",                m_sett_list.at(i).type_protocol);
                session.set("DTE_list_ports",               m_sett_list.at(i).DTE_list_ports);
                session.set("DCE_list_ports",               m_sett_list.at(i).DCE_list_ports);
                session.set("DCE_list_ip",                  m_sett_list.at(i).DCE_list_ip);
                session.set("DCE_port_number",              m_sett_list.at(i).DCE_port_number);
                session.set("DCE_ip_number",                m_sett_list.at(i).DCE_ip_number);
                session.set("role_in_network",              m_sett_list.at(i).role_in_network);
                session.set("status_polling_time_period",   m_sett_list.at(i).status_polling_time_period);
                session.set("max_connection_attempts_DCE",  m_sett_list.at(i).max_connection_attempts_DCE);
                session.set("waiting_time_status_DCE",      m_sett_list.at(i).waiting_time_status_DCE);
                session.set("waiting_time_recovery_DCE",    m_sett_list.at(i).waiting_time_recovery_DCE);
            }
            setSettingList(m_sett_list, m_json_obj);
            saveTestSettingList(m_json_obj);
        } else {   // на вкладку Настройки заходим не впервые уже сохранено знач. ping_period в объекте сессия
            QByteArray username=session.get("username").toByteArray();
            QTime logintime=session.get("logintime").toTime();
            t.setVariable("username",username);
            t.setVariable("logintime",logintime.toString("HH:mm:ss"));
            for (int i = 0; i < m_sett_list.size(); i++){
                // возможно пользователь уже изменил часть или все значения настроек
                // эти изменения должны быть обработаны сервером и записаны в файл
                const QString number  = QString::number(i);
                const QString numb  = QString::number(i + 1);
                const std::string num = std::to_string(i + 1);

                const QByteArray ping_periodID =                    QByteArray::fromStdString("ping_period" + num);
                const QByteArray version_periodID =                 QByteArray::fromStdString("version_period" + num);
                const QByteArray version_messID =                   QByteArray::fromStdString("version_mess" + num);
                const QByteArray time_messegesID =                  QByteArray::fromStdString("time_messeges" + num);
                const QByteArray type_protocolID =                  QByteArray::fromStdString("type_protocol" + num);
                const QByteArray DTE_list_portsID =                 QByteArray::fromStdString("DTE_list_ports" + num);
                const QByteArray DCE_list_portsID =                 QByteArray::fromStdString("DCE_list_ports" + num);
                const QByteArray DCE_list_ipID =                    QByteArray::fromStdString("DCE_list_ip" + num);
                const QByteArray DCE_port_numberID =                QByteArray::fromStdString("DCE_port_number" + num);
                const QByteArray DCE_ip_numberID =                  QByteArray::fromStdString("DCE_ip_number" + num);
                const QByteArray role_in_networkID =                QByteArray::fromStdString("role_in_network" + num);
                const QByteArray status_polling_time_periodID =     QByteArray::fromStdString("status_polling_time_period" + num);
                const QByteArray max_connection_attempts_DCEID =    QByteArray::fromStdString("max_connection_attempts_DCE" + num);
                const QByteArray waiting_time_status_DCEID =        QByteArray::fromStdString("waiting_time_status_DCE" + num);
                const QByteArray waiting_time_recovery_DCEID =      QByteArray::fromStdString("waiting_time_recovery_DCE" + num);

                m_sett_list[i].ping_period =                    request.getParameter(ping_periodID).toInt();
                m_sett_list[i].version_period =                 request.getParameter(version_periodID).toInt();
                m_sett_list[i].version_mess =                   request.getParameter(version_messID).toInt();
                m_sett_list[i].time_messeges =                  request.getParameter(time_messegesID).toInt();
                m_sett_list[i].type_protocol =                  request.getParameter(type_protocolID).toInt();
                m_sett_list[i].DTE_list_ports =                 request.getParameter(DTE_list_portsID).toInt();
                m_sett_list[i].DCE_list_ports =                 request.getParameter(DCE_list_portsID).toInt();
                m_sett_list[i].DCE_list_ip =                    request.getParameter(DCE_list_ipID);
                m_sett_list[i].DCE_port_number =                request.getParameter(DCE_port_numberID).toInt();
                m_sett_list[i].DCE_ip_number =                  request.getParameter(DCE_ip_numberID).toInt();
                m_sett_list[i].role_in_network =                request.getParameter(role_in_networkID).toInt();
                m_sett_list[i].status_polling_time_period =     request.getParameter(status_polling_time_periodID).toInt();
                m_sett_list[i].max_connection_attempts_DCE =    request.getParameter(max_connection_attempts_DCEID).toInt();
                m_sett_list[i].waiting_time_status_DCE =        request.getParameter(waiting_time_status_DCEID).toInt();
                m_sett_list[i].waiting_time_recovery_DCE =      request.getParameter(waiting_time_recovery_DCEID).toInt();

                if (request.getBody().count() == 0) {
                    m_sett_list[i].DCE_serial_number =              session.get("DCE_serial_number").toInt();
                    m_sett_list[i].ping_period =                    session.get(ping_periodID).toInt();
                    m_sett_list[i].version_period =                 session.get("version_period").toInt();
                    m_sett_list[i].version_mess =                   session.get("version_mess").toInt();
                    m_sett_list[i].time_messeges =                  session.get("time_messeges").toInt();
                    m_sett_list[i].type_protocol =                  session.get("type_protocol").toInt();
                    m_sett_list[i].DTE_list_ports =                 session.get("DTE_list_ports").toInt();
                    m_sett_list[i].DCE_list_ports =                 session.get("DCE_list_ports").toInt();
                    m_sett_list[i].DCE_list_ip =                    session.get("DCE_list_ip").toString();
                    m_sett_list[i].DCE_port_number =                session.get("DCE_port_number").toInt();
                    m_sett_list[i].DCE_ip_number =                  session.get("DCE_ip_number").toInt();
                    m_sett_list[i].role_in_network =                session.get("role_in_network").toInt();
                    m_sett_list[i].status_polling_time_period =     session.get("status_polling_time_period").toInt();
                    m_sett_list[i].max_connection_attempts_DCE =    session.get("max_connection_attempts_DCE").toInt();
                    m_sett_list[i].waiting_time_status_DCE =        session.get("waiting_time_status_DCE").toInt();
                    m_sett_list[i].waiting_time_recovery_DCE =      session.get("waiting_time_recovery_DCE").toInt();
                } else {
                    session.set("DCE_serial_number",            m_sett_list.at(i).DCE_serial_number);
                    session.set("ping_period",                  m_sett_list.at(i).ping_period);
                    session.set("version_period",               m_sett_list.at(i).version_period);
                    session.set("version_mess",                 m_sett_list.at(i).version_mess);
                    session.set("time_messeges",                m_sett_list.at(i).time_messeges);
                    session.set("type_protocol",                m_sett_list.at(i).type_protocol);
                    session.set("DTE_list_ports",               m_sett_list.at(i).DTE_list_ports);
                    session.set("DCE_list_ports",               m_sett_list.at(i).DCE_list_ports);
                    session.set("DCE_list_ip",                  m_sett_list.at(i).DCE_list_ip);
                    session.set("DCE_port_number",              m_sett_list.at(i).DCE_port_number);
                    session.set("DCE_ip_number",                m_sett_list.at(i).DCE_ip_number);
                    session.set("role_in_network",              m_sett_list.at(i).role_in_network);
                    session.set("status_polling_time_period",   m_sett_list.at(i).status_polling_time_period);
                    session.set("max_connection_attempts_DCE",  m_sett_list.at(i).max_connection_attempts_DCE);
                    session.set("waiting_time_status_DCE",      m_sett_list.at(i).waiting_time_status_DCE);
                    session.set("waiting_time_recovery_DCE",    m_sett_list.at(i).waiting_time_recovery_DCE);
                }
                t.setVariable("sett" + number + ".DCE_serial_number",           QString::number(m_sett_list.at(i).DCE_serial_number));
                t.setVariable("sett" + number + ".ping_period",                 QString::number(m_sett_list.at(i).ping_period));
                t.setVariable("sett" + number + ".version_period",              QString::number(m_sett_list.at(i).version_period));
                t.setVariable("version_mess" + numb,                QString::number(m_sett_list.at(i).version_mess));
                t.setVariable("sett" + number + ".time_messeges",               QString::number(m_sett_list.at(i).time_messeges));
                t.setVariable("sett" + number + ".type_protocol",               QString::number(m_sett_list.at(i).type_protocol));
                t.setVariable("sett" + number + ".DTE_list_ports",              QString::number(m_sett_list.at(i).DTE_list_ports));
                t.setVariable("sett" + number + ".DCE_list_ports",              QString::number(m_sett_list.at(i).DCE_list_ports));
                t.setVariable("sett" + number + ".DCE_list_ip",                 m_sett_list.at(i).DCE_list_ip);
                t.setVariable("sett" + number + ".DCE_port_number",             QString::number(m_sett_list.at(i).DCE_port_number));
                t.setVariable("sett" + number + ".DCE_ip_number",               QString::number(m_sett_list.at(i).DCE_ip_number));
                t.setVariable("sett" + number + ".role_in_network",             QString::number(m_sett_list.at(i).role_in_network));
                t.setVariable("sett" + number + ".status_polling_time_period",  QString::number(m_sett_list.at(i).status_polling_time_period));
                t.setVariable("sett" + number + ".max_connection_attempts_DCE", QString::number(m_sett_list.at(i).max_connection_attempts_DCE));
                t.setVariable("sett" + number + ".waiting_time_status_DCE",     QString::number(m_sett_list.at(i).waiting_time_status_DCE));
                t.setVariable("sett" + number + ".waiting_time_recovery_DCE",   QString::number(m_sett_list.at(i).waiting_time_recovery_DCE));
            }
            setSettingList(m_sett_list, m_json_obj);
            saveTestSettingList(m_json_obj);
        }
        response.write(t.toUtf8(),true);
   }
}
//--------------------------------------------------------------------------
// процедура создания списка подключенных клиентов, доступных УВС, списка настроек ММ
// создается по данным файла  "test_summary_sett.txt"
void SettingController::createTestSettingList()
{
    QString fl_path = m_testroot +"/"+ m_settname;

    QFile file (fl_path);
    QString json_str;
    QJsonParseError parseErr;
    QString error = "";

    if (!file.exists()){
      qDebug()<<"File: test_sett.txt - does not exist!";
    }
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) // Проверяем, возможно ли открыть наш файл для чтения
            return;                                      // если это сделать невозможно, то завершаем функцию
       json_str = file.readAll();                         //считываем все данные с файла в объект data
    file.close();

    QJsonDocument doc;

    doc = QJsonDocument::fromJson(json_str.toUtf8(), & parseErr);
    if (parseErr.error != QJsonParseError::NoError){
       error = parseErr.errorString();//"JSON-ошибка";
      qDebug()<< "In file: test_sett.txt - JSON error! ("+error+")";
      }
    else if(doc.isObject()){
        QJsonObject root_obj = doc.object();
        // m_json_obj = doc.object();
    // делаем обход всего документа json,считывая имя команды и получая соответствующий объект команды
    // записываем все это в QVariantMap<QString, QVariant(QJsonObject)> m_test_list
         QString  comName;
         QVariant comObj;
         for(int i = 0; i < root_obj.count(); ++i){
            comName = root_obj.keys().at(i);
            comObj = root_obj.value(comName).toObject();
            m_test_list.insert(comName,comObj);
        }
    }
}
//-----------------------------------------------------------------------------------
// сохранение json в файл "test_summary_sett.txt"
void SettingController::saveTestSettingList(QJsonObject &jObj)
{
   QString fl_path = m_testroot +"/"+ m_settname;
   QJsonDocument jDoc(jObj);
   QFile file(fl_path);
   if(file.open(QIODevice::WriteOnly)){
      file.write(QString(jDoc.toJson()).toStdString().c_str());
      file.close();
    }
}
//-----------------------------------------------------------------------------------
// установка измененных параметров установок в jsonObj
int SettingController::setSettingList(QVector<SettingStruct> & settings, QJsonObject &jObj)
{
    QJsonObject obj_settings = getObject("Settings");
    QJsonObject settingsObj = obj_settings;
    QJsonArray sett_arr = settingsObj["params"].toArray();

    for (int i = 0; i < sett_arr.count(); i++){
        QJsonObject sett_item_obj = sett_arr.at(i).toObject();

        sett_item_obj.insert("serialNumberDCE", settings.at(i).DCE_serial_number);
        sett_item_obj.insert("pingPeriod", settings.at(i).ping_period);
        sett_item_obj.insert("versionPeriod", settings.at(i).version_period);
        sett_item_obj.insert("versionMess", settings.at(i).version_mess);
        sett_item_obj.insert("timeMesseges", settings.at(i).time_messeges);
        sett_item_obj.insert("typeProtocol", settings.at(i).type_protocol);
        sett_item_obj.insert("listPortsDTE", settings.at(i).DTE_list_ports);
        sett_item_obj.insert("listPortsDCE", settings.at(i).DCE_list_ports);
        sett_item_obj.insert("listIpDCE", settings.at(i).DCE_list_ip);
        sett_item_obj.insert("portNumberDCE", settings.at(i).DCE_port_number);
        sett_item_obj.insert("ipNumberDCE", settings.at(i).DCE_ip_number);
        sett_item_obj.insert("roleInNetwork", settings.at(i).role_in_network);
        sett_item_obj.insert("statusPollingTimePeriod", settings.at(i).status_polling_time_period);
        sett_item_obj.insert("maxConnectionAttemptsDCE", settings.at(i).max_connection_attempts_DCE);
        sett_item_obj.insert("waitingTimeStatusDCE", settings.at(i).waiting_time_status_DCE);
        sett_item_obj.insert("waitingTimeRecoveryDCE", settings.at(i).waiting_time_recovery_DCE);

        sett_arr[i] = sett_item_obj;
    }
    settingsObj["params"] = sett_arr;
    jObj["Settings"] = settingsObj;

    return 1;
}
//-----------------------------------------------------------------------------------
QVector<SettingStruct> SettingController::getSettingList()
{
    QJsonObject obj_settings = getObject("Settings");

    QVector<SettingStruct> sett_list;
    SettingStruct sett_item;
    QJsonObject sett_item_obj;

    QJsonArray sett_arr = obj_settings["params"].toArray();

    for (int i = 0; i < sett_arr.count(); ++i){
        sett_item_obj = sett_arr.at(i).toObject();

        if (sett_item_obj.contains("serialNumberDCE")){
                sett_item.DCE_serial_number = sett_item_obj.value("serialNumberDCE").toInt();
        }
        if (sett_item_obj.contains("pingPeriod")){
                sett_item.ping_period = sett_item_obj.value("pingPeriod").toInt();
            }
        if (sett_item_obj.contains("versionPeriod")){
                sett_item.version_period = sett_item_obj.value("versionPeriod").toInt();
            }
        if (sett_item_obj.contains("versionMess")){
                sett_item.version_mess = sett_item_obj.value("versionMess").toInt();
            }
        if (sett_item_obj.contains("timeMesseges")){
                sett_item.time_messeges = sett_item_obj.value("timeMesseges").toInt();
            }
        if (sett_item_obj.contains("typeProtocol")){
                sett_item.type_protocol = sett_item_obj.value("typeProtocol").toInt();
            }
        if (sett_item_obj.contains("listPortsDTE")){
                sett_item.DTE_list_ports = sett_item_obj.value("listPortsDTE").toInt();
            }
        if (sett_item_obj.contains("listPortsDCE")){
                sett_item.DCE_list_ports = sett_item_obj.value("listPortsDCE").toInt();
            }
        if (sett_item_obj.contains("listIpDCE")){
                sett_item.DCE_list_ip = sett_item_obj.value("listIpDCE").toString();
            }
        if (sett_item_obj.contains("portNumberDCE")){
                sett_item.DCE_port_number = sett_item_obj.value("portNumberDCE").toInt();
            }
        if (sett_item_obj.contains("ipNumberDCE")){
                sett_item.DCE_ip_number = sett_item_obj.value("ipNumberDCE").toInt();
            }
        if (sett_item_obj.contains("roleInNetwork")){
                sett_item.role_in_network = sett_item_obj.value("roleInNetwork").toInt();
            }
        if (sett_item_obj.contains("statusPollingTimePeriod")){
                sett_item.status_polling_time_period = sett_item_obj.value("statusPollingTimePeriod").toInt();
            }
        if (sett_item_obj.contains("maxConnectionAttemptsDCE")){
                sett_item.max_connection_attempts_DCE = sett_item_obj.value("maxConnectionAttemptsDCE").toInt();
            }
        if (sett_item_obj.contains("waitingTimeStatusDCE")){
                sett_item.waiting_time_status_DCE = sett_item_obj.value("waitingTimeStatusDCE").toInt();
            }
        if (sett_item_obj.contains("waitingTimeRecoveryDCE")){
                sett_item.waiting_time_recovery_DCE = sett_item_obj.value("waitingTimeRecoveryDCE").toInt();
            }

    sett_list.append(sett_item);
    }

   return sett_list;
}
//--------------------------------------------------------------------------------------------------
QJsonObject SettingController::getObject(const QString objName)
{
   QJsonObject jObj;
   jObj = m_test_list.value(objName).toJsonObject();
   return jObj;
}
//---------------------------------------------------------------------------------------------------
