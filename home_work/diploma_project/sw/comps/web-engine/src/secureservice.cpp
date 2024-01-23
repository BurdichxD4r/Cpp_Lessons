// модуль проверки валидности пары логин:пароль при входе пользователя в систему
#include "secureservice.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>

//--------------------------------------------------------------------------------------------------
SecureService::SecureService(const QSettings* settings, QObject* parent)
{    
    m_encoding=settings->value("encoding","UTF-8").toString();
    m_secureroot=settings->value("path",".").toString();
    m_filename=settings->value("fileName",".").toString();

    if(!(m_secureroot.startsWith(":/") || m_secureroot.startsWith("qrc://")))
    {
        // Convert relative path to absolute, based on the directory of the config file.
        #ifdef Q_OS_WIN32
            if (QDir::isRelativePath(m_secureroot) && settings->format()!=QSettings::NativeFormat)
        #else
            if (QDir::isRelativePath(m_secureroot))
        #endif
        {
            QFileInfo configFile(settings->fileName());
            m_secureroot=QFileInfo(configFile.absolutePath(), m_secureroot).absoluteFilePath();
        }
    }
    qDebug("SecureService: secureroot=%s, encoding=%s filename=%s",qPrintable(m_secureroot),qPrintable(m_encoding),qPrintable(m_filename));
}
//--------------------------------------------------------------------------------------------------
// проверка данных пользователя
bool SecureService::checkAuthentication(QByteArray & login, QByteArray & pass)
{
    bool res{false};
    createUserList();
    QMap<QString, QVariant> user_data;
    user_data = getUserDataFromJsonObj();
    res = findUserDataInVector(login, pass, user_data);
    return res;
}
//--------------------------------------------------------------------------------------------------
QJsonObject SecureService::getObject(const QString objName)
{
   QJsonObject jObj;
   jObj = m_user_list.value(objName).toJsonObject();
   return jObj;
}
//--------------------------------------------------------------------------------------------------
// загрузка данных пользователя из JSON obj
QMap<QString, QVariant> SecureService::getUserDataFromJsonObj()
{
    QJsonObject  obj_users = getObject("Users");
    QVariantMap  user_list;
    QString login;
    QString pass;
    QJsonObject  user_item_obj;

    QJsonArray user_arr = obj_users["params"].toArray();

    for (int i = 0; i < user_arr.count(); ++i){
         user_item_obj = user_arr.at(i).toObject();

        if (user_item_obj.contains("login")){
            login = user_item_obj.value("login").toString();
        }
        if (user_item_obj.contains("pass")){
           pass = user_item_obj.value("pass").toString();
        }

        user_list.insert(login, pass);
    }

    return user_list;
}
//--------------------------------------------------------------------------------------------------
// проверка данных пользователя находится ли его данные в файле или нет.
bool SecureService::findUserDataInVector(QByteArray & username, QByteArray & pass, QMap<QString, QVariant> & user_list)
{
   bool res{false};

   if((username.count()>0)||(pass.count()>0) ){

      foreach(QString login, user_list.keys()){
        if((login==username)&&(user_list.value(login).toString()==pass)){
             res = true;
         }
      }
   }
    return res;
}
//---------------------------------------------------------------------------------------------------
// процедура создания списка зарегист. пользователей системы
// создается по данным файла  "web_app_server.sec"
void SecureService::createUserList()
{
    QFile secureFile (m_secureroot+"/"+m_filename);
    QString json_str;
    QJsonParseError parseErr;
    QString error = "";

    if (!secureFile.exists()){
      qDebug()<<"File: web_app_server.txt - does not exist!";
    }
    if (!secureFile.open(QIODevice::ReadOnly|QIODevice::Text)) // Проверяем, возможно ли открыть наш файл для чтения
            return;                                      // если это сделать невозможно, то завершаем функцию
       json_str = secureFile.readAll();                         //считываем все данные с файла в объект data
    secureFile.close();

    QJsonDocument doc;

    doc = QJsonDocument::fromJson(json_str.toUtf8(), & parseErr);
    if (parseErr.error != QJsonParseError::NoError){
       error = parseErr.errorString();//"JSON-ошибка";
      qDebug()<< "In file: web_app_server.sec - JSON error! ("+error+")";
      }
    else if(doc.isObject()){
         //QJsonObject root_obj = doc.object();
        m_json_obj = doc.object();
    // делаем обход всего документа json,считывая имя команды и получая соответствующий объект команды
    // записываем все это в QVariantMap<QString, QVariant(QJsonObject)> m_user_list
         QString  keyName;
         QVariant valueKey;
         for(int i = 0; i < m_json_obj.count(); ++i){
            keyName = m_json_obj.keys().at(i);
            valueKey = m_json_obj.value(keyName).toObject();
            m_user_list.insert(keyName,valueKey);
        }
    }
}
//-----------------------------------------------------------------------------------
// установка измененных параметров установок в jsonObj
int  SecureService::setUserList(QMap<QString, QVariant> & user_list, QJsonObject &jObj)
{
   QJsonObject obj_users;

   QJsonArray user_arr;

   foreach(QString login, user_list.keys()){
       QJsonObject userItem;
       userItem.insert("login",login);
       userItem.insert("pass",user_list.value(login).toString());
       user_arr.append(userItem);
   }

   obj_users.insert("obj", "UserList");
   obj_users.insert("params", user_arr);
  // obj_users.insert("crc", crc_int64);

   jObj.insert("Users",obj_users);
   return 1;
}
//-----------------------------------------------------------------------------------
// сохранение json в файл "web_app_server.sec"
void SecureService::saveUserListToFile(QJsonObject &jObj)
{
   QString fl_path = m_secureroot +"/"+ m_filename;
   QJsonDocument jDoc(jObj);
   QFile file(fl_path);
   if(file.open(QIODevice::WriteOnly)){
      file.write(QString(jDoc.toJson()).toStdString().c_str());
      file.close();
    }
}
//---------------------------------------------------------------------------------------
