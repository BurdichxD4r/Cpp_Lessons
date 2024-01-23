/* модуль verificationClient проверяет последовательность байт сообщения, принятую на транспортном уровене в модуле transport...
 * на наличие ошибок при представалении последовательности как jsonObj,  так же модуль проверяет наличие обязательных поле,
 * имеется возможность проверять структуру команды на соответствие документу с перечисленными командами и полями
 * - command_list.txt
 * если перечисленные проверки пройдены, команда в виде jsonObj передается на уровень выше для анализа её содержимого:
 * в модуль protocol...
 */

#include "verificationClient.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QtXml>

//------------------------------------------------------------------
VerificationClient::VerificationClient(NetS & net_adr, TypeMsgEnum type_msg, bool format_msg):
    VerificationBase(type_msg, format_msg)
{  
   m_transport_cl = new TransportClient(net_adr, type_msg);

       // сигнал для transport о разрыве сеанса связи
   connect(this, &VerificationBase::sigDisconToConnector, m_transport_cl, &TransportClient::slotDisconnect);
      // сигнал из transport о приходе очередного сообщения  и передачи сообщения в verification для дальнейшей обработки
   connect(m_transport_cl, &TransportBase::sigMsgToJourn, this, &VerificationBase::slotGetMsgToJourn);
   connect(m_transport_cl, &TransportBase::sigMsg, this, &VerificationClient::slotGetMsgFromServ);

     // сигнал из transport о состоянии текущего сеанса связи  и передачи этой информации в verification для дальнейшей обработки
   connect(m_transport_cl, &TransportBase::sigOkError, this, &VerificationBase::slotGetOkError);
   createCommandList();  // создание списка команд (по данным файла: command_list.txt)                        
}
//-------------------------------------------------------------------
 VerificationClient:: ~VerificationClient()
{       

} 
 //------------------------------------------------------------------
 // запрос номера порта для подключения у transport ...
 quint16 VerificationClient::getPortNumber()
 {
    return m_transport_cl->getPortNumber();
  } 
 //--------------------------------------------------------------------------------------------
 bool VerificationClient::commandStructVerify(QJsonObject  & jObj)
 {
     QJsonObject     json_sampl_in;     // эталонный объект для ответного сообщения, собранный из m_com_list (значения по умолчанию из command_list.txt)
     QJsonObject     json_sampl_out;    // объект для ответного сообщения(с частично заполнеными полями), если имя ключа совпадает с ключом в jObj_in )
     QString         err;
     QString         suffix;        // префикс для входящей команды (для выбора нужной команды из "command_list.txt")

     QString name_com_in = jObj.value("cmd").toString();     // выделим имя пришедшей команды

     if (jObj.contains("status")) // команда ответная
     {
        suffix = "_i"; // префикс для входящей команды (для выбора нужной команды из "command_list.txt")
     }
     else{
        suffix = "_o";
     }

     if(name_com_in!="ExchangeDCE" && name_com_in!="ReplyExchangeDCE"){
         json_sampl_in = m_com_list.value(name_com_in + suffix).toJsonObject();
        // processObject(jObj, json_sampl_in, CHECK);              // для сравнения(проверке принятого сообщения) здесь поле m_error может заполнится к.л. ошибкой
      }
     else if (name_com_in == "Test"){
          // оставляем команду как есть не проверяем и не ищем ошибки
     }
     else if (name_com_in == "ExchangeDCE"){
          suffix = "_o";
          json_sampl_in = m_com_list.value(name_com_in + suffix).toJsonObject();
          if(jObj.contains("DCE_ID_OPPN")){

          }
          else{
              if (jObj.contains("DCE_ID_SRC"))
                 json_sampl_in.remove("DCE_ID_DST");
              else
                 json_sampl_in.remove("DCE_ID_SRC");
              processObject(jObj, json_sampl_in, CHECK);
          }
     }
     else if (name_com_in == "ReplyExchangeDCE"){
         suffix = "_i";
         json_sampl_in = m_com_list.value(name_com_in + suffix).toJsonObject();
        if (jObj.contains("DCE_ID_SRC"))
           json_sampl_in.remove("DCE_ID_DST");
        else
          json_sampl_in.remove("DCE_ID_SRC");
        // processObject(jObj, json_sampl_in, CHECK);
     }

     if (m_error!="")               // ошибка при проверке структуры команды на соответствие протоколу
       return false;                // команда не прошла верификацию
     else
       return true;                 // команда верифицирована
 }
 //--------------------------------------------------------------------------------------------
 void VerificationClient::slotGetMsgFromServ(int k,  MsgDescrS & msg, QDateTime tm)
 {     
     Q_UNUSED(tm);    

     QJsonObject     json_Obj;
     m_error = "";                                // поле ошибок пустое, т.к. сообщение только начинает обрабатываться

      if(k == RX){                                // приняли сообщение от сервера
         if (jsonCheck(msg.ba_msg_in)){                      // проверка сообщения на наличие ошибок
            json_Obj =  createObj(msg.ba_msg_in);
            if(isCmdIdExist(json_Obj)){           // проверка  сообщения на наличие обязательных полей cmd, ?id
              if(commandStructVerify(json_Obj)){  // верификация пройдена
              //==================================================
                 msg.obj_msg_in = json_Obj;
                 emit sigCommandGet(msg);    // команда отправляется дальше на уровень выше для детального изучения, но только если она прошла все проверки
              //==================================================
              }
            }
          }

         if (m_error!=""){ // проверка пришедшего сообщения не пройдена, есть не пустое поле m_error
            QMessageBox::critical(0, "Сообщение модуль verificationClient","Ошибка принятой последовательности: " + m_error,"");
         }
      }
     //analyseMsgFromServer(ba);      
 }
 //--------------------------------------------------------------------------------------------
 //                   Подготовка сообщения для отправки по данным JSON объекта
 //--------------------------------------------------------------------------------------------
  // слот подготовки сообщения из объекта json для отправки серверу
  void VerificationClient::slotSendCommandDTE(QJsonObject & jsonObj)
  {
      m_error = "";    // поле ошибок пустое, т.к. сообщение только начинает обрабатываться

      if(isCmdIdExist(jsonObj)){           // проверка  сообщения на наличие обязательных полей cmd, ?id
        if(commandStructVerify(jsonObj)){  // верификация пройдена
        //==================================================
            QByteArray pack_to_send;
            pack_to_send = prepareMsg(jsonObj);         //  функция из BaseProtocol если сообщение нужно отправить в xml формате -
                                                        // преобразование json->xml происходит на транспортном уровне
            m_transport_cl->sendToServer(pack_to_send);   // теперь уже последовательность байт отправляется серв.
                                                        // если пройдены все проверки
        //==================================================
         }
      }
      if (m_error!=""){ // проверка пришедшего сообщения не пройдена, есть не пустое поле m_error
         QMessageBox::critical(0, "Сообщение модуль verificationClient","Ошибка в последовательности для отправки: " + m_error,"");
      }
  } 
//---------------------------------------------------------------------------------------------------
