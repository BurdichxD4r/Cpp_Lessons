// класс VerificationClient  для реализации протокола обмена сообщениями между КЛИЕНТОМ и СЕРВЕРОМ
// работает на стороне клиента и на стороне сервера
// анализирует пришедшие сообщения от оппонента и подготавливает ответ на них согласно протоколу обмена
// основная процедура анализа принятого сообщения : "analuseJSON(XML)packSlot"
// основная процедура подготовки сообщения (команды, ответа, инициативное сообщение): "passCommandSlot"

#ifndef VERIFICATIONCLIENT_H
#define VERIFICATIONCLIENT_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QDomDocument>
#include <QtXml>
#include "transportClient.h"
#include "verificationBase.h"

class VerificationClient: public VerificationBase
{
    Q_OBJECT
public:
    VerificationClient(NetS &net_adrs,
                      TypeMsgEnum type_msg = JSON,      // тип сообщения
                      bool format_msg = false           // формат сообщение
                      );
    ~VerificationClient();

    quint16     getPortNumber();                            // запрос на получение номера порта от Transport ...

public slots:
    void        slotSendCommandDTE(QJsonObject &);          // обработка сигнала из MW подготовка команды для отправки
    void        slotGetMsgFromServ(int k,  MsgDescrS & msg, QDateTime tm);

private:
    bool        commandStructVerify(QJsonObject  & jObj);    // проверка состава команды согластно документу
 //   int         verifyImitsInst(const QJsonObject  & obj);           // проверка иммитовставки на стороне ПАК АУ
//    void        analyseMsgFromServer(const QByteArray & jsonPack);   // пакет пришел от сервера
//    QJsonObject prepareJsonComVerify(QJsonObject &jObj_in);
 //   QJsonObject prepareJsonComReceiveData(QJsonObject &jObj_in);     // подготовка ответа на команду ExchangeDCE от ПАК МАС

 //   bool        checkPreAnsw(QJsonObject & jsonObj);            // проверка принятой команды на то, что она является предварительной
//    bool        checkPositiveAnsw(QJsonObject & jsonObj);       // проверка что полученный ответ соотв. штатной работе системы
//    bool        checkStartAnsw(QJsonObject & jsonObj);          // проверка что полученный ответ соотв. команде со status: start
 //   bool        checkNetBusyAnsw(QJsonObject & jsonObj);        // проверка что полученный ответ соотв. команде со status: net busy
 //   bool        checkUserBusyAnsw(QJsonObject & jsonObj);       // проверка что полученный ответ соотв. команде со status: user busy
  //  bool        checkRecvAnsw(QJsonObject & jsonObj);           // проверка что полученный ответ соотв. команде со status: recv
  //  bool        checkEndAnsw(QJsonObject & jsonObj);            // проверка что полученный ответ соотв. команде со status: end
  //  bool        checkEnqueuedAnsw(QJsonObject & jsonObj);       // проверка что полученный ответ соотв. команде со status: enqueued(ПАК МАС)
  //  bool        checkDataAnsw(QJsonObject & jsonObj);           // проверка что полученный ответ соотв. команде со data.size > 0(ПАК МАС)
    // bool        checkDisconAnsw(QJsonObject & jsonObj);

    TransportClient  *m_transport_cl;                      // коннектор осуществяющий передачу/прием с пом tcp соединения

signals:

 //  void        sigMsgReadyDTE(QByteArray &);       // сигнал о том что  пакет собран, для MW
 //  void        sigPanelOnDTE(QJsonObject &);       // сигнал о включении панели состояний для MW
};

#endif // VERIFICATIONCLIENT_H
