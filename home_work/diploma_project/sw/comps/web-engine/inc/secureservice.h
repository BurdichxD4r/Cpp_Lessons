// модуль проверки валидности пары логин:пароль при входе пользователя в систему

#ifndef SECURESERVICE_H
#define SECURESERVICE_H

#include <QObject>
#include <QSettings>
#include <QJsonObject>
#include "filelogger.h"

using namespace stefanfrings;

class SecureService:public QObject
{
    Q_OBJECT
public:
    SecureService(const QSettings* settings, QObject* parent = nullptr);

    bool                checkAuthentication(QByteArray & login,
                                    QByteArray & pass); // проверка пары логин:пароль
    QMap<QString, QVariant> getUserDataFromJsonObj();              // считывание данных [логин:пароль] из JSON объекта

    void                saveUserListToFile(QJsonObject &jObj);
    int                 setUserList(QMap<QString, QVariant> & user_list, QJsonObject &jObj);
private:
    /** Root directory of secure */
    QString m_secureroot;

    /** Encoding of text files */
    QString m_encoding;

     /** Name of file with login and pass */
    QString m_filename;

    QMap<QString, QVariant>  m_user_list;         // список зарегист. пользователей
    QJsonObject              m_json_obj;          // json объект для хранения данных пользователей

    void        createUserList();
    QJsonObject getObject(const QString objName);

    bool        findUserDataInVector(QByteArray & login, QByteArray & pass, QMap<QString, QVariant> & user_list);  // поиск введенных данных в данных, считанных из файла хранения

};

#endif // SECURESERVICE_H
