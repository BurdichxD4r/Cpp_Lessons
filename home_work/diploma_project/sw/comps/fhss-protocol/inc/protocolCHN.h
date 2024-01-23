// модуль ProtocolCHN - для работы с Сервером обмена для осущенствления обмена сообщеними меджу двумя ПАК УВС
// класс является дочерний к BaseProtocol
// модуль работает как на стороне клиента так и на стороне Сервера. Сервером является -Сервер обмена. Клиентом часть находящаяся в
// приложении ПАК УВС.

#ifndef PROTOCOLCHN_H
#define PROTOCOLCHN_H

#include <QObject>
#include "enum_list.h"
#include "transportServ.h"
#include "transportClient.h"
#include "verificationBase.h"

class ProtocolCHN :public VerificationBase
{
    Q_OBJECT
public:
    ProtocolCHN(NetS &net_adrs,
                TypeMsgEnum type_msg = JSON,
                bool format_msg = false,
                int type = 1                       // 1 - клиент, 2- сервер
               // ConnectTypeEnum type_conn = TCP     // тип соединения
               );
    ~ProtocolCHN();
public slots:
             // Клиентская часть (УВС)
    void slotDisconClient();           // обработчик сигнала из MW УВС об отключении его как клиента
   // void slotDisconnect();             // обработчик сигнала из connectDTE
    void slotSendMsgClient(const QJsonObject & jsonObj);  
    void slotAnalyseMsgClient(int,   MsgDescrS & msg, QTime);

             // Серверная часть (Сервер обмена)
    void slotGetClientList(QVector <NetConnectS> client_vec); // обработчик сигнала из ConnectorDCE о списке клиентов у сервера обмена
    void slotStopServer(); 
    void slotAnalyseMsgServer(int,  MsgDescrS &, QTime);
    void slotParamErrorSet(QString); // установка параметра порченных байт

private:
    NetS                     m_net_adr;        // адрес сервера в сети
    TransportServ            *m_connectDCE;     // модуль для работы с сетью на стороне сервера
    TransportClient          *m_connectDTE;     // модуль для работы с сетью   на стороне клиента
    int                      m_param_err{10};  // параметр для кол-ва испорченных байт
    int                      m_param_count{0}; // счетчик принятых байт


    bool     checkExchDCEFromClient(QJsonObject & jsonObj);
    void     damagePerSeconBitInByte(QJsonObject & jsonObj);
    void     analyseMsgFromServ(const QByteArray & jsonPack);    // анализ сообщения от Сервера Обмена (дальнейшая пересылка оппонентам)
    void     analyseMsgFromClient(const QByteArray & jsonPack);
    void     sendMsgOppn(const QJsonObject & jsonObj);   // отправка сообщения оппоненам
    void     changeDCEid(QJsonObject & jsonObj);   // обмена номеров обменивающихся клиентов
    bool     checkTermTypeFromClient(QJsonObject &jsonObj);   //проверка команды TermType пришедшей от клиента
signals:
    // signal to connector to stop server

     void    sigClientListToMWind(QVector <NetConnectS> client_vec);                // сигнал передачи списка подключенных клиентов в MW
     void    sigStopServOrDiscon();
     void    sigReturnMsgObjToMW(QJsonObject &);             // сигнал о возращении сообщения от Сервера Обмена в УВС главную форму

};


#endif // PROTOCOLCHN_H
