#include "protocolServ.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QtXml>
#include <QTimer>

//-------------------------------------------------------------------
ProtocolServ::ProtocolServ(NetS & net_adr,TypeMsgEnum type_msg, bool format_msg)
{
    m_verify_serv = new VerificationServ(net_adr, type_msg, format_msg);

    //=================================================================================================
    //             изменение в режиме работы компоненты приема-передачи
    connect(this, &ProtocolServ::sigDisconnect, m_verify_serv, &VerificationBase::sigDisconToConnector);// команда разорвать соед. (из формы)
    connect(m_verify_serv, &VerificationBase::sigChangeWorkStatus, this, &ProtocolServ::sigChangeWorkStatus);// сервер возможно закрылся (из сокета)
    connect(m_verify_serv, &VerificationServ::sigNetAdr, this, &ProtocolServ::sigNetAdr);   // (из сокета) сообщение об адресе прослушки

    //=================================================================================================
    //               обновление отображения информ. на Widget_DCE
    connect(m_verify_serv, &VerificationBase::sigAddMsgToJournal, this, &ProtocolServ::sigAddMsgToJournal);
    connect(m_verify_serv, &VerificationServ::sigChangeFormStatusLine, this, &ProtocolServ::sigChangeFormStatusLine);
    connect(m_verify_serv, &VerificationServ::sigUpdateClientList, this, &ProtocolServ::sigUpdateClientList);
    //=================================================================================================
    //              работа с Клиентом
    connect(m_verify_serv, &VerificationBase::sigCommandGet, this, &ProtocolServ::slotCmdFromClient);  // получена ком. от клиента!
    connect(this, &ProtocolServ::sigCmdToClient, m_verify_serv, &VerificationServ::slotCommandReReady); // отв.команда клиенту готова!
    //=================================================================================================


 //connect(m_verify_serv, &VerificationServ::sigChangeFormTitle, this, &ProtocolServ::slotSetTitleForm);

  //! m_uvs_obj.create_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
                          //QDateTime::currentDateTime().toLocalTime().toString("yyyy-MM-dd hh:mm:ss");

}
//-------------------------------------------------------------------
 ProtocolServ:: ~ProtocolServ()
 {
 }
 //-------------------------------------------------------------------
 void ProtocolServ::slotSetID(int id)
 {
     Q_UNUSED(id);
   //  m_uvs_obj.DCE_ID = id;
 }
 //-------------------------------------------------------------------
 void ProtocolServ::slotUseDST(bool use)
 {
     m_use_dst = use;
 }
 //-------------------------------------------------------------------
 void    ProtocolServ::slotSetNetAdr(NetS &adr)  // установка адреса в сети
 {
    m_protocol_obj.net_adr.address = adr.address;
    m_protocol_obj.net_adr.port = adr.port;
 }
//----------------------------------------------------------------------
 // сигнал из Widget_DCE о произошедшей ошибке при настройке модема или Радио
 void ProtocolServ::slotErrorFromTurnOnModemRadio(QString msgErr, MsgDescrS & msg)
 {
     msgErrorProcess(msgErr, msg);      //обработка ошибок при запуске модема или радио средств
 }

//----------------------------------------------------------------------
// обработка сообщения от клиента
 void ProtocolServ::slotCmdFromClient(MsgDescrS & msg)
 {
     m_err = "";              // поле ошибки пусто т.к. сообщение только начинает обрабатываться

                              // определим тип принятого сообщения. От клинета серверу может прийти 3 типа сообщений :
                                    // 1. управл.команда;
                                    // 2. отв.сообщ.со status = verify(подтверждение имитовставки);
                                    // 3. отв. сообщ со status = recv(ReplayExchangeDCE)
                                    // 4. управл. команда ExchangeDCE

    if(checkVerifyAnswFromClient(msg.obj_msg_in)){            // относится к типу 2. ?
         msgVerifyReplayProcess(msg);              // обработка ответной команды со status = virify (подтверждение имитовставки)
    }
    else if(checkRecvReplyFromClient(msg.obj_msg_in)){        // относится к типу 3.?
         msgRecvReplayProcess(msg);                // обработка ответной команды со status = recv (ReplayExchangeDCE)
    }
    else if (checkExchFromClient(msg.obj_msg_in)){            // относится к типу 4.?
         msgControlExchangeCommandProcess(msg);    // обработка управляющей команды ExchangeDCE от клиента
    }
    else{                                             // значит это сообщение типа 1. - упр. команда
         msgControlCommandProcess(msg);               // обработка управляющей команды от клиента
    }
                      // m_error может быть  заполнено внутри функции обработки   msgRecvReplayProcess(),
                      // msgControlCommandProcess(), msgVerifyReplayProcess() и все эти ошибки попадут в рез. команду ERROR

    if(m_err !=""){   // если ошибки при разборе пришедшего пакета были
        msgErrorProcess(m_err, msg);      //обработка выявленных ошибок при разборе принятой команды       
    }
 }
 //-------------------------------------------------------------------------------------
 //команда от клиента ExchangeDCE
 void ProtocolServ::msgControlExchangeCommandProcess(MsgDescrS & msg)
 {
     //int cmd_type = 2;   // команда для модема и серв. обмена
     emit sigCmdFromClient(msg);//, cmd_type);
 }
 //-------------------------------------------------------------------------------------
 //команда от клиента со status:verify
  void ProtocolServ::msgVerifyReplayProcess(MsgDescrS & msg)
 {
    //int cmd_type = 1;   // команда только для модема
    emit sigCmdFromClient(msg);// cmd_type);
    m_verify = true;

 }
  //-------------------------------------------------------------------------------------
  // команда ReplayDCE от клиента как ответ на инициативную команду ExchangeDCE
  void ProtocolServ::msgRecvReplayProcess(MsgDescrS & msg)
  {
      Q_UNUSED(msg);
   //   int cmd_type = 0;   // команда не для модема и  не для серв. обмена
   //   emit sigCmdFromClient(msg, cmd_type);
     // ответа клиенту на эту команду не требуется
     // отправляем во внешнее приложение полученную команду и ...
     // ничего не делаем
      return;
  }
  //-------------------------------------------------------------------------------------------
  // пaрсер команды при получении ее от ПАК АУ
 /* void ProtocolServ::msgEnableParse(QJsonObject  & msg_obj)
  {
      QString name_com_in = msg_obj.value("cmd").toString();    // выделим имя пришедшей команды
      QString radio_role{""};
      QString radio_ipv4{""};
      int     radio_udp1{0};
      int     dce_id{0};
      QString id{""};
      QJsonArray radio_array;
      QJsonObject radio_obj;

      if (name_com_in == "EnableDCE"){   // обработка ком. EnableDCE
        if(msg_obj.contains("radioParams")){
            radio_array = msg_obj["radioParams"].toArray();   // массив объектов

            for(int i =0; i < radio_array.count(); ++i){    // обработка всех элементов массива
                radio_obj = radio_array.at(i).toObject();
                if (radio_obj.contains("radioName")){
                    radio_role = radio_obj.value("radioName").toString();
                 }
                if (radio_obj.contains("radioIPv4")){
                    radio_ipv4 = radio_obj.value("radioIPv4").toString();
                 }
                if (radio_obj.contains("radioUdpPort1")){
                    radio_udp1 = radio_obj.value("radioUdpPort1").toInt();
                 }
            }
        }
        if (msg_obj.contains("id")){
                 id = msg_obj.value("id").toString();
        }
        if (msg_obj.contains("DCE_ID")){
                 dce_id = msg_obj.value("DCE_ID").toInt();
        }
     }
  }
  */
//---------------------------------------------------------------------------------------------
// обработака управляющей команды от клиента
  void ProtocolServ::msgControlCommandProcess(MsgDescrS & msg)
  {
     QString name_com_in = msg.obj_msg_in.value("cmd").toString();    // выделим имя пришедшей команды

      if(name_com_in !="TermType" || name_com_in !="Ping" ){          // для этих команд ответ отправляется из verificationLevel

          emit   sigCmdFromClient(msg);  // сигнал в Widget_DCE

      }
  }
  //-------------------------------------------------------------------------------------
  void ProtocolServ::msgErrorProcess(QString msgErr, MsgDescrS & msg)
  {
      msg.obj_msg_out = prepareErrorMsg(msgErr, msg.obj_msg_in);
      emit sigCmdToClient(msg);  // ответ на обработанный запрос клиента в котором была ошибка
  }

//---------------------------------------------------------------------------------------------
//              Проверка к какому типу сообщения отностся пришедшая команда от Клиента
//---------------------------------------------------------------------------------------------
  // проверка пришедшей ответной команды от клиента со статусом "verify", команды NetworkSYNC, NetworkCONNECT
  bool ProtocolServ::checkVerifyAnswFromClient(QJsonObject &jsonObj)
  {
      bool res = false;

      if(jsonObj.contains("status") && jsonObj.value("status").isString()){
          QJsonValue objVal = jsonObj.value("status");
          if (objVal == "verify" )
          {
              res = true;
          }
      }
      return res;
  }
//-------------------------------------------------------------------------------------------
// проверка пришедшей ответной команды от клиента со статусом recv, команды ReplayExchangeDCE
bool ProtocolServ::checkRecvReplyFromClient(QJsonObject &jsonObj)
{
    bool res = false;
    QString name_com = jsonObj.value("cmd").toString();

    if(name_com == "ReplyExchangeDCE") {
       if(jsonObj.contains("status") && jsonObj.value("status").isString()){
           QJsonValue objVal = jsonObj.value("status");
           if (objVal == "recv" ){
            res = true;
           }
       }
    }
    return res;
}
//--------------------------------------------------------------------------------------------
bool ProtocolServ::checkExchFromClient(QJsonObject &jsonObj)
{
    bool res = false;

    QString name_com = jsonObj.value("cmd").toString();

    if(name_com == "ExchangeDCE"){
       if(!jsonObj.contains("status")){  // клча status в команде нет
            res = true;
       }
    }
    return res;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//               Подготовка ответных сообщений клиенту
//                  слоты связаны с сигн. из CoreDCE
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// Формирование ответного сообщения клиенту сигнал из модема
// !!!! инициативные сообщения модема (из радио канала обрабатываются в другом слоте)
void  ProtocolServ::slotCmdToClient(MsgDescrS & msg_in, QJsonObject &param_list_out)
{
   //(param_list_out);   // доп. параметры из CoreDCE для формирования ответного сообщения клиенту

    QJsonObject     json_sampl_out;
    json_sampl_out = prepareAnswMsg(msg_in.obj_msg_in, param_list_out);
    msg_in.obj_msg_out = json_sampl_out;
    emit sigCmdToClient(msg_in);  // сигнал в Verify об ответном сообщении клиенту
}
//-----------------------------------------------------------------------------------------------
// подготавливаем ответное сообщение
QJsonObject ProtocolServ::prepareAnswMsg(QJsonObject &jObj_in, QJsonObject &param_list_out)
{
    QJsonObject jObj_out;
    QString suffix{"_i"}; //?

    QString name_com = jObj_in.value("cmd").toString();     // выделим из шаблона имя пришедшей команды
    QString uuid_com; // = jObj_in.value("id").toString();  // выделим из шаблона id

    // получим шаблон для ответного сообщения
    if(name_com != "ExchangeDCE" && name_com != "ReplyExchangeDCE"){
       uuid_com = jObj_in.value("id").toString();  // выделим из шаблона id
       jObj_out.insert("id", uuid_com);            // заполним поле id (по данным jObj_in.id)
       jObj_out = m_verify_serv->getObjectSampl(name_com, suffix);
       processObject(jObj_in, jObj_out, FILL_SAME); // !!!! (возм. эта ф-ция не нужна)заполним другие совпадающие поля образца(по данным jObj_in),
    }
    else if (name_com == "ExchangeDCE"){
        jObj_out = m_verify_serv->getObjectSampl("ReplyExchangeDCE", suffix);
    }

    jObj_out = processParamList(jObj_out, param_list_out);  // дозаполняем ответ данными из списка параметров в зав. от команды

    return jObj_out;     // возвращаем  заполненный шаблон ответной команды (совпадающими полями из входящей команды)
}

  //-------------------------------------------------------------------------------------------
  // подготавливаем ответное сообщение об ошибке (если при разборе принятой последовательности выявлена ошибка)
  // может быть отправлена после предварительной ответной команды
  QJsonObject ProtocolServ::prepareErrorMsg(QString err_str, QJsonObject &jObj_in)
  {
      QJsonObject jObj_out;
      QString suffix{"_i"}; //?
      QString name_com = "Error";     // выделим из шаблона имя
      QString uuid_com = jObj_in.value("id").toString();// выделим из шаблона id
      // получим шаблон ответного сообщения из m_com_list (по данным jObj_in.cmd);
      jObj_out =  m_verify_serv->getObjectSampl(name_com, suffix);
      jObj_out.insert("id", uuid_com); // заполним поле id (по данным jObj_in.id)
      jObj_out.insert("errorMsg", err_str); // заполним поле id

      return jObj_out;     // возвращаем частично заполненный шаблон ответной команды
  }
//---------------------------------------------------------------------------------------------
 QJsonObject  ProtocolServ::processParamList(QJsonObject &jsonObj,  // команда ответная
                                             QJsonObject &param_list_out)
  {
     QString name_com = jsonObj.value("cmd").toString();    // выделим из шаблона имя и дозаполним шаблон в одной из функции ниже

           if(name_com == "Version"){
              versionRe(jsonObj,param_list_out);
              return jsonObj;
           }
           else if(name_com == "GetDCEInfo"){
               dceInfoRe(jsonObj, param_list_out);
               return jsonObj;
           }
           else if(name_com == "GetRadioDirInfo"){
               dirInfoRe(jsonObj, param_list_out);
               return jsonObj;
           }
           else if(name_com == "Status"){
               statusRe(jsonObj, param_list_out);
               return jsonObj;
           }
           else if(name_com == "Error"){
               errorRe(jsonObj, param_list_out);
               return jsonObj;
            }
           else if(name_com == "EnableDCE"){
               enableRe(jsonObj, param_list_out);
               return jsonObj;
           }
           else if(name_com == "DisableDCE"){
               disableRe(jsonObj, param_list_out);
               return jsonObj;
           }
           else if(name_com == "NetworkSYNC"){
               syncRe(jsonObj, param_list_out);
               return jsonObj;
           }
           else if(name_com == "NetworkCONNECT"){
               connectRe(jsonObj, param_list_out);
               return jsonObj;
           }
           else if(name_com == "NetworkDISCON"){
               disconRe(jsonObj, param_list_out);
               return jsonObj;
           }
           else if(name_com == "ReplyExchangeDCE"){
               replyRe(jsonObj, param_list_out);
               return jsonObj;
           }           
           else if(name_com == "ExchangeDCE"){
               exchangeRe(jsonObj, param_list_out);
               return jsonObj;
           }

         return jsonObj;
 }
 //--------------------------------------------------------------------------------------------
 //--------------------------------------------------------------------------------------------
 //               Подготовка инициативных сообщений клиенту
 //                  слоты связаны с сигн. из CoreDCE
 //--------------------------------------------------------------------------------------------
 //--------------------------------------------------------------------------------------------
 // Формирование ответного сообщения клиенту сигнал из модема
 // !!!! инициативные сообщения модема (из радио канала обрабатываются в другом слоте)
 // дискрипторы у таких команд = 0; а так же нет объекта входной команды
 void  ProtocolServ::slotCmdToClientFromChanel(QString cmd_name, QJsonObject &param_list_out)
 {
    //(param_list_out);   // доп. параметры из CoreDCE для формирования ответного сообщения клиенту
     //
     QJsonObject     json_sampl_out;
     MsgDescrS msg;
     json_sampl_out = prepareInitiativMsg(cmd_name, param_list_out);
     msg.obj_msg_out = json_sampl_out;
     msg.descr = 0;

     emit sigCmdToClient(msg);  // сигнал в Verify об ответном сообщении клиенту
 }
 //-----------------------------------------------------------------------------------------------
 // подготавливаем инициативное сообщение
 QJsonObject ProtocolServ::prepareInitiativMsg(QString cmd_name, QJsonObject &param_list_out)
 {
     QJsonObject jObj_out;
     QString suffix{"_i"}; //?
     //QString uuid_com;

     // получим шаблон для иниат. сообщения
     if(cmd_name != "ExchangeDCE"){
        jObj_out = m_verify_serv->getObjectSampl(cmd_name, suffix);
     }
     else if(cmd_name == "ExchangeDCE")
         jObj_out = m_verify_serv->getObjectSampl("ExchangeDCE", "_o");

     jObj_out = processParamList(jObj_out, param_list_out);  // дозаполняем ответ данными из списка параметров в зав. от команды

     return jObj_out;     // возвращаем  заполненный шаблон ответной команды (совпадающими полями из входящей команды)
 }
 //-----------------------------------------------------------------------------------------
 //                        ОТВЕТНЫЕ/ИНИциативные команды для отправки КЛИЕНТУ
 //-----------------------------------------------------------------------------------------
   void ProtocolServ::versionRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
       Q_UNUSED(param_list_out);

       QString field("list");
       QStringList field_name_list;
       QStringList field_val_list;

       QJsonArray objArr;
       QJsonObject objItem;

       field_name_list<<"version" << "build" <<"vendor"<<"description";
       field_val_list<<m_protocol_obj.version.version
                     <<m_protocol_obj.version.build
                     <<m_protocol_obj.version.vendor
                     <<m_protocol_obj.version.description;

       for (int i = 0; i < field_name_list.count(); ++i)
             objItem.insert(field_name_list.at(i), field_val_list.at(i));
       objArr.append(objItem);
     //  objArr.append(objItem);   // образец для добавления еще одного значения массива list:[]
     //  objArr.append(objItem);
       obj.insert(field, objArr);
   }
   //--------------------------------------------------------------------
   // ответ на команду о состоянии модема
   // ответ формируется в виде списка из 1 элемента
   void ProtocolServ::dceInfoRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
       QString field("dceList");
       QStringList field_name_list;
       QStringList field_val_list;

       QJsonArray objArr;
       QJsonObject objItem;

       if(param_list_out.contains("DCE_ID") && param_list_out.contains("stage") &&
          param_list_out.contains("busy") && param_list_out.contains("create_time") &&
          param_list_out.contains("enable_time") && param_list_out.value("DCE_ID").isDouble() &&
          param_list_out.value("stage").isString()&& param_list_out.value("busy").isString()&&
          param_list_out.value("create_time").isDouble() && param_list_out.value("enable_time").isDouble())
         {
           field_name_list<<"DCE_ID" << "port" <<"IP"<<"radioType"<<"stage"<<"busy"<< "creationTime"<<"enableTime";

           field_val_list << QString::number(param_list_out.value("DCE_ID").toInt())
                          << QString::number(m_protocol_obj.net_adr.port)
                          << m_protocol_obj.net_adr.address
                          << m_protocol_obj.radio_role
                          << param_list_out.value("stage").toString()
                          << param_list_out.value("busy").toString();

           for (int i = 0; i < 2; ++i){
                 objItem.insert(field_name_list.at(i), field_val_list.at(i).toInt());
           }

           for (int i = 2; i < field_name_list.count()-2; ++i){
                 objItem.insert(field_name_list.at(i), field_val_list.at(i));
           }
           objItem.insert(field_name_list.at(field_name_list.count()-2), param_list_out.value("create_time"));
           objItem.insert(field_name_list.at(field_name_list.count()-1), param_list_out.value("enable_time"));

           objArr.append(objItem);
         //  objArr.append(objItem);   // образец для добавления еще одного значения массива list:[]
         //  objArr.append(objItem);
           obj.insert(field, objArr);
        }
        else{
            QJsonObject obj_;
            obj_.insert("cmd", "Error");
            obj_.insert("errorMessage", "НЕТ данных по Модемy");
            obj = obj_;
        }
   }
   //--------------------------------------------------------------------
   // ответ на команду о созданном радио направлении
   //
   void ProtocolServ::dirInfoRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
       QString field("dirList");
       QStringList field_name_list;
       QStringList field_val_list;

       QJsonArray objArr;
       QJsonObject objItem;

       if(param_list_out.contains("DCE_ID") && param_list_out.contains("DCE_ID_OPPN") &&
          param_list_out.contains("s_n") && param_list_out.contains("err_RS") &&
          param_list_out.value("DCE_ID").isDouble() && param_list_out.value("DCE_ID_OPPN").isDouble()&&
          param_list_out.value("s_n").isDouble() && param_list_out.value("err_RS").isDouble())
         {
           field_name_list<<"DCE_ID" << "DCE_ID_OPPN" <<"sn"<<"errRS";

           field_val_list << QString::number(param_list_out.value("DCE_ID").toInt())
                          << QString::number(param_list_out.value("DCE_ID_OPPN").toInt())
                          << QString::number(param_list_out.value("s_n").toInt())
                          << QString::number(param_list_out.value("err_RS").toInt());

           for (int i = 0; i < field_name_list.count(); ++i)
                 objItem.insert(field_name_list.at(i), field_val_list.at(i).toInt());

           if(param_list_out.value("DCE_ID_OPPN").toInt() != 0){
              objArr.append(objItem);
           }
           obj.insert(field, objArr);
        }
        else{
            QJsonObject obj_;
            obj_.insert("cmd", "Error");
            obj_.insert("errorMessage", "НЕТ данных по созданным радионаправлениям.");
            obj = obj_;
        }

   }
   //--------------------------------------------------------------------
   void ProtocolServ::statusRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
       QStringList field_name_list;
       QStringList field_val_list;
       QString field("statusList");
       QJsonObject objItem;

       if(param_list_out.contains("DCE_ID") && param_list_out.contains("create_time") &&
          param_list_out.contains("enable_time") && param_list_out.value("DCE_ID").isDouble() &&
          param_list_out.value("create_time").isDouble()&& param_list_out.value("enable_time").isDouble())
         {
           field_name_list << "DCE_ID" << "creationTime" << "enableTime";

           objItem.insert(field_name_list.at(0), param_list_out.value("DCE_ID"));
           objItem.insert(field_name_list.at(1), param_list_out.value("create_time"));
           objItem.insert(field_name_list.at(2), param_list_out.value("enable_time"));

           obj.insert(field, objItem);
        }
        else{
            QJsonObject obj_;
            obj_.insert("cmd", "Error");
            obj_.insert("errorMessage", "НЕТ данных о статусе модема.");
            obj = obj_;
        }
   }
   //------------------------------------------------------------------
   void ProtocolServ::enableRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
     uint32_t dce_id{0};
     if(obj.contains("DCE_ID"))
         dce_id  = obj.value("DCE_ID").toInt(); // id присвоенный данному УВС

       // поверяем переданный параметр и формируем объект для ответа клиенту
     if(param_list_out.contains("turn_on") && param_list_out.value("turn_on")== true) {
           // m_uvs_obj.enable_time = QDateTime::currentDateTime().toLocalTime().toString("yyyy-MM-dd hh:mm:ss");
     //    m_uvs_obj.enable_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
     //    m_uvs_obj.stage = ENABLE;
     //    m_uvs_obj.DCE_ID = dce_id;                 // заполняем номер УВС номером пришедшем в команде АУ при отправке ответного сообщения для АУ
                                                       // теперь при формировании всех последующих команд необх. сверять их значения с этим номером.
             // emit sigChangeFormTitle("ПАК УВС # " + QString::number(dce_id)); // выводим номер на MW
             // команда отправляем как есть не корректируя
      }
      else{
          obj.insert("cmd", "Error");
          obj.insert("errorMessage", "Модем с DCE_ID: "+ QString::number(dce_id)+" НЕ ВКЛ.!");
      }
   }
   //------------------------------------------------------------------
   void ProtocolServ::disableRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
     uint32_t dce_id{0};
     if(obj.contains("DCE_ID"))
        dce_id  = obj.value("DCE_ID").toInt(); // id присвоенный данному УВС

        // поверяем переданный параметр и формируем объект для ответа клиенту
     if(param_list_out.contains("turn_on") && param_list_out.value("turn_on")== false) {

          //  emit sigChangeFormTitle("ПАК УВС # ... " );// убираем номер УВС после отключения на MW
      //  m_uvs_obj.DCE_ID = 0xffff;   // пока номер УВС не определен
      //  obj.insert("DCE_ID", 0xffff);
       // m_uvs_obj.enable_time = 0; // сообщаем о том что УВС был выключен
       // m_uvs_obj.stage = INIT;
     }
     else{
         obj.insert("cmd", "Error");
         obj.insert("errorMessage", "Модем с DCE_ID: "+ QString::number(dce_id)+" НЕ ВЫКЛ.!");
     }
   }
 //------------------------------------------------------------------
 void ProtocolServ::syncRe(QJsonObject & obj, QJsonObject &param_list_out)
 {
    uint32_t dce_id{0};
    uint32_t dce_id_dst{0};

   if(obj.contains("DCE_ID"))
      dce_id = obj.value("DCE_ID").toInt();
   if(obj.contains("DCE_ID_OPPN"))
      dce_id_dst = obj.value("DCE_ID_OPPN").toInt();

   if(param_list_out.contains("internalTime") && param_list_out.contains("status") && param_list_out.contains("id") &&
       param_list_out.contains("DCE_ID") && param_list_out.contains("DCE_ID_OPPN") &&
       param_list_out.value("internalTime").isArray() && param_list_out.value("status").isString()&& param_list_out.value("id").isString()&&
       param_list_out.value("DCE_ID").isDouble() && param_list_out.value("DCE_ID_OPPN").isDouble())
      {
        obj.insert("id", param_list_out.value("id").toString());
        obj.insert("DCE_ID", param_list_out.value("DCE_ID").toInt());
        obj.insert("DCE_ID_OPPN", param_list_out.value("DCE_ID_OPPN").toInt());
        obj.insert("internalTime", param_list_out.value("internalTime").toArray());
        obj.insert("status",  param_list_out.value("status").toString());
        // возможно добавить поля imitationIns,  synchroPack
      }
    else{
          QJsonObject obj_;
          obj_.insert("cmd", "Error");
          obj_.insert("errorMessage", "НЕТ синхронизации Модемов с DCE_ID: "+ QString::number(dce_id)+" и " +  QString::number(dce_id_dst));
          obj = obj_;
      }

 }
   //------------------------------------------------------------------
   // заполнение ответного сообщения данными
   void ProtocolServ::connectRe(QJsonObject & obj,  QJsonObject &param_list_out)
   {
       uint32_t dce_id{0};
       uint32_t dce_id_dst{0};

       if(obj.contains("DCE_ID"))
          dce_id = obj.value("DCE_ID").toInt();
       if(obj.contains("DCE_ID_OPPN")){
          dce_id_dst = obj.value("DCE_ID_OPPN").toInt();
       }


       if(param_list_out.contains("freqRX") && param_list_out.contains("freqTX") &&
          param_list_out.contains("status") && param_list_out.contains("id") &&
          param_list_out.contains("DCE_ID") && param_list_out.contains("DCE_ID_OPPN") &&
          param_list_out.value("DCE_ID").isDouble() && param_list_out.value("DCE_ID_OPPN").isDouble() &&
          param_list_out.value("freqRX").isDouble() && param_list_out.value("freqTX").isDouble() &&
          param_list_out.value("status").isString()&& param_list_out.value("id").isString())
          {
              obj.insert("id", param_list_out.value("id").toString());
              obj.insert("DCE_ID", param_list_out.value("DCE_ID").toInt());
              obj.insert("DCE_ID_OPPN", param_list_out.value("DCE_ID_OPPN").toInt());
              obj.insert("freqRX", param_list_out.value("freqRX").toInt());
              obj.insert("freqTX", param_list_out.value("freqTX").toInt());
              obj.insert("status",  param_list_out.value("status").toString());
           // возможно проверить и добавить другие поля NetworkConnect
          }
          else{
              QJsonObject obj_;
              obj_.insert("cmd", "Error");
              obj_.insert("errorMessage", "НЕТ  установленого соединения Модемов с DCE_ID: "+ QString::number(dce_id)+" и " +  QString::number(dce_id_dst));
              obj = obj_;
          }
   }

   //------------------------------------------------------------------
  void  ProtocolServ::disconRe(QJsonObject & obj, QJsonObject &param_list_out)
   {

      uint32_t dce_id{0};
      uint32_t dce_id_dst{0};

      if(obj.contains("DCE_ID"))
         dce_id = obj.value("DCE_ID").toInt();
      if(obj.contains("DCE_ID_OPPN")){
         dce_id_dst = obj.value("DCE_ID").toInt();
      }

     if(param_list_out.contains("status") && param_list_out.contains("id") &&
        param_list_out.value("status").isString()&& param_list_out.value("id").isString()&&
        param_list_out.contains("DCE_ID") && param_list_out.contains("DCE_ID_OPPN") &&
        param_list_out.value("DCE_ID").isDouble() && param_list_out.value("DCE_ID_OPPN").isDouble())
     {
         obj.insert("id", param_list_out.value("id").toString());
         obj.insert("DCE_ID", param_list_out.value("DCE_ID").toInt());
         obj.insert("DCE_ID_OPPN", param_list_out.value("DCE_ID_OPPN").toInt());
         obj.insert("status",  param_list_out.value("status").toString());

      }
     else{
          QJsonObject obj_;
          obj_.insert("cmd", "Error");
          obj_.insert("errorMessage", "НЕТ разрыва соединения Модемов с DCE_ID: "+ QString::number(dce_id)+" и " +  QString::number(dce_id_dst));
          obj = obj_;
         }

   }
   //------------------------------------------------------------------
   void ProtocolServ::exchangeRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
       int dce_id{0};
       int dce_id_dst{0};

       if(obj.contains("DCE_ID"))
          dce_id = obj.value("DCE_ID").toInt();
       if(obj.contains("DCE_ID_DST")){
          dce_id_dst = obj.value("DCE_ID_DST").toInt();
       }


       if(param_list_out.contains("data") && param_list_out.contains("MSG_ID") && param_list_out.contains("type") &&
          param_list_out.contains("DCE_ID") && param_list_out.contains("DCE_ID_DST") &&
          param_list_out.value("DCE_ID").isDouble() && param_list_out.value("DCE_ID_DST").isDouble()&&
          param_list_out.value("data").isString() &&  param_list_out.value("MSG_ID").isDouble() && param_list_out.value("type").isString())
          {
              obj.insert("data", param_list_out.value("data").toString());
              obj.insert("MSG_ID",  param_list_out.value("MSG_ID").toInt());
              obj.insert("type", param_list_out.value("type").toString());
              obj.insert("DCE_ID", param_list_out.value("DCE_ID").toInt());
              obj.insert("DCE_ID_DST", param_list_out.value("DCE_ID_DST").toInt());

          }
          else{
              QJsonObject obj_;
              obj_.insert("cmd", "Error");
              obj_.insert("errorMessage", "НЕТ поля data, MSG_ID или type для передачи в инициативном сообщении Модема с DCE_ID_DST: "+ QString::number(dce_id_dst));
              obj = obj_;
          }

   }
   //------------------------------------------------------------------
   void ProtocolServ::replyRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
       uint32_t dce_id{0};
       uint32_t dce_id_dst{0};

       if(obj.contains("DCE_ID"))
          dce_id = obj.value("DCE_ID").toInt();
       if(obj.contains("DCE_ID_DST")){
          dce_id_dst = obj.value("DCE_ID").toInt();

       }

       if(param_list_out.contains("status") && param_list_out.contains("MSG_ID") &&
            param_list_out.contains("DCE_ID") && param_list_out.contains("DCE_ID_DST") &&
            param_list_out.contains("type") && param_list_out.contains("msgsInQueue") &&
            param_list_out.contains("bytesInQueue") && param_list_out.value("status").isString() &&
            param_list_out.value("DCE_ID").isDouble() && param_list_out.value("DCE_ID_DST").isDouble()&&
            param_list_out.value("MSG_ID").isDouble() && param_list_out.value("type").isString() &&
            param_list_out.value("msgsInQueue").isDouble() && param_list_out.value("bytesInQueue").isDouble())
          {
              obj.insert("status", param_list_out.value("status").toString());
              obj.insert("MSG_ID",  param_list_out.value("MSG_ID").toInt());
              obj.insert("msgsInQueue",  param_list_out.value("msgsInQueue").toInt());
              obj.insert("bytesInQueue",  param_list_out.value("bytesInQueue").toInt());
              obj.insert("type", param_list_out.value("type").toString());
              obj.insert("DCE_ID", param_list_out.value("DCE_ID").toInt());
              obj.insert("DCE_ID_DST", param_list_out.value("DCE_ID_DST").toInt());
          }
          else{
              QJsonObject obj_;
              obj_.insert("cmd", "Error");
              obj_.insert("errorMessage", "НЕТ поля(полей): status, MSG_ID, type, msgsInQueue или bytesInQueue \n для передачи в ответном сообщении Модема с DCE_ID_DST: "+ QString::number(dce_id_dst));
              obj = obj_;
          }
   }
   //------------------------------------------------------------------
   void ProtocolServ::errorRe(QJsonObject & obj, QJsonObject &param_list_out)
   {
       Q_UNUSED(obj);
       Q_UNUSED(param_list_out);
      // команду ОШИБКА отправляем как есть не корректируя
   }

//-----------------------------------------------------------------------------------------------
 // подготовка ответной команды со статусом start
 /* QJsonObject ProtocolServ::prepareStartAnswMsg(QJsonObject &jObj_in)
  {
      QJsonObject jObj_out;

      QString name_com = jObj_in.value("cmd").toString(); // выделим из шаблона имя

      QString suffix{"_i"};

      // получим шаблон для ответного сообщения из m_com_list (по данным jObj_in.cmd);
      if (name_com != "ExchangeDCE"){
           QString uuid_com = jObj_in.value("id").toString();  // выделим из шаблона id
         // jObj_out = m_wdg_dce->getSamplObj(name_com, suffix);
          jObj_out.insert("id", uuid_com);             // заполним поле id (по данным jObj_in.id)
          processObject(jObj_in, jObj_out, FILL_SAME); // заполним другие совпадающие поля образца(по данным jObj_in),
                                                       //если что поправим их из модуля MainWind
      }
      else if(name_com == "ExchangeDCE"){
        // jObj_out = m_parent.getSamplObj("ReplyExchangeDCE",suffix);
       //  jObj_out.insert("id", uuid_com);
         jObj_out.insert("type",jObj_in.value("type").toString());
         jObj_out.insert("DCE_ID",jObj_in.value("DCE_ID").toInt());
         jObj_out.insert("MSG_ID",jObj_in.value("MSG_ID").toInt());
         if(m_use_dst){
            jObj_out.insert("DCE_ID_DST",jObj_in.value("DCE_ID_DST").toInt());
            if(jObj_out.contains("DCE_ID_SRC"))
              jObj_out.remove("DCE_ID_SRC");
         }
         else{
             jObj_out.insert("DCE_ID_OPPN",jObj_in.value("DCE_ID_DST").toInt());
             if(jObj_out.contains("DCE_ID_SRC"))
                      jObj_out.remove("DCE_ID_SRC");
             if(jObj_out.contains("DCE_ID_DST"))
                      jObj_out.remove("DCE_ID_DST");
            // jObj_out.insert("DCE_ID_DST",jObj_in.value("DCE_ID_DST").toInt());
          //   jObj_out.insert("MSG_ID",jObj_in.value("MSG_ID").toInt());
           //  if(jObj_out.contains("DCE_ID_SRC"))
            //   jObj_out.remove("DCE_ID_SRC");
         }

       //  jObj_out.insert("msgsInQueue",m_queue.count()+1);
         int num = 0;
        // for (int i = 0; i < m_queue.count(); ++i){
        //     num = num + decodingGettingObject(m_queue.at(i).obj_msg_in);
            //num = num + m_queue.at(i).value("data").toString().length();
        // }
         jObj_out.insert("bytesInQueue", num); // кол-во байт в очеренди
      }
      jObj_out.insert("status", "start");
      return jObj_out;     // возвращаем частично заполненный шаблон ответной команды (совпадающими полями из входящей команды)
  }
  */
 //----------------------------------------------------------------------------------------------
 // подготовка ответной команды со статусом end
/*  QJsonObject ProtocolServ::prepareEndAnswMsg(QJsonObject &jObj_in)
  {
      QJsonObject jObj_out;
      QString suffix{"_i"};

      QString name_com = jObj_in.value("cmd").toString(); // выделим из шаблона имя
      QString uuid_com; // = jObj_in.value("id").toString();  // выделим из шаблона id

      // получим шаблон для ответного сообщения из m_com_list (по данным jObj_in.cmd);
      if(name_com != "ExchangeDCE"){
          uuid_com = jObj_in.value("id").toString();  // выделим из шаблона id
        // jObj_out = m_parent.getSamplObj(name_com, suffix);
         jObj_out.insert("id", uuid_com);             // заполним поле id (по данным jObj_in.id)
         processObject(jObj_in, jObj_out, FILL_SAME); // заполним другие совпадающие поля образца(по данным jObj_in),

      }
      else if(name_com == "ExchangeDCE"){
          //  jObj_out = m_parent.getSamplObj("ReplyExchangeDCE", suffix);
           // jObj_out.insert("id", uuid_com);

            jObj_out.insert("type",jObj_in.value("type").toString());
            jObj_out.insert("DCE_ID",jObj_in.value("DCE_ID").toInt());
            jObj_out.insert("MSG_ID",jObj_in.value("MSG_ID").toInt());
            if(m_use_dst){
               jObj_out.insert("DCE_ID_DST",jObj_in.value("DCE_ID_DST").toInt());
               if(jObj_out.contains("DCE_ID_SRC"))
                  jObj_out.remove("DCE_ID_SRC");
            }
            else{
                jObj_out.insert("DCE_ID_OPPN",jObj_in.value("DCE_ID_DST").toInt());
                if(jObj_out.contains("DCE_ID_SRC"))
                         jObj_out.remove("DCE_ID_SRC");
                if(jObj_out.contains("DCE_ID_DST"))
                         jObj_out.remove("DCE_ID_DST");
            }

       //     jObj_out.insert("msgsInQueue",m_queue.count());
            int num = 0;
           // for (int i = 0; i < m_queue.count(); ++i){
           //     num = num + decodingGettingObject(m_queue.at(i).obj_msg_in);
             //  num = num + m_queue.at(i).value("data").toString().length();
           // }
            jObj_out.insert("bytesInQueue", num); // кол-во байт в очеренди
      }
      jObj_out.insert("status", "end");
      return jObj_out;     // возвращаем частично заполненный шаблон ответной команды (совпадающими полями из входящей команды)
  }
  */
  //----------------------------------------------------------------------------------------------
  // подготовка ответной команды со статусом enqueued
 /*  QJsonObject ProtocolServ::prepareEnqueuedAnswMsg(QJsonObject &jObj_in)
   {
       QJsonObject jObj_out;

       QString name_com = "ReplyExchangeDCE";
     //  QString uuid_com = jObj_in.value("id").toString();  // выделим из шаблона id
       QString suffix{"_i"};

     //  jObj_out = m_parent.getSamplObj(name_com, suffix);
      // jObj_out.insert("id", uuid_com);             // заполним поле id (по данным jObj_in.id)
       // !!!!! если будут ошибки это место повнимательнее
       //  processObject(jObj_in, jObj_out, FILL_SAME); // заполним другие совпадающие поля образца(по данным jObj_in),
                                                    //если что поправим их из модуля MainWind
       jObj_out.insert("status", "enqueued");
       jObj_out.insert("type",jObj_in.value("type").toString());
       jObj_out.insert("DCE_ID",jObj_in.value("DCE_ID").toInt());
       jObj_out.insert("MSG_ID",jObj_in.value("MSG_ID").toInt());
       if(m_use_dst){
          jObj_out.insert("DCE_ID_DST",jObj_in.value("DCE_ID_DST").toInt());
          if(jObj_out.contains("DCE_ID_SRC"))
             jObj_out.remove("DCE_ID_SRC");
       }
       else{
           jObj_out.insert("DCE_ID_OPPN",jObj_in.value("DCE_ID_DST").toInt());
           if(jObj_out.contains("DCE_ID_SRC"))
                    jObj_out.remove("DCE_ID_SRC");
           if(jObj_out.contains("DCE_ID_DST"))
                    jObj_out.remove("DCE_ID_DST");
       }

     //     jObj_out.insert("msgsInQueue",m_queue.count());

       int num = 0;
      // for (int i = 0; i < m_queue.count(); ++i){
      //     num = num + decodingGettingObject(m_queue.at(i).obj_msg_in);
          //num = num + m_queue.at(i).value("data").toString().length();
      // }
       jObj_out.insert("bytesInQueue", num); // кол-во байт в очеренди

       return jObj_out;     // возвращаем частично заполненный шаблон ответной команды (совпадающими полями из входящей команды)
 }
*/
//-------------------------------------------------------------------------------------
/*QJsonObject ProtocolServ::fillCommandRePlus(QJsonObject & jsonObj)
{
    QString name_com = jsonObj.value("cmd").toString();    // выделим из шаблона имя и дозаполним шаблон в одной из функции ниже

    if(name_com == "NetworkSYNC"){
        syncEndRe(jsonObj);
        return jsonObj;
    }
    else if(name_com == "NetworkCONNECT"){
        connectEndRe(jsonObj);
        return jsonObj;
    }
    else if(name_com == "NetworkDISCON"){   // вроде :end у ком. DISCONect - нету
     //   disconStartRe(jsonObj);
        return jsonObj;
    }
    return jsonObj;
}
*/


  //--------------------------------------------------------------------------------------------
/*  int ProtocolServ::decodingDataLength(QJsonValue val)
  {
      int len;
      QByteArray bb, ba;

      bb.append(val.toString().toUtf8());       // зашифрованные данные в байтах помещаем в байтовый массив
      ba.append(QByteArray::fromBase64(bb.data()));// декодируем принятые байты и помещаем в байтовый массив
      len = ba.length();
      return len;
  }
  //-------------------------------------------------------------------------------------
  int ProtocolServ::decodingGettingObject(QJsonObject json_in)
  {
      QJsonValue objVal;
      int len{0};
      if(json_in.contains("data")){
        objVal = json_in.value("data");
        len = decodingDataLength(objVal);
       }
      return len;
  }
  */
  //----------------------------------------------------------------------------------------
  // процедура рекурсивной обработки json объектов()
  void ProtocolServ::processObject(QJsonObject & obj_in,   // объект принятый, сформированный из входящей последовательности
                              QJsonObject & obj_samp,      // объект эталонный, сформированный по данным m_com_list т.е.из файла command_list
                              ModeEnum mode)               // режим обработки (CHECK - проверяется входящий объект на соответствие с эталонным,
                                                           //                  FILL_SAME - после того как входной объект проверен, по некоторым
                                                           //его данным + эталонный объект ->формируется формирутся объект отвертной команды)
  {
     if((!obj_in.isEmpty()) && (!obj_samp.isEmpty()) ){    // если переданные объекты не пустые
        for (int i = 0; i < obj_samp.count(); ++i){
            QString key_samp = obj_samp.keys().at(i);           // очередной ключ в объекте эталоне
            QJsonValue val_in = obj_in.value(key_samp);         // значение того же ключа в входном объекте
            QJsonValue val_samp = obj_samp.value(key_samp);     // значение ключа в эталонном пакете
            bool s, d, a, o, sd, ds = false;                    // флаги для определения типа значения из данного ключа

            double val_in_d {0};
            QString val_in_s{""};

                                 // определяем считанные значения из входного объекта, и эталонного объекта
            s = val_in.isString() && val_samp.isString();
            d = val_in.isDouble() && val_samp.isDouble();
            sd = val_in.isDouble() && val_samp.isString();
            ds = val_in.isString() && val_samp.isDouble();
            a = val_in.isArray() && val_samp.isArray();
            o = val_in.isObject() && val_samp.isObject();

           if(obj_in.contains(key_samp) && (a|d|s|o|ds|sd)){ // если есть совпадение названия полей и их типов

               if(d|s){                                // если совпадение по типу строки или числа
                  obj_samp.insert(key_samp, val_in);   // пришедшее значение записываем в эталонный объект
               }   // обработка строки ли числа

               else if (sd){
                   val_in_s = val_in.toString();
                   obj_in.insert(key_samp, val_in_s);
               }
               else if (ds){
                   val_in_d = val_in.toDouble();
                   obj_in.insert(key_samp, val_in_d);
               }
               else if (a){  // если совпадение по типу массив
                                // проверяем поля и значения массива
                   QJsonArray ja_in = val_in.toArray();
                   QJsonArray ja_samp = val_samp.toArray();

                   for (int j = 0; j < ja_in.count(); ++j){
                       if(ja_in.at(j).isObject()){       // если значения массива - объекты =>
                         QJsonObject subTr_in = ja_in.at(j).toObject();
                         QJsonObject subTr_samp = ja_samp.at(j).toObject();
                         processObject(subTr_in, subTr_samp, mode); // рекурсивно вызываем эту же функцию
                                                                    //передава ей элемены массивов для дальнейщего обхода
                       }
                       else if (ja_in.at(j).isDouble() || ja_in.at(j).isString() ||ja_in.at(j).isBool()){
                               obj_samp.insert(key_samp, val_in);   // пришедшее значение записываем в эталонный объект
                               break;   // выходим из цикла j предполагая что все элемнты массива одного типа , не объекты
                       }
                   }
               }   // обработка массива

               else if (o){   // если совпадение по типу объект
                  // проверяем поля и значения объекта
                  QJsonObject jo_in = val_in.toObject();
                  QJsonObject jo_sampl = val_samp.toObject();
                  processObject(jo_in, jo_sampl, mode);      // рекурсивно вызываем эту же функцию
              } // обработка объекта
          }  // поля в эталоне и входящем объекте - имеются и совпадают по типу

          else {  // если в пришедшей последовательности нужного поля нет или тип не совпадает
              if(mode == CHECK){  // сообщение о несовпадении входного и эталонного объекта по ключам требуется только для режима проверки
                // QMessageBox::critical(0, "Сообщение","В сообщении JSON ошибка структуры команды: ("+key_samp+")" ,"");
                 m_err = " Верификация. Структура команды: " + key_samp;
               }
             } // выдаем ошибку структуры JSON сообщения
         }   // обход эталонного объекта завершен
       }     // входной и эталонный объекты, переданные в функцию не пустые
     }
//-------------------------------------------------------------------------------------------------------
  //----------Сделано по просьбе Ивана ---------------------------------
  // переносим в CoreDCE
  //-------------------------------------------------------------------
  // установка флага занятости модема с формы
/*  void ProtocolServ::slotSetBusy(bool busy)
  {
      if (busy)
          m_uvs_obj.busy = 1;
      else
          m_uvs_obj.busy = 0;

  }
  //--------------------------------------------------------------------
  // установка флага состояния модема с формы
  void ProtocolServ::slotSetStage(int index)
  {
      switch (index){
        case 1: m_uvs_obj.stage = INIT;
                m_uvs_obj.enable_time = 0;
              break;
        case 2: m_uvs_obj.stage = ENABLE;
                m_uvs_obj.enable_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
              break;
        case 3: m_uvs_obj.stage = NET_SYNC;
                m_uvs_obj.DCE_ID_OPPN = 21;
                m_uvs_obj.enable_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
              break;
        case 4: m_uvs_obj.stage = NET_CONNECT;
                m_uvs_obj.DCE_ID_OPPN = 21;
                m_uvs_obj.enable_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
            break;
        case 5: m_uvs_obj.stage = NET_DISC;
                m_uvs_obj.DCE_ID_OPPN = 0;
                m_uvs_obj.enable_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
          break;
        case 6: m_uvs_obj.stage = EXCH;
                m_uvs_obj.DCE_ID_OPPN = 21;
                m_uvs_obj.enable_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
          break;
        default : m_uvs_obj.stage =INIT;
                  m_uvs_obj.DCE_ID_OPPN = 21;
                  m_uvs_obj.enable_time = 0;
             break;
        }
  }
  */
  //-------------------------------------------------------------------
// пока это не нужно!
//-------------------------------------------------------------------------------------------------------
 /* void ProtocolServ::setQueueDepth(int depth)
  {
      m_queue_depth = depth;
  }
  */
  //-------------------------------------------------------------------
 /* void ProtocolServ::disconnectExch()
  {
      m_com_wr[0]=0;
      m_com_wr[1]=0;
      m_com_wr[2]=0;
  }
  */
  //-------------------------------------------------------------------
  // иммитация отправки кодограммы в Сервер Обмена
 /* void ProtocolServ::writeCodoInExch(QJsonObject &jobj)
  {
      // сигнал о помещении команды в радио канал происходит в зависимости от состояния
      // соответств. этой команде счетчика.
      // счетчик обнуляется при возникновеннии ошибки !(m_timer_ex) при прохождении команды и при отработки команды ENABLE_DCE

          if(jobj.contains("status"))
             jobj.insert("status", "");    // считаем, что сообщение которое идет через радио канал  имеет поле status == ""
          QString name_com = jobj.value("cmd").toString();    // выделим из шаблона имя

          if(name_com == "NetworkSYNC"){
              ++m_com_wr[0];
              if(m_com_wr[0] < 2){
                 emit sigCodoToExch(jobj);
              }
          }
          else if(name_com == "NetworkCONNECT"){
              ++m_com_wr[1];
              if(m_com_wr[1] < 2){
                 emit sigCodoToExch(jobj);
              }
          }
          else if(name_com == "NetworkDISCON"){
              ++m_com_wr[2];
              if(m_com_wr[2] < 2){
                 emit sigCodoToExch(jobj);
              }
          }
          else if(name_com == "ExchangeDCE"){
             // ++m_com_wr[3];
             // if(m_com_wr[3] < 2){
              emit sigCodoToExch(jobj);
             // }
          }
          else if (name_com == "EnableDCE"){   // при приходе команды счетчики отправки в радио канал - обнуляются для всех команд
              for (int i = 0; i < 4; ++i)
                  m_com_wr[i] = 0;
          }

          // здесь необходимо распарсить json-объект и нужную информацию из этого объекта отправить в радио канал,
          // вызвав соответствующую функцию из модуля Влад.Алекс.Соколова, передав этой функции необходимые параметры
          // являющиеся полями json-объекта
      }
  */

  //-------------------------------------------------------------------------------------------
  // подготовка ответной команды со статусом recv
 /*  QJsonObject ProtocolServ::prepareRecvAnswMsg(QJsonObject &jObj_in)
   {
       QJsonObject jObj_out;
       QString suffix{"_i"}; //?
       QString name_com = jObj_in.value("cmd").toString(); // выделим из шаблона имя
       QString uuid_com; //= jObj_in.value("id").toString();  // выделим из шаблона id

       if(name_com != "ExchangeDCE" || name_com != "ReplyExchangeDCE"){
             uuid_com = jObj_in.value("id").toString();  // выделим из шаблона id
             jObj_out.insert("id", uuid_com);             // заполним поле id (по данным jObj_in.id)
       }
       // получим шаблон для ответного сообщения из m_com_list (по данным jObj_in.cmd);
       jObj_out = m_wdg_dce->getSamplObj(name_com, suffix);
      // jObj_out.insert("id", uuid_com);             // заполним поле id (по данным jObj_in.id)
       processObject(jObj_in, jObj_out, FILL_SAME); // заполним другие совпадающие поля образца(по данным jObj_in),
                                                    //если что поправим их из модуля MainWind
       jObj_out.insert("status", "recv");

       return jObj_out;     // возвращаем частично заполненный шаблон ответной команды (совпадающими полями из входящей команды)

   }
   */
  //-----------------------------------------------------------------------------------------------
  // опустошение очереди
  /*void ProtocolServ::queueDequeue()
  {
     QJsonObject jObj_out;
     if(m_queue.count() > 0){
         m_current_msg = m_queue.dequeue();                 // выбираем объект из очереди предназначенный для отправки

         jObj_out = prepareStartAnswMsg(m_current_msg.obj_msg_in);      // формируем ответное сообщение со статусом start
         m_current_msg.obj_msg_out = jObj_out;
         emit sigCmdReReady(m_current_msg);                       // отправляем ответ клиенту
         int len_data = decodingGettingObject(m_current_msg.obj_msg_in);
         m_time_slot_pass = (len_data * 8 *1000 / m_rate);
         m_obj_pass = false;
       //  m_slot_timer_pass->start(m_time_slot_pass);           // иммитация начала отправки в радиосеть
        // m_slot_timer_pass->singleShot(m_time_slot_pass, this, SLOT(slotPassTimerAlarm()));
        QTimer::singleShot(m_time_slot_pass, this, SLOT(slotPassTimerAlarm()));
       }
  }
  */
  //--------------------------------------------------------------------------
  // обработка таймера отправки слота в радиоканал (Сервер обмена)
  /*void ProtocolServ::slotPassTimerAlarm()
  {
      //m_slot_timer_pass->stop();
      QJsonObject obj_for_exch_serv = m_current_msg.obj_msg_in;
      QString name_com = m_current_msg.obj_msg_in.value("cmd").toString();
      if (name_com == "ExchangeDCE"){
         if(m_use_dst){

         }
         else{   // если не используем ID_DST  то в сообщении поле DCE_ID_DST, переделываем его перед отправкой дальше в молуль серв обменат.
             if(obj_for_exch_serv.contains("DCE_ID_DST")){
                 obj_for_exch_serv.insert("DCE_ID_OPPN",obj_for_exch_serv.value("DCE_ID_DST").toInt());
                 obj_for_exch_serv.remove("DCE_ID_DST");
                 obj_for_exch_serv.remove("DCE_ID_SRC");
              }
         }
      }
      writeCodoInExch(obj_for_exch_serv); // в сервер обмена отправляем раскодированное поле data

      if (name_com == "ExchangeDCE"){
          MsgDescrS msg;
          msg = m_current_msg;
          msg.obj_msg_out = prepareEndAnswMsg(m_current_msg.obj_msg_in); // формируем ответное сообщение со статусом end Без изменения имя поля

          emit sigCmdReReady(msg);                            // отправляем ответ клиенту
          m_obj_pass = true;
          queueDequeue();
         // m_slot_timer_get->start(m_time_slot_get);
       }
  }
  */
  //---------------------------------------------------------------------------
   // получение кодограммы из сервера обмена
  /* void ProtocolServ::сodoFromExch(QJsonObject & obj_in)
   {
       QJsonObject jObj_out;
       MsgDescrS msg;
       QString name_com = obj_in.value("cmd").toString();
       //m_current_obj = obj_in;
       msg.obj_msg_in = obj_in;
       QString prefix{"_i"};

       if (name_com == "ExchangeDCE"){
          jObj_out = obj_in;    //  информ. команда из сервера обмена
          if(m_use_dst){
              if(jObj_out.contains("DCE_ID_DST"))
                  jObj_out.remove("DCE_ID_DST");
          }
      //    else{   // если не используем ID_DST  то в сообщении поле DCE_ID_OPPN, переделываем его перед отправкой дальше в молуль верифик.
      //        if(jObj_out.contains("DCE_ID_OPPN")){
      //            jObj_out.insert("DCE_ID_SRC",jObj_out.value("DCE_ID_OPPN").toInt());
      //            jObj_out.remove("DCE_ID_OPPN");
      //         }
      //    }

      }
       else if (name_com == "NetworkSYNC"){
           jObj_out =  m_parent.getSamplObj(name_com, prefix); // управляющая ответная ком.
           jObj_out = prepareAnswMsg(obj_in);
           syncRecvRe(jObj_out);
       }
       else if (name_com == "NetworkCONNECT"){
           jObj_out =  m_parent.getSamplObj(name_com, prefix); // управляющая ответная ком.
           jObj_out = prepareAnswMsg(obj_in);
           connectRecvRe(jObj_out);
       }
       else if (name_com == "NetworkDISCON"){
           jObj_out =  m_parent.getSamplObj(name_com, prefix); // управляющая ответная ком.
           jObj_out = prepareAnswMsg(obj_in);
           disconRecvRe(jObj_out);
       }
       msg.obj_msg_out = jObj_out;
       msg.descr = 0;
       emit sigCmdReReady(msg);

   }
  */
  //--------------------------------------------------------------------------------------------
  /*bool ProtocolServ::checkEnquReplyExch(QJsonObject &jsonObj)
  {
      bool res = false;

      QString name_com = jsonObj.value("cmd").toString();
      if(name_com == "ReplyExchangeDCE"){
         if(jsonObj.contains("status") && jsonObj.value("status").isString()){  //
             QJsonValue objVal = jsonObj.value("status");
             if (objVal == "enqueued" )
             {
                 res = true;
             }
         }
      }
      return res;
  }
  //--------------------------------------------------------------------------------------------
  bool ProtocolServ::checkStartReplyExch(QJsonObject &jsonObj)
  {
      bool res = false;

      QString name_com = jsonObj.value("cmd").toString();
      if(name_com == "ReplyExchangeDCE"){
         if(jsonObj.contains("status") && jsonObj.value("status").isString()){  //
             QJsonValue objVal = jsonObj.value("status");
             if (objVal == "start" )
             {
                 res = true;
             }
         }
      }
      return res;
  }
  */
  //--------------------------------------------------------------------------
 // обработка таймера приема объекта из радиоканал (сервера обмена)
/* void ProtocolServ::slotGetTimerAlarm()
 {
    // m_slot_timer_get->stop();
    if(m_use_dst){
        m_current_msg.obj_msg_out.insert("DCE_ID",m_current_msg.obj_msg_in.value("DCE_ID_DST").toInt());
        m_current_msg.obj_msg_out.insert("DCE_ID_SRC",m_current_msg.obj_msg_in.value("DCE_ID").toInt());

        if(m_current_msg.obj_msg_out.contains("DCE_ID_DST"))
           m_current_msg.obj_msg_out.remove("DCE_ID_DST");
    }
    else{
        m_current_msg.obj_msg_out.insert("DCE_ID",m_current_msg.obj_msg_in.value("DCE_ID_OPPN").toInt());
        m_current_msg.obj_msg_out.insert("DCE_ID_OPPN",m_current_msg.obj_msg_in.value("DCE_ID").toInt());

    }

     emit sigCmdReReady(m_current_msg); // отправляем клиенту команду ExchangeDCE принятую из радио канала
 }
 */
 //------------------------------------------------------------------------
  /*void  ProtocolServ:: slotGetCmd(MsgDescrS & msg)       // !!!! обработка команды клиента (сигнал полученной от WidgetDCE)
   {

      m_err = "";              // поле ошибки пусто т.к. пакет только начинает обрабатываться

                            // определим тип принятого сообщения. От клинета серверу может прийти 3 типа сообщений :
                                     // 1. управл.команда;
                                     // 2. отв.сообщ.со status = verify(подтверждение имитовставки);
                                     // 3. отв. сообщ со status = recv(ReplayExchangeDCE)
                                     // 4. управл. команда ExchangeDCE

     if(checkVerifyAnswFromClient(msg.obj_msg_in)){            // относится к типу 2. ?
          msgVerifyReplayProcess(msg);              // обработка ответной команды со status = virify (подтверждение имитовставки)
     }
     else if(checkRecvReplyFromClient(msg.obj_msg_in)){        // относится к типу 3.?
          msgRecvReplayProcess(msg);                // обработка ответной команды со status = recv (ReplayExchangeDCE)
     }
     else if (checkExchFromClient(msg.obj_msg_in)){            // относится к типу 4.?
          msgControlExchangeCommandProcess(msg);    // обработка управляющей команды ExchangeDCE от клиента
     }
     else{                                             // значит это сообщение типа 1. - упр. команда
          msgControlCommandProcess(msg);            // обработка управляющей команды от клиента
     }
                       // m_error может быть  заполнено внутри функции обработки   msgRecvReplayProcess(),
                       // msgControlCommandProcess(), msgVerifyReplayProcess() и все эти ошибки попадут в рез. команду ERROR

     if(m_err !=""){   // если ошибки при разборе пришедшего пакета были
         msgErrorProcess(m_err, msg);      //обработка выявленных ошибок при разборе принятой команды
     }
  }
  */
  //-------------------------------------------------------------------------------------
  /*void ProtocolServ::setRatePass(int rate)
  {
      m_rate = rate;
  }
  */
