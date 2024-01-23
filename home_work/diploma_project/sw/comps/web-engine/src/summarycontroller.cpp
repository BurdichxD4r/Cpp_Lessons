// вкладка отображения данных о состоянии системы на текущий момент времени
#include "summarycontroller.h"

#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QCoreApplication>
#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include "template.h"
#include "global.h"
#include <QDateTime>


//const QString TEST_FILE_NAME = "test_summary_sett.txt";

//--------------------------------------------------------------------
SummaryController::SummaryController(const QSettings* settings, QObject* parent)
    : HttpRequestHandler{parent}
{
    m_encoding=settings->value("encoding","UTF-8").toString();
    m_testroot=settings->value("path",".").toString();
    m_sammname=settings->value("sammName",".").toString();

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
    qDebug("SummaryController: testroot=%s, encoding=%s filename=%s",qPrintable(m_testroot),qPrintable(m_encoding),qPrintable(m_sammname));

}
//-------------------------------------------------------------------
void SummaryController::service(HttpRequest &request, HttpResponse &response)
{
    HttpSession session=sessionStore->getSession(request,response,true);
    QString username=session.get("username").toString();

    createTestSummaryList();   // создаем список клиентов и УВС для отображения на форме
    m_modem_list  = getModemList();
    m_client_list = getClientList();

    response.setHeader("Content-Type", "text/html; charset=UTF-8");

    QByteArray language=request.getHeader("Accept-Language");
    Template t=templateCache->getTemplate("summary_templ",language);
    t.setVariable("name",username);
    t.setCondition("logged-in",!username.isEmpty());

    t.loop("row",m_modem_list.count());
        for(int i=0; i<m_modem_list.count(); i++) {
            QString number = QString::number(i);
            QString num = QString::number(i+1);
            QString dce_id = QString::number(m_modem_list.at(i).DCE_ID);
            QString port   = QString::number(m_modem_list.at(i).port);
            QString ip    = m_modem_list.at(i).IP;
            QString radio_type    = m_modem_list.at(i).radio_type;
            QDateTime cdt = QDateTime::fromMSecsSinceEpoch(m_modem_list.at(i).creation_time);
            QString creation_time   = cdt.toLocalTime().toString("yyyy-MM-dd hh:mm:ss.z");
            QDateTime cdt1 = QDateTime::fromMSecsSinceEpoch(m_modem_list.at(i).enable_time);
            QString enable_time   = cdt1.toLocalTime().toString("yyyy-MM-dd hh:mm:ss.z");
            QString stage    = m_modem_list.at(i).stage;
            QString busy    = m_modem_list.at(i).busy;
            QString regim   = QString::number(m_modem_list.at(i).regim);


            t.setVariable("row"+number+".number",num);
            t.setVariable("row"+number+".dce_id",dce_id);
            t.setVariable("row"+number+".port",port);
            t.setVariable("row"+number+".IP",ip);
            t.setVariable("row"+number+".radioType",radio_type);
            t.setVariable("row"+number+".creationTime",creation_time);
            t.setVariable("row"+number+".enableTime",enable_time);
            t.setVariable("row"+number+".stage",stage);
            t.setVariable("row"+number+".busy",busy);
            t.setVariable("row"+number+".regim",regim);
        }

    t.loop("row_cl",m_client_list.count());
        for(int i=0; i<m_client_list.count(); i++) {
            QString number = QString::number(i);
            QString num = QString::number(i+1);
            QString port = QString::number(m_client_list.at(i).port);
            QDateTime cdt2 = QDateTime::fromMSecsSinceEpoch(m_client_list.at(i).connect_time);
            QString connect_time = cdt2.toLocalTime().toString("yyyy-MM-dd hh:mm:ss.z");
            QString type   = m_client_list.at(i).type;

            t.setVariable("row_cl"+number+".number",num);
            t.setVariable("row_cl"+number+".port",port);
            t.setVariable("row_cl"+number+".connectTime",connect_time);
            t.setVariable("row_cl"+number+".type",type);

        }
    response.write(t.toUtf8(),true);
}
//--------------------------------------------------------------------------
QVector<ModemStruct> SummaryController::getModemList()
{
    QJsonObject obj_modems = getObject("Modems");

    QVector<ModemStruct>  modem_list;
    ModemStruct modem_item;
    QJsonObject modem_item_obj;

    QJsonArray modem_arr = obj_modems["params"].toArray();

    for (int i = 0; i < modem_arr.count(); ++i){
         modem_item_obj = modem_arr.at(i).toObject();

         if (modem_item_obj.contains("DCE_ID")){
            modem_item.DCE_ID = modem_item_obj.value("DCE_ID").toInt();
        }
        if (modem_item_obj.contains("port")){
            modem_item.port = modem_item_obj.value("port").toInt();
        }
        if (modem_item_obj.contains("IP")){
            modem_item.IP = modem_item_obj.value("IP").toString();
        }
        if (modem_item_obj.contains("radioType")){
            modem_item.radio_type = modem_item_obj.value("radioType").toString();
        }
        if (modem_item_obj.contains("creationTime")){
            modem_item.creation_time = modem_item_obj.value("creationTime").toDouble();
        }
        if (modem_item_obj.contains("enableTime")){
            modem_item.enable_time = modem_item_obj.value("enableTime").toDouble();
        }
        if (modem_item_obj.contains("stage")){
            modem_item.stage = modem_item_obj.value("stage").toString();
        }
        if (modem_item_obj.contains("busy")){
            modem_item.busy = modem_item_obj.value("busy").toString();
        }
        if (modem_item_obj.contains("regim")){
            modem_item.regim = modem_item_obj.value("regim").toInt();
        }

        modem_list.append(modem_item);
    }

return modem_list;
}
//--------------------------------------------------------------------------
QVector<ClientStruct> SummaryController::getClientList()
{
    QJsonObject obj_clients = getObject("Clients");

    QVector<ClientStruct> client_list;
    ClientStruct client_item;
    QJsonObject  client_item_obj;

    QJsonArray client_arr = obj_clients["params"].toArray();

    for (int i = 0; i < client_arr.count(); ++i){
         client_item_obj = client_arr.at(i).toObject();

         if (client_item_obj.contains("port")){
            client_item.port = client_item_obj.value("port").toInt();
        }
        if (client_item_obj.contains("connectTime")){
            client_item.connect_time = client_item_obj.value("connectTime").toDouble();
        }
        if (client_item_obj.contains("type")){
            client_item.type = client_item_obj.value("type").toString();
        }

     client_list.append(client_item);
    }
   return client_list;
}

//--------------------------------------------------------------------------
// процедура создания списка подключенных клиентов , доступных УВС, настроек
// создается по данным файла  "test_summary.txt"
void SummaryController::createTestSummaryList()
{   
    QString fl_path = m_testroot +"/"+ m_sammname;

    QFile file (fl_path);
    QString json_str;
    QJsonParseError parseErr;
    QString error = "";

    if (!file.exists()){
      qDebug()<<"File: test_summary.txt - does not exist!";
    }
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) // Проверяем, возможно ли открыть наш файл для чтения
            return; // если это сделать невозможно, то завершаем функцию
       json_str = file.readAll(); //считываем все данные с файла в объект data
    file.close();

    QJsonDocument doc;

    doc = QJsonDocument::fromJson(json_str.toUtf8(), & parseErr);
    if (parseErr.error != QJsonParseError::NoError){
       error = parseErr.errorString();//"JSON-ошибка";
      qDebug()<< "In file: test_summary.txt - JSON error! ("+error+")";
      }
    else if(doc.isObject()){
         QJsonObject root_obj = doc.object();
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
//--------------------------------------------------------------------------------------------------
QJsonObject SummaryController::getObject(const QString objName)
{
   QJsonObject jObj;
   jObj = m_test_list.value(objName).toJsonObject();
   return jObj;
   }
//--------------------------------------------------------------------------------------------------
