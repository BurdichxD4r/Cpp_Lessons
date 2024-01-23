#ifndef PROTOCOLSERV_H
#define PROTOCOLSERV_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include "enum_list.h"
#include "struct_list.h"
#include <QQueue>
#include "QTimer"
#include "verificationServ.h"

class ProtocolServ:public QObject
{
Q_OBJECT
public:
    ProtocolServ(NetS &net_adrs,
                 TypeMsgEnum type_msg = JSON,      // тип сообщения
                 bool format_msg = false           // формат сообщение
            );
    ~ProtocolServ();   

public slots:
    void        slotSetID(int id);
    void        slotSetNetAdr(NetS &adr);                                 // установка адреса в сети

   void         slotErrorFromTurnOnModemRadio(QString msgErr, MsgDescrS & msg);
   void         slotCmdToClientFromChanel(QString cmd_name, QJsonObject &param_list_out); // подготовка сообщения клиенту инициативного (из радио канала)
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   void         slotCmdFromClient(MsgDescrS & msg);                       // сообщение от клиента из модуля Verification...
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void         slotCmdToClient(MsgDescrS & msg_in, QJsonObject &param_list_out);     // подготовка ответного сообщения клиенту
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
   void         slotUseDST(bool);
 //  void         slotSetBusy(bool);
  // void         slotSetStage(int);
   //  void         slotDisconnect();


private:
   VerificationServ    *m_verify_serv;                // верификация на уровни сервера
   QString              m_err;                        //  для  хранения ошибок обработки сообщений json
   ProtocolParamS       m_protocol_obj;               //  Состояние протокола (структура задается выше)
   bool                 m_use_dst{true};              // использование поля DCE_ID_DST в составе ком. ExchangeDCE
   bool                 m_verify{false};              // флаг подтверждение иммитовставки на пришедшее сообщение

            // проверка сообщения от клиента на содержание к.л. поля или значения
   bool        checkVerifyAnswFromClient(QJsonObject & jsonObj);    // проверка принятой от клиента ответной команды со статусом = verify
   bool        checkRecvReplyFromClient(QJsonObject &jsonObj);      // проверка принятой от клиента ответной команды со статусом = recv команды ReplayExchadgeDCE
   bool        checkExchFromClient(QJsonObject &jsonObj);           // проверка принятой от клиента команды ExchangeDCE

   void        msgControlCommandProcess(MsgDescrS & msg);  // обработка сервером управляющей команды от клиента
   void        msgVerifyReplayProcess(MsgDescrS & msg);    // обработка сервером ответной команды verify
   void        msgRecvReplayProcess(MsgDescrS & msg);      // обработка сервером ответной команды recv(ReplyExchangeDCE)
   void        msgControlExchangeCommandProcess(MsgDescrS & msg);// обработка сервером управляющей команды ExchangeDCE от клиента
   void        msgErrorProcess(QString msgErr, MsgDescrS & msg);                  // обработка сервером ошибок в сообщении принятом от клиента
   void        msgEnableParse(QJsonObject  & msg_obj);        // порсер команды для Гука
           // подготовка шаблонов для ответных сообщений
   QJsonObject prepareAnswMsg(QJsonObject &jObj_in, QJsonObject &param_list_out);
   QJsonObject prepareInitiativMsg(QString cmd_name, QJsonObject &param_list_out);

   QJsonObject prepareErrorMsg(QString err_str, QJsonObject &jObj_in); // подготавка ответн сообщ об ошибке

   void        processObject(QJsonObject & obj_in,        // обработка объектов команд, входящ.(принятая послед.) и эталонной из m_com_list
                             QJsonObject & obj_sampl,
                             ModeEnum mode);
   QJsonObject processParamList(QJsonObject &jOjb_out,   // обработка списка параметров для записи в ответную команду для ответа клиенту
                                QJsonObject &param_list_out);

  // int         decodingGettingObject(QJsonObject json_in);    // декодирование принятого от клиента сообщения
 //  int         decodingDataLength(QJsonValue val);           // размер передаваемых данных после декодирования

   void        versionRe(QJsonObject & obj, QJsonObject &param_list_out);      // подготовка ответного сообщения клиенту Version c полем list : [...]
   void        dceInfoRe(QJsonObject & obj, QJsonObject &param_list_out);      // подготовка ответного сообщения клиенту GetDCEInfo c полем dce_list : [...]
   void        dirInfoRe(QJsonObject & obj, QJsonObject &param_list_out);      // подготовка ответного сообщения клиенту GetDirInfo c полем dir_list : [...]
   void        statusRe(QJsonObject & obj, QJsonObject &param_list_out);       // подготовка ответного сообщения клиенту Status c полем statusList : {...}
   void        enableRe(QJsonObject & obj, QJsonObject &param_list_out);       // подготовка ответного сообщения клиенту EnableDCE
   void        disableRe(QJsonObject & obj, QJsonObject &param_list_out);      // подготовка ответного сообщения клиенту DisableDCE
   void        errorRe(QJsonObject & obj, QJsonObject &param_list_out);        // подготовка ответного сообщения клиенту Error
   void        syncRe(QJsonObject & obj, QJsonObject &param_list_out);         // подготовка ответного сообщения клиенту  команды NetworkSYNC
   void        connectRe(QJsonObject & obj, QJsonObject &param_list_out);      // подготовка ответного сообщения клиенту  команды NetworkCONNECT
   void        disconRe(QJsonObject & obj, QJsonObject &param_list_out);       // подготовка ответного сообщения клиенту  команды NetworkDISCON
   void        replyRe(QJsonObject & obj, QJsonObject &param_list_out);       // подготовка ответного сообщения клиенту  команды ReplayExchangeDCE
   void        exchangeRe(QJsonObject & obj, QJsonObject &param_list_out);     // подготовка инициативного сообщения клиенту  команда ExchangeDCE


signals:

   void       sigAddMsgToJournal(int tx, QByteArray & ba, QDateTime tm);
   void       sigNetAdr(NetS &);
   void       sigChangeWorkStatus(QString & str, QTime tm);
   void       sigChangeFormStatusLine(QString);               // в widgetDCE об изменении статус-строки формы
   void       sigUpdateClientList(QVector <NetConnectS>);     // в widgetDCE c передачей обновленного списка подключенных клиентов
   void       sigDisconnect();                                // остановка сервера
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void       sigCmdToClient(MsgDescrS & msg);                  // !!! готова ответная команда для клиента
   void       sigCmdFromClient(MsgDescrS & msg);                // пришла команда от клиента
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 //  void       sigDataToModem(char * data, int len);// сигнал модему с принятыми от клиента данными

};


#endif // PROTOCOLSERV_H
