// базовый класс протокола взаимодействия
// объеденяет базовые функции классов VerificationServ, VerificationClient :
#ifndef VERIFICATIONBASE_H
#define VERIFICATIONBASE_H

#include <QObject>
#include <QTime>
#include <QVariant>
#include <QMap>
#include <QString>
#include <QJsonObject>

#include "enum_list.h"
#include "struct_list.h"
#include "logmaker.h"


class VerificationBase :public QObject
{
Q_OBJECT
public:

    VerificationBase(TypeMsgEnum type_msg = JSON, // тип сообщения
                      bool format_msg = false);                  // формат сообщения
    ~VerificationBase();


     QMap<QString, QVariant> getCmdList();             // метод для получения списка команд согласно документу
     QJsonObject             getObjectSampl(const QString cmd,  // метод для получения структуры команды из списка команд по документу
                                            const QString &suffix);   // задает свойство команды: запрос или ответ
     void        processObject(QJsonObject & obj_in,        // обработка объектов команд, входящ.(принятая послед.) и эталонной из m_com_list
                               QJsonObject & obj_sampl,
                               ModeEnum mode);
public slots:
    void        slotGetMsgToJourn(int,  QByteArray &, QDateTime);  // обработка полученного сигнала от Transport... с принятым сокетом сообщением
    void        slotGetOkError(QString &, QTime );      // обработка полученного сигнала от Transport с данными о состоянии соединения (ok, err)
    void        slotDisconnect();                       // обработка сообщения от mainW об отключении

protected:    
    TypeMsgEnum              m_type_msg;                   // тип сообщения (JSON or XML)
    bool                     m_format_msg;                 // формат сообщения (компактный или обычный формат)в случае типа сообщения JSON
    QMap<QString, QVariant>  m_com_list;                   // список команд, считанная с "command_list.txt"
    QString                  m_error;                    // Описание ошибки, произошедшей во время работы протокола
    LogMaker                *m_log_maker;                // логирование сообщений
    int                      m_count{0};                // счетчик сообщений


    void        addToLog(int k,  QByteArray & ba, QDateTime tm, LogUseEnum flag_use);
    void        createCommandList();                       // создание списка сообщений - m_com_list
    bool        isCmdIdExist(QJsonObject  & jObj);         // проверка наличия полаей cmd и id
    QJsonObject createObj(QByteArray & jsonPack);          // создание объекта(json) из последовательности байт
    bool        jsonCheck(QByteArray & jsonPack);          // проверка json пакета на содержание ошибок

    QByteArray  prepareMsg(const QJsonObject & objectCom); // функция подготовки сообщения по данным объекта json
    QByteArray  createJsonMsg(const QJsonObject  & obj,    // сообщение в json формате по данным json объекта
                              bool format_msg);
signals:        
    void        sigAddMsgToJournal(int,  QByteArray &, QDateTime ); // для MW с данными принятыми(отправленными) Transport... для добавления их в журнал
    void        sigChangeWorkStatus(QString &, QTime );         // для MW об ошибке/норм работе соединения из Transport...
    void        sigDisconToConnector();                         // Transport... о прекращении сеанса связи
    void        sigCommandGet(MsgDescrS & msg);               // сигнал о команде принятой, проверенной на данном уровне и готовой для передаче на уровень выше

};

#endif // VERIFICATIONBASE_H
