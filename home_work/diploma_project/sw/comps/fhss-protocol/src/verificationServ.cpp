#include "verificationServ.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QtXml>

//-----------------------------------------------------------------------------------
VerificationServ::VerificationServ(NetS & net_adrs, TypeMsgEnum type_msg, bool format_msg):
    VerificationBase(type_msg, format_msg)
{  
   m_transport_serv = new TransportServ(net_adrs, type_msg);
         // разрыв соединеия
   connect(this, &VerificationBase::sigDisconToConnector, m_transport_serv, &TransportServ::slotStopServer);
        // приём сообщения
   connect(m_transport_serv, &TransportBase::sigMsgToJourn, this, &VerificationBase::slotGetMsgToJourn);
   connect(m_transport_serv, &TransportBase::sigMsg, this, &VerificationServ::slotGetMsgFromClient);
        // состояние соединени
   connect(m_transport_serv, &TransportBase::sigOkError, this, &VerificationBase::slotGetOkError);
        // список подключенных клиентов
   connect(m_transport_serv, &TransportServ::sigClientListDCE, this, &VerificationServ::slotGetClientList);

   connect(m_transport_serv, &TransportServ::sigNetAdr, this, &VerificationServ::sigNetAdr);
   createCommandList();  // создание списка сообщений (по данным файла: command_list.txt)                         
 }
//------------------------------------------------------------------------------------
 VerificationServ:: ~VerificationServ()
 {
     delete m_transport_serv;
 }
 //-----------------------------------------------------------------------------------
 QMap<QString, QVariant> VerificationServ::getCmdList()
 {
     return m_com_list;
 }
 //-------------------------------------------------------------------------------------
 // процедура отправки сообщения Клиенту (через модуль ConectorDCE)
 void VerificationServ::sendCommandToClient(MsgDescrS & msg)
 {
      QByteArray pack_to_send;
      pack_to_send = prepareMsg(msg.obj_msg_out);    // функция из BaseProtocol если сообщение нужно отправить в xml формате -
                                             // преобразование json->xml происходит внутри prepareMsg(jsonObj)
     msg.ba_msg_out = pack_to_send;

     QVector <NetConnectS> feat_connect_vec;        // массив характеристик присоединенных к серверу клиентов
     feat_connect_vec = m_transport_serv->formClientListDCE();

     NetConnectS  feat_client;
     QString client_type{""};
     QString name_com = msg.obj_msg_out.value("cmd").toString(); // выделим из пришедшей команды имя;

     //!! необходима проверка descr на 0 (если сообщение пришло из сервера обмена),
     //!! тогда проверяем по наименованию комманд и типу подключенных клиентов
     //!!
     if(msg.descr == 0) {// сообщение из сервера обмена, его нужно разослать всем клиентам  по подходящему типу(MAS or AU)
        for(int i = 0; i < feat_connect_vec.count(); ++i){  // просматриваем весь пул клиентов
           feat_client = feat_connect_vec.at(i);         // выбираем очередного
               // если тип клиента
           if ((feat_client.client_type == "MAS") && (name_com == "ExchangeDCE" || name_com == "ReplyExchangeDCE" )){
               m_transport_serv->sendToClient(feat_client.discr, msg.ba_msg_out); // отправляем сообщение пришедшее из серв. обмена
           }
           else if ((feat_client.client_type == "AU") &&
                    (name_com == "NetworkSYNC" ||
                     name_com == "NetworkCONNECT" ||
                     name_com == "NetworkDISCON" )){
               m_transport_serv->sendToClient(feat_client.discr, msg.ba_msg_out); // отправляем сообщение пришедшее из серв. обмена
           }
        }
     }
     else{   // если дискриптор не 0
       m_transport_serv->sendToClient(msg.descr, msg.ba_msg_out);
     }
 }
 //------------------------------------------------------------------------------------
 void  VerificationServ::slotGetNetAdr(NetS& net_adr)
 {
     emit sigNetAdr(net_adr);
 }
 //------------------------------------------------------------------------------------
 // обработчик поступления от Connector списка подключенных клиентов
 void VerificationServ::slotGetClientList(QVector <NetConnectS> client_vec)
 {
     emit sigUpdateClientList(client_vec);  //  сигнал в Widget c данными о прдключ. клиентах
 } 
 //------------------------------------------------------------------------------------
 //====================================================================================
 // обработка сигнала из вне о готовности ответной команды для отправки клинету
 // для отправке её клиенту
 void VerificationServ::slotCommandReReady(MsgDescrS & msg)
 {    
    sendCommandToClient(msg);
 } 
 //====================================================================================
 //------------------------------------------------------------------------------------
 void VerificationServ::slotGetMsgFromClient(int k,  MsgDescrS & msg, QDateTime tm)
 {
    Q_UNUSED(tm);

    QJsonObject     json_Obj;
    m_error = "";                                // поле ошибок пустое, т.к. сообщение только начинает обрабатываться

     if(k == RX){                                // приняли сообщение от клиента
        if (jsonCheck(msg.ba_msg_in)){                      // проверка сообщения на наличие ошибок
           json_Obj =  createObj(msg.ba_msg_in);
           if(isCmdIdExist(json_Obj)){           // проверка  сообщения на наличие обязательных полей cmd, id
             if(commandStructVerify(json_Obj)){  // верификация пройдена
             //==================================================
                msg.obj_msg_in = json_Obj;
              //  msgPreAnswProcess(msg);     // обработка принятой команды с выдачей предв. ответа клиенту и занесения в журнал WindgetDCE
                if(!msgPingAnswer(msg) && !namedClient(msg))// ответ на команду Ping,  выше команда не передается// именование клиента(проверка если пришла команда TermType)
                    emit sigCommandGet(msg);    // команда отправляется дальше только если она прошла все проверки и это не Ping и не typeTerm

//==================================================
              }
           }
         }
        if (m_error!=""){ // проверка пришедшего сообщения не пройдена, есть не пустое поле m_error
           // QMessageBox::critical(0, "Сообщение модуль fhss-protocolDCE","Ошибка принятой последовательности: "+m_error,"");
            msgErrAnswProcess(m_error,msg);      //обработка выявленных ошибок при разборе принятой команды
        }
     }
 }
 //-------------------------------------------------------------------------------------
 bool VerificationServ::namedClient(MsgDescrS & msg)
 {
     bool res = false;
     // действия по именованию клиента
     QString name_com = msg.obj_msg_in.value("cmd").toString(); // выделим из пришедшей команды имя;
     if (name_com =="TermType"){   // если пришедшая команда cmd = "TermType"
         // выделим тип подключенного клиета
         QString term_name = msg.obj_msg_in.value("name").toString();
         m_transport_serv->namedClient(msg.descr, term_name);   // именуем пока еще безымянного клиента, типом содержащимся в ком. TermType
         msg.obj_msg_out = prepareTermMsg(msg.obj_msg_in);         // подготовка ПРЕДВАРИТЕЛЬНОГО ответа
         res = true;
         sendCommandToClient(msg);
     }
    return res;
 }
 //-------------------------------------------------------------------------------------
 void VerificationServ::msgPreAnswProcess(MsgDescrS & msg)
 {
    QString name_com = msg.obj_msg_in.value("cmd").toString(); // выделим из пришедшей команды имя;
    if(name_com !="ExchangeDCE" && name_com !="ReplyExchangeDCE" && !msg.obj_msg_in.contains("status") ){  // предварительный ответ для ком. инф. обмена не формируем
       msg.obj_msg_out = preparePreMsg(msg.obj_msg_in);         // подготовка ПРЕДВАРИТЕЛЬНОГО ответа
       sendCommandToClient(msg);
    }
 }
 //-------------------------------------------------------------------------------------
 bool VerificationServ::msgPingAnswer(MsgDescrS & msg)
 {
    bool res = false;
    QString name_com = msg.obj_msg_in.value("cmd").toString(); // выделим из пришедшей команды имя;
    if(name_com =="Ping"){  //  если ком. Ping, возвращаем ответ  в том же виде
       msg.obj_msg_out = preparePingMsg(msg.obj_msg_in);
       res = true;
       sendCommandToClient(msg);
    }
    return res;
 }
 //-------------------------------------------------------------------------------------
 void VerificationServ::msgErrAnswProcess(QString msgErr, MsgDescrS & msg)
 {
     msg.obj_msg_out = prepareErrorMsg(msgErr, msg.obj_msg_in);
     sendCommandToClient(msg);   // сигнал в ProtocolDCE - необх. отправить отетную команду ERROR
 }
 //----------------------------------------------------------------------------------------------
  // подготовка ответа на команду Ping , формируется если поля cmd, id считаны и правильно идентифицированы
  // после нее может быть либо положительная команда - ответ, либо сообщение об ощибке
  QJsonObject VerificationServ::preparePingMsg(QJsonObject &jObj_in)
  {
      QJsonObject jObj_out;
      QString suffix = "_i"; // суффикс для команды (для выбора нужной команды из "command_list.txt")
      QString name_com = "Ping";     // выделим из шаблона имя
      QString uuid_com = jObj_in.value("id").toString();// выделим из шаблона id
      // получим шаблон ответного сообщения из m_com_list (по данным jObj_in.cmd);
      jObj_out = m_com_list.value(name_com + suffix).toJsonObject();
      jObj_out.insert("id", uuid_com); // заполним поле id (по данным jObj_in.id)

      return jObj_out;     // возвращаем частично заполненный шаблон ответной команды
  }
 //----------------------------------------------------------------------------------------------
  // подготовка предварительного ответного сообщения , формируется если поля cmd, id считаны и правильно идентифицированы
 // после нее может быть либо положительная команда - ответ, либо сообщение об ощибке
  QJsonObject VerificationServ::preparePreMsg(QJsonObject &jObj_in)
  {
      QJsonObject jObj_out;
      QJsonObject jObj_item;
      // этой команды нет в сommand_list.txt -она служебная (не основная)
      QString name_com = jObj_in.value("cmd").toString(); // выделим из пришедшей команды имя;
      QString uuid_com = jObj_in.value("id").toString();  // выделим из пришедшей команды id

      // получим шаблон ответного сообщения из m_com_list (по данным jObj_in.cmd);
      if(name_com == "ExchangeDCE"){
          name_com = "ReplyExchangeDCE";
      }
       jObj_out.insert("cmd", name_com);
       jObj_out.insert("id", uuid_com); // заполним поле id (по данным jObj_in.id)
           jObj_item.insert("StringAnswer","Ok");
       jObj_out.insert("param", jObj_item);     // заполним поле param

      return jObj_out;//  заполненный шаблон ответной команды
  }
  //----------------------------------------------------------------------------------------------
   // подготовка ответного сообщения на ком. TermType , формируется если поля cmd, id считаны и правильно идентифицированы
  // после нее может быть либо положительная команда - ответ, либо сообщение об ощибке
   QJsonObject VerificationServ::prepareTermMsg(QJsonObject &jObj_in)
   {
       QJsonObject jObj_out;
       QString suffix = "_i";     // суффикс для команды (для выбора нужной команды из "command_list.txt")

       QString name_com = "TermType";     // выделим из шаблона имя
       QString uuid_com = jObj_in.value("id").toString();// выделим из шаблона id
       // получим шаблон ответного сообщения из m_com_list (по данным jObj_in.cmd);
       jObj_out = m_com_list.value(name_com + suffix).toJsonObject();
       jObj_out.insert("id", uuid_com); // заполним поле id (по данным jObj_in.id)
       jObj_out.insert("name", "UVS");

       return jObj_out;     // возвращаем частично заполненный шаблон ответной команды
   }
 //-------------------------------------------------------------------------------------
 // подготавливаем ответное сообщение об ошибке (если при разборе принятой последовательности выявлена ошибка)
 // может быть отправлена после предварительной ответной команды
 QJsonObject VerificationServ::prepareErrorMsg(QString err_str, QJsonObject &jObj_in)
 {
     QJsonObject jObj_out;
     QString     suffix= "_i";       // суфф для команды (для выбора нужной команды из "command_list.txt")

     QString name_com = "Error";     // выделим из шаблона имя
     QString uuid_com = jObj_in.value("id").toString();// выделим из шаблона id
     // получим шаблон ответного сообщения из m_com_list (по данным jObj_in.cmd);
     jObj_out = m_com_list.value(name_com + suffix).toJsonObject();
     jObj_out.insert("id", uuid_com); // заполним поле id (по данным jObj_in.id)
     jObj_out.insert("errorMsg", err_str);

     return jObj_out;     // возвращаем частично заполненный шаблон ответной команды
 }
//-------------------------------------------------------------------------
bool VerificationServ::commandStructVerify(QJsonObject  & jObj)
{
    QJsonObject     json_sampl_in;     // эталонный объект для ответного сообщения, собранный из m_com_list (значения по умолчанию из command_list.txt)
    QJsonObject     json_sampl_out;    // объект для ответного сообщения(с частично заполнеными полями), если имя ключа совпадает с ключом в jObj_in )
    QString         err;
    QString         suffix;            // суффикс команды (для выбора нужной команды из "command_list.txt")

    QString name_com_in = jObj.value("cmd").toString();     // выделим имя пришедшей команды

    if (jObj.contains("status")) // если команда ответная, содержащая поле status
    {
       suffix = "_i"; // суффикс для входящей команды (для выбора нужной команды из "command_list.txt")
    }
    else{
        suffix = "_o";
    }
     // проверка на содержание команды в списке доп.
    if(!(m_com_list.contains(name_com_in + "_i")) && !(m_com_list.contains(name_com_in + "_o"))){
        m_error="Команды нет в списке допустимых команд.";
        return false;
    }
    if(name_com_in!="ExchangeDCE" && name_com_in!="ReplyExchangeDCE"){
        json_sampl_in = m_com_list.value(name_com_in + suffix).toJsonObject();
       // processObject(jObj, json_sampl_in, CHECK);              // для сравнения(проверке принятого сообщения) здесь поле m_error может заполнится к.л. ошибкой
     }
    else if (name_com_in=="ExchangeDCE"){
         suffix = "_o";
         json_sampl_in = m_com_list.value(name_com_in + suffix).toJsonObject();
        if (jObj.contains("DCE_ID_SRC"))
           json_sampl_in.remove("DCE_ID_DST");
        else
            json_sampl_in.remove("DCE_ID_SRC");
        processObject(jObj, json_sampl_in, CHECK);
    }
    else if (name_com_in=="ReplyExchangeDCE"){
        suffix = "_i";
        json_sampl_in = m_com_list.value(name_com_in + suffix).toJsonObject();
        if(jObj.contains("DCE_ID_OPPN")){

         }
        else{
          if (jObj.contains("DCE_ID_SRC"))
            json_sampl_in.remove("DCE_ID_DST");
          else
            json_sampl_in.remove("DCE_ID_SRC");
        }
       // processObject(jObj, json_sampl_in, CHECK);
    }

    if (m_error!="")               // ошибка при проверке структуры команды на соответствие протоколу
      return false;                // команда не прошла верификацию
    else
      return true;                 // команда верифицирована
}
//-------------------------------------------------------------------------


