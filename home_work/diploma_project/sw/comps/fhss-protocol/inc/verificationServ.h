#ifndef VERIFICATIONSERV_H
#define VERIFICATIONSERV_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QQueue>
#include <QDomDocument>
#include <QtXml>

#include "transportServ.h"
#include "verificationBase.h"



class VerificationServ:  public VerificationBase
{
    Q_OBJECT
public:
    VerificationServ(NetS &net_adrs,                                    // адрес в сети
                     TypeMsgEnum type_msg = JSON,                       // тип сообщения
                     bool format_msg = false                            // форматировананность сообщение
                     );
    ~VerificationServ();     
      QMap<QString, QVariant> getCmdList();

public slots:                     
      void        slotGetClientList(QVector <NetConnectS>);          // обработка сигнала от connector о подключ клиентах
      void        slotGetMsgFromClient(int,  MsgDescrS & msg, QDateTime );   // обработка сигнала от connector о сообщении пришедшем от клиента
      void        slotCommandReReady(MsgDescrS & msg);      // !!!! обработка ответной команды из вне
      void        slotGetNetAdr(NetS &);

private:
    TransportServ               *m_transport_serv;             // модуль для работы с сетью (tcp sockets)
    QMap <int, ClientTypeEnum>   m_clients_type;               // пул подключенных клиентов к данному серверу (по типам (МАС, АУ, УВС))

    bool        commandStructVerify(QJsonObject  & jObj);      // проверка соответствия структуры пакета документу
    void        msgErrAnswProcess(QString msgErr,  MsgDescrS & msg); // обработка ошибки пришедшей команды
    void        msgPreAnswProcess(MsgDescrS & msg_in);     // обработка пришедшей команды в части предварительного ответа
    bool        msgPingAnswer(MsgDescrS & msg);
    QJsonObject prepareErrorMsg(QString err_str, QJsonObject &jObj_in);  // подготовка ответа  клиенту если пришедшая от клиента команда содержала ошибку
    QJsonObject preparePingMsg(QJsonObject &jObj_in);
    QJsonObject preparePreMsg(QJsonObject &jObj_in);
    QJsonObject prepareTermMsg(QJsonObject &jObj_in);
    bool        namedClient(MsgDescrS & msg);             // процедура именования клиента (если пришедшая от клиента ком - TermType)
    void        sendCommandToClient(MsgDescrS & msg);      // отправка сообщ. клиенту(для сервера - как правило, ответное сообщение, но может быть инициативным)



signals:

   void       sigUpdateClientList(QVector <NetConnectS>);     // в widgetDCE c передачей обновленного списка подключенных клиентов
   void       sigChangeFormTitle(QString);                    // в widgetDCE об изменении заголовка формы
   void       sigChangeFormStatusLine(QString);               // в widgetDCE об изменении статус-строки формы 
 //  void       sigCommandGet(MsgDescrS & msg);               // !!!!сигнал о полученной команде от DTE(клиента-АУ или МАС) для внешего приложения
   void       sigNetAdr(NetS &);
};
#endif // VERIFICATIONSERV_H
