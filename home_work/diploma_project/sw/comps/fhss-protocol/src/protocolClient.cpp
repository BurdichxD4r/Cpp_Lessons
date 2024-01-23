
#include "protocolClient.h"

#include <QUuid>
#include <QMessageBox>
#include <QJsonObject>

const QString SUFFIX_Q {"_o"};   // query - команда-запрос
const QString SUFFIX_A {"_i"};   // answer - команда-ответ


//-----------------------------------------------------------------------
ProtocolClient::ProtocolClient(NetS & net_adr, ClientTypeEnum type_client, TypeMsgEnum type_msg, bool format_msg)
{

    m_verify_client = new VerificationClient(net_adr, type_msg, format_msg);

    m_client_type = type_client;

           // обработка события прихода пакета из CONNECTORDTE в MW_DTE(через ProtocolDTE)
    connect(m_verify_client, &VerificationBase::sigAddMsgToJournal, this, &ProtocolClient::slotPackFromTransport);
       // обработка события прихода состояния соединения из CONNECTORDTE в MW_DTE(через ProtocolDTE)
    connect(m_verify_client, &VerificationBase::sigChangeWorkStatus, this, &ProtocolClient::slotErrorFromTransport);
       // обработка события включения индикаторов состояния в MW_DTE(сигнал высылается из ProtocolDTE
       // после анализа пришедшего сообщения)
    connect (m_verify_client, &VerificationBase::sigCommandGet, this, &ProtocolClient::slotAnalyseMsgFromServer);
 //   connect(m_verify_client, &VerificationClient::sigPanelOnDTE, this, &ProtocolClient::slotPanelOn);
      // сигнал из MW_DTE для необходимости сформированного (или доформированного ) сообщения
      // в ProtocolDTE и последующей передачей  в Connector_DTE для отправке в ПАК УВС
    connect(this, &ProtocolClient::sigSendCommand, m_verify_client, &VerificationClient::slotSendCommandDTE);
      // сигнал из MW_DTE о разрыве связи с сервером. для передачи через PROTOCOLDTE в ConnectorDTE
    connect(this, &ProtocolClient::sigDiscon, m_verify_client, &VerificationBase::slotDisconnect);
}
//------------------------------------------------------------------------
ProtocolClient::~ProtocolClient()
{

}
//------------------------------------------------------------------------
void ProtocolClient::slotPackFromTransport(int k,  QByteArray & ba, QDateTime tm )
{
     emit sigAddMsgToJournal(k, ba, tm);
}
//------------------------------------------------------------------------
void ProtocolClient::slotErrorFromTransport(QString & str, QTime tm )
{
     emit sigChangeWorkStatus(str, tm);
}
//------------------------------------------------------------------------
void ProtocolClient::slotDiscon()
{
     emit sigDiscon(); // соединение разрывается
}
//-----------------------------------------------------------------------
void ProtocolClient::slotSendCommandDTE(QJsonObject & obj)
{
     emit sigSendCommand(obj);
}
//-----------------------------------------------------------------------
// для экземпляра ПРМ
// данные считываютс с экранной формы
void ProtocolClient::slotParityLenGet(int len)
{
   m_parity_L = len;
}
//------------------------------------------------------------------------
// для экземпляра ПРМ
// данные считываютс с экранной формы
void ProtocolClient::slotNoParityLenGet(int len)
{
    m_noparity_L = len;
}
//-----------------------------------------------------------------------
// для экземпляра ПРМ
// данные считываютс с экранной формы
void ProtocolClient::slotParityAmplGet(int ampl)

{
   m_parity_A = ampl;
}
//------------------------------------------------------------------------
// для экземпляра ПРМ
// данные считываютс с экранной формы
void ProtocolClient::slotNoParityAmplGet(int ampl)
{
    m_noparity_A = ampl;
}
//-----------------------------------------------------------------------
bool ProtocolClient::termTypeCmdSend()
{
    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);
    QString cmd = "TermType";
    QString name_term{"UNKN"};

    if (m_client_type == MAS)name_term = "MAS";
    else if (m_client_type == AU)   name_term = "AU";

    QJsonObject obj_sampl;

    if(m_verify_client!=nullptr){  // если объект протокола создан
          // получим из протокола шаблон команды для отправки
          obj_sampl = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
          // заполняем основные поля объекта

          QStringList field_name_list;
          QStringList field_val_list;

          field_name_list<< "id"<<"name";
          field_val_list<<uuid_str<<name_term ;
          for (int i = 0; i < field_name_list.count(); ++i){
                obj_sampl.insert(field_name_list.at(i), field_val_list.at(i));
          }
          emit sigSendCommand(obj_sampl);
          return true;    // сообщение сформировано и отправлено
      }
    return false;    // сообщение сформировано и отправлено
}
//-----------------------------------------------------------------------
bool ProtocolClient::pingCmdSend()
{
    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);
    QJsonObject obj_sampl;
    QString cmd = "Ping";

    if(m_verify_client!=nullptr){  // если объект проверки создан
            // получим из него шаблон команды для отправки
        obj_sampl = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
            // заполняем основные поля объекта

        QStringList field_name_list;
        QStringList field_val_list;

        field_name_list<< "id";
        field_val_list<< uuid_str ;
        for (int i = 0; i < field_name_list.count(); ++i){
             obj_sampl.insert(field_name_list.at(i), field_val_list.at(i));
         }
        emit sigSendCommand(obj_sampl);   // связь с verify_client::slotSendCommandDTE(jObj);
        return true;    // сообщение сформировано и отправлено
     }
     return false;         // сообщение не сформировано  или не отправлено
}
//-----------------------------------------------------------------------
bool ProtocolClient::testCmdSend()
{
    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);
    QJsonObject obj_sampl;
    QString cmd = "Test";

    if(m_verify_client!=nullptr){  // если объект проверки создан
            // получим из него шаблон команды для отправки
        obj_sampl = m_verify_client->getObjectSampl("Ping", SUFFIX_Q);  // берем образец для ком. Ping
            // заполняем основные поля объекта

        QStringList field_name_list;
        QStringList field_val_list;

        field_name_list<< "id"<<"param";
        field_val_list<< uuid_str <<"Hello!" ;

        for (int i = 0; i < field_name_list.count(); ++i){
             obj_sampl.insert(field_name_list.at(i), field_val_list.at(i));
         }
        // заменяем ключ cmd значением "Test"
        obj_sampl.remove("cmd");
        obj_sampl.insert("test",cmd);
        emit sigSendCommand(obj_sampl);   // связь с verify_client::slotSendCommandDTE(jObj);
        return true;    // сообщение сформировано и отправлено
     }
     return false;         // сообщение не сформировано  или не отправлено
}
//-----------------------------------------------------------------------
bool ProtocolClient::versionCmdSend()
{
    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);
    QString cmd = "Version";
    QJsonObject obj_sampl;

    if(m_verify_client!=nullptr){  // если объект протокола создан
               // получим из протокола шаблон команды для отправки
       obj_sampl = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
                  // заполняем основные поля объекта
       QStringList field_name_list;
       QStringList field_val_list;

       field_name_list << "id";
       field_val_list << uuid_str;

       for (int i = 0; i < field_name_list.count(); ++i){
                 obj_sampl.insert(field_name_list.at(i), field_val_list.at(i));
       }
       emit sigSendCommand(obj_sampl);
       return true;    // сообщение сформировано и отправлено
    }
    return false;         // сообщение не сформировано  или не отправлено
}
//-----------------------------------------------------------------------
//               Режим работы ПАК АУ
//-----------------------------------------------------------------------
bool ProtocolClient::dceInfoCmdSend()
{
    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);
    QString cmd = "GetDCEInfo";
    QJsonObject obj_sampl;

    if(m_verify_client!=nullptr){
                // получим из протокола шаблон команды для отправки
        obj_sampl = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
               // заполняем основные поля объекта
        QStringList field_name_list;
        QStringList field_val_list;

        field_name_list << "id";
        field_val_list << uuid_str;

        for (int i = 0; i < field_name_list.count(); ++i){
            obj_sampl.insert(field_name_list.at(i), field_val_list.at(i));
         }
        emit sigSendCommand(obj_sampl);
        return true;    // сообщение сформировано и отправлено
     }
     return false;         // сообщение не сформировано  или не отправлено
}
//-----------------------------------------------------------------------
bool ProtocolClient::dirInfoCmdSend()
{
    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);
    QString cmd = "GetRadioDirInfo";
    QJsonObject obj_sampl;

    if(m_verify_client!=nullptr){
                // получим из протокола шаблон команды для отправки
        obj_sampl = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
               // заполняем основные поля объекта
        QStringList field_name_list;
        QStringList field_val_list;

        field_name_list << "id";
        field_val_list << uuid_str;

        for (int i = 0; i < field_name_list.count(); ++i){
            obj_sampl.insert(field_name_list.at(i), field_val_list.at(i));
         }
        emit sigSendCommand(obj_sampl);
        return true;    // сообщение сформировано и отправлено
     }
     return false;         // сообщение не сформировано  или не отправлено
}
//-----------------------------------------------------------------------
bool ProtocolClient::statusCmdSend()
{
    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);
    QString cmd = "Status";
    QJsonObject obj_sampl;

    if(m_verify_client!=nullptr){
                // получим из протокола шаблон команды для отправки
        obj_sampl = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
               // заполняем основные поля объекта
        QStringList field_name_list;
        QStringList field_val_list;

        field_name_list << "id";
        field_val_list << uuid_str;

        for (int i = 0; i < field_name_list.count(); ++i){
            obj_sampl.insert(field_name_list.at(i), field_val_list.at(i));
         }
        emit sigSendCommand(obj_sampl);
        return true;    // сообщение сформировано и отправлено
     }
     return false;         // сообщение не сформировано  или не отправлено
}
//-----------------------------------------------------------------------
quint32 ProtocolClient::enableCmdSend(EnableParamStruct * data_struct)
{
    QJsonObject json_obj;   // приготовили объект для заполнения (пока он пустой)
    QString     cmd = "EnableDCE";
    qint32      dce_id{0};

    if(m_verify_client!=nullptr){
      json_obj = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
      //  if(m_au_obj.status == INIT ){
       fillCommandEnable(json_obj, data_struct);
       emit sigSendCommand(json_obj);
       dce_id  = json_obj.value("DCE_ID").toInt(); // id для УВС
     }
    return dce_id;
}
//-----------------------------------------------------------------------
void ProtocolClient::fillCommandEnable(QJsonObject &json_obj , EnableParamStruct *data_struct)
{     
    EnableParamStruct enable_param = *(EnableParamStruct*)(data_struct);

    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);

    // заполняем основные поля объекта
    QStringList field_name_list;
    QStringList field_val_list;

    QStringList field_name_list1;
    QStringList field_val_list1;


    /*
     * "bandWidth":"",
                "modeSwitch":"",
                "modeExch1":"",                
                "freqOffset":0,
                "frequenctAll":0,
                "freqActive":0,
                "frequencyList":[0,0],
                "frequencyPrimList":[0,0],
                "frequencyAddList":[0,0],
                "frequencyAdaptation":"",
                "constellationAdaptation":"",
                "rateSwitch":"",
                "pcpKey":0,
                "initPcpTime":0,
                "modeFreqPcp":0,
                "inputPort":[0,0],
                "inputProt":"",
                "modeMDM":0,
                "outputAddr":"",
                "outputPort":0,
                "outputProt":"",
                "ppfIndex":[0,0],
                "ppfNumber":0,
                "ppfSwitch":"",
                "radioDirName":"",
                "rcvrAddr":"",
                "rcvrPort":0,
                "rcvrProt":"",
                "samplingRate":0
     */

    // основные ключи
    field_name_list<<"DCE_ID"<<"bandWidth"
                   <<"freqOffset"<<"frequencyAll"
                   <<"freqActive"
                   <<"pcpKey"
                   <<"initPcpTime"
                   <<"modeFreqPcp"
                   <<"rateSwitch"                                 
                   <<"rcvrPort"
                   <<"ppfNumber"
                   <<"outputPort"
                   <<"modeMDM"
                   <<"samplingRate";

    field_val_list<<QString::number((uint32_t)enable_param.dce_id)
                 << QString::number(enable_param.mode_bw)
                 << QString::number(enable_param.fr_offset)
                 << QString::number(enable_param.fr_all)
                 << QString::number(enable_param.fr_active)
                 << QString::number(enable_param.pcp_key)
                 << QString::number(enable_param.initPcpTime)
                 << QString::number(enable_param.modeFreqPcp)
                 << QString::number(enable_param.tm_switch)                                  
                 << QString::number(enable_param.rcvr_port)
                 << QString::number(enable_param.ppf_number)
                 << QString::number(enable_param.output_port)
                 << QString::number(enable_param.mode_var)
                 << QString::number(enable_param.freq_discr);

    for (int i = 0; i < field_name_list.count(); ++i){
         json_obj.insert(field_name_list.at(i), field_val_list.at(i).toInt());
    }

    field_name_list1<<"modeSwitch"<<"modeExch1"<<"bandWidth"<<"rateSwitch"
                    <<"frequencyAdaptation"<<"constellationAdaptation"
                    <<"inputProt"<<"outputProt"<<"outputAddr"
                    <<"ppfSwitch"<<"radioDirName"<<"rcvrAddr"<<"rcvrProt";
    QString modeSw{""};
    if(enable_param.modeSw == NRCH){
        modeSw = "NRCH";
    }
    else if(enable_param.modeSw == PPRCH){
        modeSw = "PPRCH";
    }
    else if(enable_param.modeSw == FRCH){
        modeSw = "FRCH";
    }
    else
        modeSw = "FRCH";

    QString modeEx{""};
    switch (enable_param.modeEx){
      case SIMPLEX : modeEx = "simplex";
            break;
      case HALF_DUPLEX : modeEx = "half_duplex";
            break;
      case DUPLEX :modeEx = "duplex";
            break;
      default : modeEx = "half_duplex";
           break;
      }
    QString adapt{""};
    switch (enable_param.fr_adapt){

      case OFF :adapt = "off";
            break;
      case ON :adapt = "on";
            break;
      default:
           break;
      }
    QString adapt1{""};
    switch (enable_param.cons_adapt){

      case OFF :adapt1 = "off";
            break;
      case ON :adapt1 = "on";
            break;
      default:
           break;
      }

    field_val_list1
                    <<modeSw
                    <<modeEx
                    <<QString::number(enable_param.mode_bw)
                    <<QString::number(enable_param.tm_switch)
                    <<adapt
                    <<adapt1
                    <<enable_param.input_prot
                    <<enable_param.output_prot
                    <<enable_param.output_adr
                    <<enable_param.ppf_switch
                    <<enable_param.radio_dir_name
                    <<enable_param.rcvr_adr
                    <<enable_param.rcvr_prot;


    json_obj.insert("id", uuid_str);
    json_obj.insert("cmd", "EnableDCE");
    for (int i = 0; i < field_name_list1.count(); ++i){
         json_obj.insert(field_name_list1.at(i), field_val_list1.at(i));
    }
    // список портов
    QJsonArray objArr_port;

    for (int i = 0; i < enable_param.input_port_list.count(); ++i)
        objArr_port.append(enable_param.input_port_list.at(i));

    // список индексов частот
    QJsonArray objArr_ppfInd;

    for (int i = 0; i < enable_param.ppf_ind_list.count(); ++i)
        objArr_ppfInd.append(enable_param.ppf_ind_list.at(i));

    // список частот
    QJsonArray objArr_freq;

    for (int i = 0; i < enable_param.freq_list.count(); ++i)
        objArr_freq.append(enable_param.freq_list.at(i));

    // список частот НПП
    QJsonArray objArr_freq_prim;

    for (int i = 0; i < enable_param.freq_list_prim.count(); ++i)
        objArr_freq_prim.append(enable_param.freq_list_prim.at(i));

    // список частот ЗЧ
    QJsonArray objArr_freq_add;

    for (int i = 0; i < enable_param.freq_list_add.count(); ++i)
        objArr_freq_add.append(enable_param.freq_list_add.at(i));

   QJsonArray objArr_param;
    // параметры радиосредств  
   for (int j = 0; j < enable_param.radio_param.count(); ++j){
        QJsonObject objItem1;
        field_name_list.clear();
        QStringList rs;
         rs << enable_param.radio_param.at(j).name;
         rs << QString::number(enable_param.radio_param.at(j).tcp_port);
         rs << QString::number(enable_param.radio_param.at(j).udp_port1);
         rs << QString::number(enable_param.radio_param.at(j).udp_port2);
         rs << enable_param.radio_param.at(j).ipv4;

         objItem1.insert("radioName", rs.first());
         field_name_list<< "radioTcpPort"<<"radioUdpPort1"<<"radioUdpPort2";
         for (int i = 0; i < field_name_list.count(); ++i)
            objItem1.insert(field_name_list.at(i), rs.at(i+1).toInt());
         objItem1.insert("radioIPv4", rs.last());
         objArr_param.append(objItem1);
   }
    json_obj.insert("inputPort",objArr_port);
    json_obj.insert("ppfIndex",objArr_ppfInd);
    json_obj.insert("frequencyList",objArr_freq);
    json_obj.insert("frequencyPrimList",objArr_freq_prim);
    json_obj.insert("frequencyAddList",objArr_freq_add);
    json_obj.insert("radioParams",objArr_param);


}
//-----------------------------------------------------------------------
bool ProtocolClient::disableCmdSend(DisableParamStruct * data_struct)
{
    QJsonObject json_obj;   // приготовили объект для заполнения (пока он пустой)
    QString     cmd = "DisableDCE";

    if(m_verify_client!=nullptr){
      json_obj = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
      //  if(m_au_obj.status == INIT ){
       fillCommandDisable(json_obj, data_struct);
       emit sigSendCommand(json_obj);
       return true;
     }
  return false;
}
//---------------------------------------------------------------------------------
void ProtocolClient::fillCommandDisable(QJsonObject &json_obj , DisableParamStruct *data_struct)
{
    DisableParamStruct disable_param = *(DisableParamStruct*)(data_struct);

    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);

    // заполняем основные поля объекта
    QStringList field_name_list;
    QStringList field_val_list;

    // основные ключи
    field_name_list<<"DCE_ID";

    field_val_list<<QString::number((uint32_t)disable_param.dce_id);

    json_obj.insert("id", uuid_str);
    json_obj.insert("cmd", "DisableDCE");
    for (int i = 0; i < field_name_list.count(); ++i){
         json_obj.insert(field_name_list.at(i), field_val_list.at(i).toInt());
    }
}
//---------------------------------------------------------------------------------
void ProtocolClient::fillCommandSynchro(QJsonObject &json_obj, NetSYNCParamStruct *data_struct)
{
    NetSYNCParamStruct synchro_param = *(NetSYNCParamStruct*)(data_struct);

    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);

    // заполняем основные поля объекта
    QStringList field_name_list;
    QStringList field_val_list;

    // основные ключи
    field_name_list<<"DCE_ID"<<"DCE_ID_OPPN"<<"imitationIns";

    field_val_list<< QString::number((uint32_t)synchro_param.dce_id)
                  << QString::number((uint32_t)synchro_param.dce_id_dst)
                  << QString::number((uint32_t)synchro_param.imitIns);

    // синхро пакет
    QJsonArray objArr_synchro_pack;

    for (int i = 0; i < synchro_param.syncPack.count(); ++i)
        objArr_synchro_pack.append(synchro_param.syncPack.at(i));

    json_obj.insert("id", uuid_str);
    json_obj.insert("cmd", "NetworkSYNC");
    for (int i = 0; i < field_name_list.count(); ++i){
         json_obj.insert(field_name_list.at(i), field_val_list.at(i).toInt());
    }
    json_obj.insert("synchroPack",objArr_synchro_pack);

}
//-----------------------------------------------------------------------
bool ProtocolClient::syncCmdSend(NetSYNCParamStruct * data_struct)
{
    QJsonObject json_obj;   // приготовили объект для заполнения (пока он пустой)
    QString     cmd = "NetworkSYNC";

    if(m_verify_client!=nullptr){
      json_obj = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
      //  if(m_au_obj.status == INIT ){
       fillCommandSynchro(json_obj, data_struct);
       emit sigSendCommand(json_obj);
       return true;
     }
  return false;
}
//-----------------------------------------------------------------------
bool ProtocolClient::connectCmdSend(NetCONNECTParamStruct *data_struct)
{
    QJsonObject json_obj;   // приготовили объект для заполнения (пока он пустой)
    QString     cmd = "NetworkCONNECT";

    if(m_verify_client!=nullptr){
      json_obj = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
      //  if(m_au_obj.status == INIT ){
       fillCommandConnect(json_obj, data_struct);
       emit sigSendCommand(json_obj);
       return true;
     }
  return false;
}
//---------------------------------------------------------------------------------
void ProtocolClient::fillCommandConnect(QJsonObject &json_obj, NetCONNECTParamStruct *data_struct)
{
    NetCONNECTParamStruct connect_param = *(NetCONNECTParamStruct*)(data_struct);

    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);

    // заполняем основные поля объекта
    QStringList field_name_list;
    QStringList field_val_list;

    QStringList field_name_list1;
    QStringList field_val_list1;

    // основные ключи
    field_name_list<<"DCE_ID"<<"DCE_ID_OPPN"<<"imitationIns"<<"factorNet"<<"factorUser"
                  <<"bandWidth"<<"rate"<<"freeParam"<<"modeChangeFreq"<<"rateSwitch";

    field_val_list<< QString::number((uint32_t) connect_param.dceId)
                  << QString::number((uint32_t) connect_param.dceIdCorr)
                  << QString::number((uint32_t) connect_param.imitIns)
                  << QString::number((uint32_t) connect_param.factorNet)
                  << QString::number((uint32_t) connect_param.factorUser)
                  << QString::number((uint32_t)connect_param.modeBW)//ui->cb_regim_bw->currentText()
                  << QString::number((uint32_t)connect_param.rate)//ui->cb_rate->currentText()
                  << QString::number((uint32_t)connect_param.freeParam)//ui->sb_freeParam->text()
                  << QString::number((uint32_t)connect_param.modeChangeFreq)//ui->cb_regim_change_freq->currentText()//QString::number((uint32_t) m_netCONNECT_param.modeChangeFreq)
                  << QString::number((uint32_t)connect_param.modeRateSwitch);//ui->cb_rate_switch->currentText(); //QString::number((uint32_t) m_netCONNECT_param.modeRateSwitch);

    // основные ключи
    field_name_list1<<"modeSwitch"<<"modeExch1"<<"modeExch2"
                   <<"modeAdapt"<<"modeAutoDiscon"<<"radioNetType"
                   <<"haveInfo"<<"priority";

    QString modeSw{""};
    if(connect_param.modeSwitch == NRCH){
        modeSw = "NRCH";
    }
    else if(connect_param.modeSwitch == PPRCH){
        modeSw = "PPRCH";
    }
    else if(connect_param.modeSwitch == FRCH){
        modeSw = "FRCH";
    }
    else
        modeSw = "FRCH";


    QString modeEx{""};
    switch (connect_param.modeEx2){
      case SIMPLEX : modeEx = "simplex";
            break;
      case HALF_DUPLEX : modeEx = "half_duplex";
            break;
      case DUPLEX :modeEx = "duplex";
            break;
      default: modeEx = "duplex";
           break;
      }


    QString modeTrans{""};
    switch (connect_param.modeTrans){
      case SYNC : modeTrans = "sync";
            break;
      case ASYNC : modeTrans = "async";
            break;
      default:   modeTrans = "sync";
           break;
      }

    QString adapt{""};
    switch (connect_param.modeAdapt){

      case OFF : adapt = "off";
            break;
      case ON : adapt = "on";
            break;
      default: adapt = "off";
           break;
      }

    QString autoDisc{""};
    switch (connect_param.autoDiscon){

      case ON : autoDisc = "yes";
            break;
      case OFF : autoDisc = "no";
            break;
      default: autoDisc = "no";
           break;
      }
    QString radioType{""};
    switch (connect_param.radioNetType){

      case POINT : radioType = "point_to_point";
            break;
      case MULTIPOINT : radioType = "point_to_multipoint";
            break;
      default:   radioType = "point_to_point";
           break;
      }

    QString haveInf{""};
    switch (connect_param.haveInfo){

      case NO : haveInf = "no info";
            break;
      case YES : haveInf = "have info";
            break;
      default:  haveInf = "no info";
           break;
      }

    QString priority{""}; // M=0, V, R, C, O
    switch (connect_param.priority){

      case M : priority = "М";
            break;
      case V : priority = "В";
            break;
      case R : priority = "Р";
          break;
      case C : priority = "С";
          break;
      case O : priority = "О";
          break;
      default:   priority = "М";
           break;
      }
    field_val_list1<< modeSw
                  << modeTrans
                  << modeEx
                  << adapt
                  << autoDisc
                  << radioType
                  << haveInf
                  << priority;

    // синхро пакет
    QJsonArray objArr_synchro_pack;

    for (int i = 0; i < connect_param.syncPack.count(); ++i)
        objArr_synchro_pack.append(connect_param.syncPack.at(i));

    json_obj.insert("id", uuid_str);
    json_obj.insert("cmd", "NetworkCONNECT");
    for (int i = 0; i < field_name_list.count(); ++i){
         json_obj.insert(field_name_list.at(i), field_val_list.at(i).toInt());
    }
    for (int i = 0; i < field_name_list1.count(); ++i){
         json_obj.insert(field_name_list1.at(i), field_val_list1.at(i));
    }
    json_obj.insert("synchroPack",objArr_synchro_pack);

}
//---------------------------------------------------------------------------------
void  ProtocolClient::fillCommandDiscon(QJsonObject &json_obj, NetDISCONParamStruct *data_struct)
{
    NetDISCONParamStruct discon_param = *(NetDISCONParamStruct*)(data_struct);

    QUuid uuid = QUuid::createUuid();
    QString uuid_str = uuid.toString(QUuid::WithoutBraces);

    // заполняем основные поля объекта
    QStringList field_name_list;
    QStringList field_val_list;

    // основные ключи
    field_name_list<<"DCE_ID"<<"DCE_ID_OPPN";

    field_val_list<<QString::number((uint32_t)discon_param.dce_id);
    field_val_list<<QString::number((uint32_t)discon_param.dce_id_corr);


    json_obj.insert("id", uuid_str);
    json_obj.insert("cmd", "NetworkDISCON");
    for (int i = 0; i < field_name_list.count(); ++i){
         json_obj.insert(field_name_list.at(i), field_val_list.at(i).toInt());
    }
}
//-----------------------------------------------------------------------
bool ProtocolClient::disconCmdSend(NetDISCONParamStruct * data_struct)
{
    QJsonObject json_obj;   // приготовили объект для заполнения (пока он пустой)
    QString     cmd = "NetworkDISCON";

    if(m_verify_client!=nullptr){
      json_obj = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
      //  if(m_au_obj.status == INIT ){
       fillCommandDiscon(json_obj, data_struct);
       emit sigSendCommand(json_obj);
       return true;
     }
  return false;
}
//-----------------------------------------------------------------------
//                 Режим работы ПАК МАС
//-----------------------------------------------------------------------
bool ProtocolClient::exchCmdSend(ExchangeParamStruct * data_struct)
{
    QJsonObject json_obj;   // приготовили объект для заполнения (пока он пустой)
    QString     cmd = "ExchangeDCE";

    if(m_verify_client!=nullptr){
      json_obj = m_verify_client->getObjectSampl(cmd, SUFFIX_Q);
      //  if(m_au_obj.status == INIT ){
       fillCommandExchange(json_obj, data_struct);
       ++m_msg_queue;
       emit sigSendCommand(json_obj);       
       return true;
     }
  return false;
}
//-----------------------------------------------------------------------
void ProtocolClient::fillCommandExchange(QJsonObject &json_obj, ExchangeParamStruct *data_struct)
{
    // поле data в структуре - находится в закодированном виде
    ExchangeParamStruct exchange_param = *(ExchangeParamStruct*)(data_struct);

   // QUuid uuid = QUuid::createUuid();
   // QString uuid_str = uuid.toString(QUuid::WithoutBraces);

    // заполняем основные поля объекта
    QStringList field_name_list;
    QStringList field_val_list;

    // основные ключи
    field_name_list<<"DCE_ID"<</*"DCE_ID_SRC" <<*/"DCE_ID_DST" <<"MSG_ID";    

    field_val_list<<QString::number((uint32_t)exchange_param.dce_id);
    field_val_list<<QString::number((uint32_t)exchange_param.dce_id_corr);
    field_val_list<<QString::number((uint32_t)exchange_param.msg_id);

   // json_obj.insert("id", uuid_str);  // В команде нет поля id
    json_obj.insert("cmd", "ExchangeDCE");

    // в поле data - упаковываем данные требуемой четности
    if(m_mode_exch != INFO_EXCH){   // режим теста
       if(m_parity_count){  // пакет четный
           json_obj.insert("data", exchange_param.data_parity.data());
         // json_obj.insert("data", QByteArray::fromBase64(exchange_param.data_parity.data()));     //.toBase64().data()); //помещаем данные в пакет в закодированном виде
          m_parity_count = false;
        }
        else {   // пакет нечетный
           json_obj.insert("data", exchange_param.data_noparity.data());
           // json_obj.insert("data", QByteArray::fromBase64(exchange_param.data_noparity.data()));
            m_parity_count = true;
         }
    }
    else{    // режим инфообмена
        json_obj.insert("data", exchange_param.data_general.data());
       // json_obj.insert("data", QByteArray::fromBase64(exchange_param.data_general.data()));
    }

   // ba_.append(QByteArray::fromBase64(bb.data()));// декодируем принятые байты и помещаем в байтовый массив
    switch (exchange_param.type){
    case 0: json_obj.insert("type", "Test");
        break;
    case 1: json_obj.insert("type", "Raw");
        break;
    case 2: json_obj.insert("type", "File");
        break;
    case 3: json_obj.insert("type", "Chat");
        break;
    default:json_obj.insert("type", "Raw");;
        break;
    }

    for (int i = 0; i < field_name_list.count(); ++i){
         json_obj.insert(field_name_list.at(i), field_val_list.at(i).toInt());
    }
   int bits_pass = decodingGettingObject(json_obj);
   emit sigPassBit(bits_pass);
 }

//-----------------------------------------------------------------------
//     Анализ пришедших команд и ответных сообщений
//-----------------------------------------------------------------------

// анализируем сообщение пришедшее от ПАК УВС (сервера)
void ProtocolClient::slotAnalyseMsgFromServer(MsgDescrS &msg)
{
         // если json последовательности ошибок нет ->
    if(checkPreAnsw(msg.obj_msg_in)){  // ** проверяем сообщение не является ли оно предварительным ответом
                   // пришедщее сообщение - предварительное !
                   // ни чего не делаем, ждем след. сообщения
    }
    else if(checkPositiveAnsw(msg.obj_msg_in)){
       //emit sigPanelOnDTE(obj); // отправляем сигнал что нужно включить панель на главной форме

     }
    else if(checkStartAnsw(msg.obj_msg_in)){  // *** проверяем сообщение не является ли ответым сообщением со статусом START
        //emit sigPanelOnDTE(obj); // отправляем сигнал что нужно включить панель на главной форме
    }
    else if(checkRecvAnsw(msg.obj_msg_in)){  // *** проверяем сообщение не является ли ответым сообщением со статусом RECV
       if( verifyImitsInst(msg.obj_msg_in) ){ // проверка иммитовставки на стороне ПАК АУ
         QJsonObject jObj_ = prepareJsonComVerify(msg.obj_msg_in); // подготовка  ответа CЕРВЕРУ со статусом VERIFY
         msg.obj_msg_out = jObj_;
         slotSendCommandDTE(jObj_);  // отправляем сигнал с verify
        }
        else
           QMessageBox::critical(0, "Сообщение","Иммитовставка в сообщении не прошла проверку." ,"");
    }
    else if (checkNetBusyAnsw(msg.obj_msg_in)){
        //emit sigPanelOnDTE(obj); // отправляем сигнал что нужно включить панель на главной форме
    }
    else if (checkUserBusyAnsw(msg.obj_msg_in)){
       // emit sigPanelOnDTE(obj); // отправляем сигнал что нужно включить панель на главной форме
    }
    else if (checkEndAnsw(msg.obj_msg_in)){
      //  emit sigPanelOnDTE(obj); // отправляем сигнал что нужно включить панель на главной форме
    }
    else if (checkEnqueuedAnsw(msg.obj_msg_in)){  // получили от сервера команду с полем status = enqueued
      //  emit sigPanelOnDTE(obj);
               // пока ничего не делаем !!
    }
    else if (checkDataAnsw(msg.obj_msg_in)){ // получили от сервера команду с полем data
        QJsonObject jObj_ = prepareJsonComReceiveData(msg.obj_msg_in); // подготовка  ответа CЕРВЕРУ со статусом RECEIVED
        slotSendCommandDTE(jObj_);  // отправляем сигнал с receied серверу
        //emit sigPanelOnDTE(obj);
    }
    // else if (checkDisconAnsw(jObj_in)){
    //     emit sigPanelOnDTE(jObj_in); // отправляем сигнал что нужно включить панель на главной форме
    // }
}
//-------------------------------------------------------------------------------------
 // функция проверки иммитовставки команды пришедшей с ПАК УВС
int ProtocolClient::verifyImitsInst(const QJsonObject  & obj)
{
    Q_UNUSED(obj);
   //static int res{-1}; имитация что проверка пройдена со второго раза отправленной команды
    static int res{0};
    ++res ;
    return res;
}
//----------------------------------------------------------------------------------------------
// подготавливаем ответное сообщение на команду со статусом verify
QJsonObject ProtocolClient::prepareJsonComVerify(QJsonObject &jObj_in)
{
    QJsonObject jObj_out;

    QString name_com = jObj_in.value("cmd").toString(); // выделим из шаблона имя
    QString uuid_com = jObj_in.value("id").toString();  // выделим из шаблона id

    // получим шаблон для ответного сообщения из m_com_list (по данным jObj_in.cmd);
    jObj_out = (m_verify_client->getCmdList()).value(name_com + SUFFIX_A).toJsonObject();
    jObj_out.insert("id", uuid_com);             // заполним поле id (по данным jObj_in.id)
    m_verify_client->processObject(jObj_in, jObj_out, FILL_SAME); // заполним другие совпадающие поля образца(по данным jObj_in),
                                                 //если что поправим их из модуля MainWind
    jObj_out.insert("status", "verify");  // здесь должна быть проверка имитовставке
    return jObj_out;     // возвращаем частично заполненный шаблон ответной команды (совпадающими полями из входящей команды)
}
//----------------------------------------------------------------------------------------------
// подготавливаем ответное сообщение на команду ExchangeDCE
QJsonObject ProtocolClient::prepareJsonComReceiveData(QJsonObject &jObj_in)
{
    QJsonObject jObj_out;

    QString name_com =  "ReplyExchangeDCE"; //jObj_in.value("cmd").toString(); // выделим из шаблона имя
  //  QString uuid_com = jObj_in.value("id").toString();  // выделим из шаблона id

    // получим шаблон для ответного сообщения из m_com_list (по данным jObj_in.cmd);
    jObj_out = (m_verify_client->getCmdList()).value(name_com + SUFFIX_A).toJsonObject();
   // jObj_out.insert("id", uuid_com);             // заполним поле id (по данным jObj_in.id)
   // processObject(jObj_in, jObj_out, FILL_SAME); // заполним другие совпадающие поля образца(по данным jObj_in),
                                                 //если что поправим их из модуля MainWind
    jObj_out.insert("status", "recv");
    jObj_out.insert("type",jObj_in.value("type").toString());
    jObj_out.insert("MSG_ID",jObj_in.value("MSG_ID").toInt());

    if(jObj_in.contains("DCE_ID_OPPN")){
        jObj_out.insert("DCE_ID",jObj_in.value("DCE_ID").toInt());
        jObj_out.insert("DCE_ID_OPPN",jObj_in.value("DCE_ID_OPPN").toInt());
        if(jObj_out.contains("DCE_ID_DST"))
          jObj_out.remove("DCE_ID_DST");
        if(jObj_out.contains("DCE_ID_SRC"))
          jObj_out.remove("DCE_ID_SRC");
    }
    else{
       if(jObj_in.contains("DCE_ID_SRC")){
          jObj_out.insert("DCE_ID",jObj_in.value("DCE_ID").toInt());
          jObj_out.insert("DCE_ID_SRC",jObj_in.value("DCE_ID_SRC").toInt());
          if(jObj_out.contains("DCE_ID_DST"))
            jObj_out.remove("DCE_ID_DST");
       }
    }
    return jObj_out;     // возвращаем частично заполненный шаблон ответной команды (совпадающими полями из входящей команды)
}
//-------------------------------------------------------------------------------------------
// проверка принятой команды на то что она является предварительной
 bool ProtocolClient::checkPreAnsw(QJsonObject & jsonObj)
 {
     bool res = false;
     if(jsonObj.contains("param") && jsonObj.value("param").isObject()){
         QJsonObject objItem = jsonObj.value("param").toObject();
         if (objItem.contains("StringAnswer") &&
            (objItem.value("StringAnswer")=="Ok")&&
            (objItem.value("StringAnswer").isString()))
         {
             res = true;
         }
     }
     return res;
 }
 //-----------------------------------------------------------------
 // проверка пришедшей команды  является ли она ответом на раненее отправленную команду со статусом "recv"
 bool ProtocolClient::checkRecvAnsw(QJsonObject & jsonObj)
 {
     bool res = false;
     if(jsonObj.contains("status") && jsonObj.value("status").isString()){
         QJsonValue objVal = jsonObj.value("status");
         if (objVal == "recv" )
         {
             res = true;
         }
     }
     return res;
 }

 //----------------------------------------------------------------
 // проверка пришедшей команды  является ли она ответом на раненее отправленную команду
 bool ProtocolClient::checkPositiveAnsw(QJsonObject & jsonObj)
 {
     bool res = false;
     QString val{""};

     if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString() && !jsonObj.contains("status")){
         QJsonValue objVal = jsonObj.value("cmd");
        // if ((objVal != "Error")&&(objVal != "ExchangeDCE"))
         if (objVal != "ExchangeDCE")
         {
             if(objVal == "Ping")
                 emit sigPanelOnDTE(PING,val);
             else if (objVal == "Error")
                 emit sigPanelOnDTE(ERROR,val);
             else if (objVal == "GetDCEInfo")
                 emit sigPanelOnDTE(DCE_INFO,val);
             else if (objVal == "GetRadioDirInfo")
                 emit sigPanelOnDTE(DIR_INFO,val);
             else if (objVal == "Version")
                 emit sigPanelOnDTE(VER,val);
             else if (objVal == "Status")
                 emit sigPanelOnDTE(STATUS,val);
             else if (objVal == "DisableDCE" )
                 emit sigPanelOnDTE(DISBL,val);
             else if (objVal == "EnableDCE" )
                 emit sigPanelOnDTE(ENBL,val);
             res = true;
         }         
     }
     return res;
 }
 //----------------------------------------------------------------
 // проверка пришедшей команды  является ли она ответом на раненее отправленную команду со статусом "recv"
 bool ProtocolClient::checkStartAnsw(QJsonObject & jsonObj)
 {
     bool res = false;
     QString val{""};

     if(jsonObj.contains("status") && jsonObj.value("status").isString()){
         QJsonValue objVal = jsonObj.value("status");
         if (objVal == "start" )
         {
             if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
                 QJsonValue objVal = jsonObj.value("cmd");
                 if (objVal == "NetworkCONNECT" )
                 {
                   emit sigPanelOnDTE(CONNECT_START,val);
                   res = true;
                 }
             }
         }
     }
     return res;
 }
 //----------------------------------------------------------------
 // проверка пришедшей команды  является ли она ответом на раненее отправленную команду со статусом "recv"
 bool ProtocolClient::checkNetBusyAnsw(QJsonObject & jsonObj)
 {
     bool res = false;
     QString val{""};

     if(jsonObj.contains("status") && jsonObj.value("status").isString()){
         QJsonValue objVal = jsonObj.value("status");
         if (objVal == "net busy" )
         {
             if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
                 QJsonValue objVal = jsonObj.value("cmd");
                 if (objVal == "NetworkCONNECT" )
                 {
                     emit sigPanelOnDTE(NET_BUSY,val);
                     res = true;
                 }
             }
         }
     }
     return res;
 } 
 //----------------------------------------------------------------
 // проверка пришедшей команды  является ли она ответом на раненее отправленную команду со статусом "recv"
 bool ProtocolClient::checkUserBusyAnsw(QJsonObject & jsonObj)
 {
     bool res = false;
     QString val{""};

     if(jsonObj.contains("status") && jsonObj.value("status").isString()){
         QJsonValue objVal = jsonObj.value("status");
         if (objVal == "user busy" )
         {
             if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
                 QJsonValue objVal = jsonObj.value("cmd");
                 if (objVal == "NetworkCONNECT" )
                  {
                    emit sigPanelOnDTE(USER_BUSY,val);
                    res = true;
                 }
             }
         }
     }
     return res;
 } 
 //-----------------------------------------------------------------
 // проверка пришедшей команды  является ли она ответом на раненее отправленную команду со статусом "recv"
 bool ProtocolClient::checkEndAnsw(QJsonObject & jsonObj)
 {
     bool res = false;
     QString val{""};

     if(jsonObj.contains("status") && jsonObj.value("status").isString()){
         QJsonValue objVal = jsonObj.value("status");
         if (objVal == "end" )
         {
             if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
                 QJsonValue objVal = jsonObj.value("cmd");
                 if (objVal == "NetworkSYNC" )
                 {
                    res = true;
                    emit sigPanelOnDTE(SYNCH, val);
                 }
                 else if (objVal == "NetworkCONNECT" )
                 {
                   QString chanel = getChannel(jsonObj);
                   res = true;
                   emit sigPanelOnDTE(CONNECT_END,chanel);
                 }
                 else if (objVal == "NetworkDISCON" )
                 {
                    res = true;
                    emit sigPanelOnDTE(DISCON,val);
                 }
                 else if (objVal == "ReplyExchangeDCE" ){
                     if(jsonObj.contains("msgsInQueue")){
                        int m_quer = jsonObj.value("msgsInQueue").toInt();
                          if(m_quer <= m_msg_queue){
                             m_msg_queue = jsonObj.value("msgsInQueue").toInt();
                             emit sigMsgInQue(m_msg_queue);
                           }
                      }
                     if(jsonObj.contains("bytesInQueue")){
                        m_byte_queue = jsonObj.value("bytesInQueue").toInt();
                        emit sigBytesInQue(m_byte_queue);
                      }
                      res = true;
                      emit sigPanelOnDTE(REPLY_END,val);
                 }
             }
         }
     }
     return res;
 }  
//--------------------------------------------------------------------------
 // проверка пришедшей команды  является ли она ответом на раненее отправленную команду со статусом "enqueued"
 bool ProtocolClient::checkEnqueuedAnsw(QJsonObject & jsonObj)
 {
     bool res = false;
     QString val{""};

     if(jsonObj.contains("status") && jsonObj.value("status").isString()){
         QJsonValue objVal = jsonObj.value("status");
         if (objVal == "enqueued" )
         {
             if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
                 QJsonValue objVal = jsonObj.value("cmd");
                 if (objVal == "ReplyExchangeDCE" ){
                     if(jsonObj.contains("msgsInQueue")){
                         int m_quer = jsonObj.value("msgsInQueue").toInt();
                         if(m_quer >= m_msg_queue){
                            m_msg_queue = jsonObj.value("msgsInQueue").toInt();
                           emit  sigMsgInQue(m_msg_queue);
                         }

                      }
                     if(jsonObj.contains("bytesInQueue")){
                         emit sigBytesInQue(jsonObj.value("bytesInQueue").toInt());
                          m_byte_queue = jsonObj.value("bytesInQueue").toInt();
                      }
                      res = true;
                      emit sigPanelOnDTE(REPLY_END,val);
                 }
            }

             res = true;
         }
     }
     return res;
 }

 //--------------------------------------------------------------------------
  // пока предполагаем что команда networkdiscon обрабатывается так же как и все сетевые команды (с проверкой иммитовстваки)
  // проверка пришедшей команды  является ли она ответом на раненее отправленную команду со статусом "recv"
 /*  bool ProtocolDTE::checkDisconAnsw(QJsonObject & jsonObj)
  {
     bool res = false;
     if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
        if(jsonObj.value("cmd") == "NetworkDISCON"){
          if(jsonObj.contains("status") && jsonObj.value("status").isString()){
             QJsonValue objVal = jsonObj.value("status");
             if (objVal == "" )
               {
                 res = true;
                }
           }
        }
     }
     return res;
  }
 */
  //-----------------------------------------------------------------
  // определение номеров каналлов приема/передачи из пришедшей ответной команды
  // NetworkCONNECT из полей : freqRX, freqT
  // Данная информация необходима для отображения на индикаторе состояния
  QString ProtocolClient::getChannel(QJsonObject &jsonObj)
  {
      QString res{""};
      QJsonValue objVal1;
      if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
          QJsonValue objVal = jsonObj.value("cmd");
          if (objVal == "NetworkCONNECT" ){
            if(jsonObj.contains("status") && jsonObj.value("status").isString()){
                  QJsonValue objVal = jsonObj.value("status");
                 if (objVal == "end" ){
                    if(jsonObj.contains("freqRX") && jsonObj.value("freqRX").isDouble()){
                       int rx = jsonObj.value("freqRX").toInt();
                        res = " f_RX=" + QString::number(rx);
                     }
                    if(jsonObj.contains("freqTX") && jsonObj.value("freqTX").isDouble()){
                        int tx = jsonObj.value("freqTX").toInt();
                        res = res +", f_TX="+ QString::number(tx);
                     }
                  }
              }
           }
       }
      return res;
  }

//---------------------------------------------------------------------------
 // по сигналу из WidgDTE о задании режима работы ПАК МАС
 void ProtocolClient::slotExchMode(int m)//ExchangeModeEnum mode)
 {
     if(m == 0)
         m_mode_exch = TEST_EXCH;
     else
         m_mode_exch = INFO_EXCH;
 }
 //--------------------------------------------------------------------------
   // проверка пришедшей команды  является ли она командой Exchange c полем DATA
 bool ProtocolClient::checkDataAnsw(QJsonObject & jsonObj)
   {
       bool res = false;

       if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
           QJsonValue objVal = jsonObj.value("cmd");
           if (objVal == "ExchangeDCE" )
           {
               if(jsonObj.contains("data")){
                 QJsonValue objVal = jsonObj.value("data");

                 QBitArray  bt;
                 QByteArray bb, ba_;
                 bb.append(objVal.toString().toUtf8());       // зашифрованные данные в байтах помещаем в байтовый массив                 
                 ba_.append(QByteArray::fromBase64(bb.data()));// декодируем принятые байты и помещаем в байтовый массив
                 m_info_exch.byte_get = m_info_exch.byte_get + ba_.count();  // подсчет принятых байт
                 emit sigGetBit(m_info_exch.byte_get);

                 //  подсчет ошибок на байт
                 if(m_mode_exch == TEST_EXCH){
                    m_info_exch.bit_err = getErrorInObjParityNew(ba_);  // поврежденные байты
                    emit sigErrBit(m_info_exch.bit_err);
                    }                 
               }
               res = true;
            }
        }
       return res;
   }
 //--------------------------------------------------------------------
 int ProtocolClient::getErrorInObjParityNew(QByteArray & ba_study)
 {

     // Начало пакета! ищем в принятой послед. признак пакета по ампл.
     for (int i = 0; i < ba_study.count(); ++i){
         // Старт работы теста
         if (ba_study.at(i) == m_parity_A && m_test_pack.start == 2){ // если нашлось начало пакета A1
             // формируем реестр параметров изучаемой после. (принятой) в случае чет. послед.
             m_test_pack.ampl = m_parity_A;
             m_test_pack.len_max = m_parity_L;// общая ожидаемая длинна пакета
             ++m_test_pack.len;                // текущая (уже прочитанная длинна послед.)
             m_test_pack.parity = 1;           // послед. четная
             m_test_pack.start = 1;            // далее пакет продолжается
         }
         else if (ba_study.at(i) == m_noparity_A && m_test_pack.start == 2){ // начало пакета A2
             // формируем реестр параметров изучаемой после. (принятой) в случае нечет. послед.
             m_test_pack.ampl = m_noparity_A;
             m_test_pack.len_max = m_noparity_L; // общая ожидаемая длинна пакета
             ++m_test_pack.len;            // текущая (уже прочитанная длинна послед.)
             m_test_pack.parity = 0;       // флаг нечет послед.
             m_test_pack.start = 1;         // далее пакет продолжается
         }

          // Продолжение пакета - любого
         else if (ba_study.at(i) == m_test_pack.ampl && m_test_pack.start == 1 /*&& m_test_pack.end == 02*/){
             ++m_test_pack.len;

         }

         // Окончание пакета и начало нового или ошибка в пакете
         else if (ba_study.at(i) != m_test_pack.ampl && m_test_pack.start == 1 /*&& m_test_pack.end == 2*/){

             // ошибка внутри пакета
               if((ba_study.at(i) != m_parity_A) &&     // байт != А1 (амплитуде четного сигн.)
                  (ba_study.at(i) != m_noparity_A) &&   // байт != А2 (ампилитуде не четного сигн.)
                  (ba_study.at(i)!= 0)) {                // байт != 0
                   QByteArray ba_;
                   ba_.append(ba_study.at(i));   // форм. послед. из одного ошиб. байта
                   //++m_test_pack.err = m_test_pack.err + getBitErr(ba_);            // оценим битовую ошибку
                   m_test_pack.err += getBitErr(ba_);
                 }

                // окончание предыдушего и начало нового пакета
               else if((ba_study.at(i) == m_parity_A) || (ba_study.at(i) == m_noparity_A)){

                    if (m_test_pack.len <= m_test_pack.len_max){
                        if(m_test_pack.len_max - m_test_pack.len) // если текущая длинна послед. не соотв. ожидаемой длинне- это ошибка
                           m_test_pack.err = m_test_pack.err + (m_test_pack.len_max - m_test_pack.len);
                        m_test_pack.len = 0;
                        ++m_test_pack.len;

                        m_test_pack.parity_pre = m_test_pack.parity;
                        m_test_pack.parity = (m_test_pack.parity + 1)%2;
                        m_test_pack.start = 1;                        
                    }

                    else if (m_test_pack.len > m_test_pack.len_max){  // имеется признак пропуска пакета
                                          // ошибка составляет кол-во байт в пакете * 8, т.е. представлена в битах
                        if(m_test_pack.len_max - m_test_pack.len)
                           m_test_pack.err = m_test_pack.err + 8*(m_test_pack.len - m_test_pack.len_max);
                        m_test_pack.len = 0;
                        ++m_test_pack.len;

                        m_test_pack.parity_pre = m_test_pack.parity;
                        m_test_pack.parity = (m_test_pack.parity + 1)%2;
                        m_test_pack.start = 1;                       
                    }

                    m_test_pack.ampl = ba_study.at(i);
                    if(ba_study.at(i) == m_parity_A)
                       m_test_pack.len_max = m_parity_L;
                    else
                      m_test_pack.len_max = m_noparity_L;
               }
           }
     }

    return m_test_pack.err;
 }
 //-----------------------------------------------------------------
 // подсчет min битовой ошибки
 int ProtocolClient::getBitErr(QByteArray & ba_study)
 {
     // выясним какому ампитудному значению (четному или нечетному) относится наш ошибочный байт
     // (сравниваем с обоими и выбираем min по кол-ву бит),

     QByteArray ba_sample1{};  // эталонная четная послед
     QByteArray ba_sample2{};  // эталонная нечетная послед
     int err1{0};// число битовых ошибок в изучаемой послед
     int err2{0};// число битовых ошибок в изучаемой послед.

     ba_sample1.append(m_parity_A);
     ba_sample2.append(m_noparity_A);

     if(ba_sample1.count()>0 && ba_study.count()>0){
        QBitArray bt_sample = byteToBit(ba_sample1);        // эталонная битовая последовательность
        QBitArray bt_study = byteToBit(ba_study);            // изучаемая битовая последовательность

        for (int i  = 0; i < bt_study.count(); ++i){ // приступим к анализу последовательности
             if(bt_sample.at(i) != bt_study.at(i)){   // если позиционные биты в сравниваемых последоват не соврадают
              ++err1;                                // тогда ошибка
              }
         }
     }

     if(ba_sample2.count()>0 && ba_study.count()>0){
        QBitArray bt_sample = byteToBit(ba_sample2);    // эталонная битовая последовательность
        QBitArray bt_study = byteToBit(ba_study);           // изучаемая битовая последовательность

        for (int i  = 0; i < bt_study.count(); ++i){ // приступим к анализу последовательности
             if(bt_sample.at(i) != bt_study.at(i)){   // если позиционные биты в сравниваемых последоват не соврадают
              ++err2;                                // тогда ошибка
              }
         }
     }
     // возвращаем min битовую ошибку
     if(err1 >= err2 )
       return err2;
     else
       return err1;

 }
 //-----------------------------------------------------------------
 QBitArray ProtocolClient::byteToBit(QByteArray & ba)
 {
     QBitArray bt (ba.count() * 8);   // задаем размер битового массива

     for (int i = 0; i < ba.count(); ++i){ // цикл по всем байтам
         for(int b = 0; b < 8; ++b){       // цикл по всем битайм в тек. байте
             bt.setBit(i*8+b, ba.at(i) & (1 << b));
         }
     }
     return bt;
 }

//--------------------------------------------------------------------------------------
int ProtocolClient::decodingDataLength(QJsonValue val)
 {
     int len;
     QByteArray bb, ba;

     bb.append(val.toString().toUtf8());       // зашифрованные данные в байтах помещаем в байтовый массив
     ba.append(QByteArray::fromBase64(bb.data()));// декодируем принятые байты и помещаем в байтовый массив
     len = ba.length();
     return len;
 }
 //-------------------------------------------------------------------------------------
 int ProtocolClient::decodingGettingObject(QJsonObject json_in)
 {
     QJsonValue objVal;
     int len{0};
     if(json_in.contains("data")){
       objVal = json_in.value("data");
       len = decodingDataLength(objVal);
      }
     return len;
 }
 //--------------------------------------------------------------------------------------
