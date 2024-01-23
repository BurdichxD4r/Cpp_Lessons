#include "protocolCHN.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QtXml>

//-----------------------------------------------------------------------------------------------
ProtocolCHN::ProtocolCHN(NetS & net_adrs, TypeMsgEnum type_msg, bool format_msg, int type):
              VerificationBase(type_msg, format_msg), m_net_adr(net_adrs)

{
    if(type == 1 ){   // часть клиента

       m_connectDTE = new TransportClient(m_net_adr, type_msg);
       connect(this, &ProtocolCHN::sigStopServOrDiscon, m_connectDTE, &TransportClient::slotDisconnect);
       connect(m_connectDTE, &TransportBase::sigOkError, this, &VerificationBase::slotGetOkError);
       connect(m_connectDTE, &TransportBase::sigMsg, this, &ProtocolCHN::slotAnalyseMsgServer);
       connect(m_connectDTE, &TransportBase::sigMsgToJourn, this, &VerificationBase::slotGetMsgToJourn);
    }
    if(type == 2){    // часть сервера

       m_connectDCE = new TransportServ(m_net_adr, type_msg);
       connect(this, &ProtocolCHN::sigStopServOrDiscon, m_connectDCE, &TransportServ::slotStopServer);
       connect(m_connectDCE, &TransportServ::sigClientListDCE, this, &ProtocolCHN::slotGetClientList);
       connect(m_connectDCE, &TransportBase::sigOkError, this, &VerificationBase::slotGetOkError);
       connect(m_connectDCE, &TransportBase::sigMsg, this, &ProtocolCHN::slotAnalyseMsgClient);
       connect(m_connectDCE, &TransportBase::sigMsgToJourn, this, &VerificationBase::slotGetMsgToJourn);
    } 
 }
//-------------------------------------------------------------------------------------------------
ProtocolCHN::~ProtocolCHN()
{
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//                                   Клиентская часть (УВС)
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// обработчик сигнала из MW УВС об отключении клиента
 void ProtocolCHN::slotDisconClient()
 {
     emit sigStopServOrDiscon();  // сигнал в connectorDTE об отключении клиента
 }
 //-----------------------------------------------------------------------------------------------
 // обработка сигнала из УВС MW по отправке в коннекторDTE сообщения для сервера обмена
 // отправка JsonObj Серверу обмена
 void ProtocolCHN::slotSendMsgClient(const QJsonObject & jsonObj)
 {
     QByteArray pack_to_send;
     pack_to_send = prepareMsg(jsonObj); // если сообщение нужно отправить в xml формате -
                                         // преобразование json->xml происходит внутри preparePacket(jsonObj)
     m_connectDTE->sendToServer(pack_to_send);

 }
 //----------------------------------------------------------------------------------------------
 //  сообщения для Сервера обмена от   клиента(УВС)

 //-------------------------------------------------------------------
 // анализ принятой из сети команды,
 void ProtocolCHN::slotAnalyseMsgClient(int k,  MsgDescrS & msg , QTime tm)
 {
    Q_UNUSED(tm);

   if(k == 0)
     analyseMsgFromServ(msg.ba_msg_in);
 }
 //--------------------------------------------------------------------------------------------
 void ProtocolCHN::analyseMsgFromClient(const QByteArray & jsonPack) // not From - For
 {   
    QJsonDocument   doc_in;         // json док. для загрузке в него принятой послед.
    QJsonParseError parseErr_in;    // ошибки возникающие при парсинге принятой послед. в json док.
    QJsonObject     jObj_in;        // json объект созданый из принятой по сети последовательности
    QString         err;

    m_error = "";   // вновь пришедший пакет, поле ошибки пусто т.к. пакет только начинает обрабатываться

    if(jsonPack.count() > 0){  // принятое сообщение не пустое
            // проверим принятое сообщение на ошибки JSON
       doc_in = QJsonDocument::fromJson(jsonPack, & parseErr_in);
            // если есть ошибки JSON - выдаем сообщение
       if (parseErr_in.error != QJsonParseError::NoError){
            err = parseErr_in.errorString();//"JSON-ошибка";
            QMessageBox::critical(0, "Сообщение","В сообщении JSON ошибка! ("+QString(jsonPack)+")" ,"");
            m_error = "В сообщении JSON ошибка! ("+err+")";
        }
           // если json последовательности ошибок нет ->
       else if(doc_in.isObject()){
            jObj_in = doc_in.object(); // формируем из сообщения json объект
            emit sigReturnMsgObjToMW(jObj_in);
       }  // в переданной последовательности, json ошибок - нет

    }   // принятое сообщение не пустое
 }

 //-----------------------------------------------------------------------------------------------------
   // проверка пришедшей команды TermType
bool ProtocolCHN::checkTermTypeFromClient(QJsonObject &jsonObj)
   {
       bool res = false;

       if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
           QJsonValue objVal = jsonObj.value("cmd");
           if (objVal == "TermType" )
           {
               res = true;
           }
       }
       return res;
   }
 //-----------------------------------------------------------------------------------------------------
 //                                 Серверная часть (Сервер обмена)
 //-----------------------------------------------------------------------------------------------------
 //-----------------------------------------------------------------------------------------------------
 // обработчик сигнала  об остановки сервера из MW сервера обмена
 void ProtocolCHN::slotStopServer()
 {
     emit sigStopServOrDiscon();   // сигнал в connector DCE об остановке сервера
 }
 //------------------------------------------------------------------
 // обработчик сигнала из connectDCE со списком подключенных клиентов
 void ProtocolCHN::slotGetClientList(QVector <NetConnectS> client_vec)
 {
    // сигнал в MW со списком подключенных к серверу клиентов
    emit sigClientListToMWind (client_vec);
 }
//-------------------------------------------------------------------------------------------------------
// сообщения для Клиента  (УВС) от Сервера обмена
//-------------------------------------------------------------------------------------------------------

// обработчик сигнала из MW Сервера обмена
// анализ принятой из сети команды, если команда пришла на сервер
// будет выдан сигнал об отправке ответного сообщ.- sendAnswer)
//
 void ProtocolCHN::slotAnalyseMsgServer(int k,  MsgDescrS & msg , QTime tm)
 {
     Q_UNUSED(tm);
     if(k == 0)
       analyseMsgFromClient(msg.ba_msg_in);
 }
//-------------------------------------------------------------------------------------------------------
void ProtocolCHN::analyseMsgFromServ(const QByteArray & jsonPack)
{
    QJsonDocument   doc_in;         // json док. для загрузке в него принятой послед.
    QJsonParseError parseErr_in;    // ошибки возникающие при парсинге принятой послед. в json док.
    QJsonObject     jObj_in;        // json объект созданый из принятой по сети последовательности
    QString         err;

    if(jsonPack.count() > 0){  // принятое сообщение не пустое
        doc_in = QJsonDocument::fromJson(jsonPack, & parseErr_in);
            // если есть ошибки JSON - выдаем сообщение
        if (parseErr_in.error != QJsonParseError::NoError){
            err = parseErr_in.errorString();//"JSON-ошибка";
            QMessageBox::critical(0, "Сообщение","В сообщении JSON ошибка! ("+QString(jsonPack)+")" ,"");
            m_error = "В сообщении JSON ошибка! ("+err+")";
        }
        // если json последовательности ошибок нет ->
        else if(doc_in.isObject()){
            jObj_in = doc_in.object(); // формируем из сообщения json объект

            if(checkTermTypeFromClient(jObj_in)){  // проверка полученной команды на TermType
                QString term_name = jObj_in.value("nameTerm").toString();
                m_connectDCE->namedClient(m_connectDCE->getSocket()->socketDescriptor(), term_name);
             }// именуем пока еще безымянного клиента, типом содержащимся в ком. TermType

            // здесь можно отправить  сигнал в MW Сервера обмена c данными о том какие номера УВС обмениваются командами
            else{
                if(checkExchDCEFromClient(jObj_in)){  // проверка полученной команды на ExchangeDCE
                    damagePerSeconBitInByte(jObj_in);
                 }
                changeDCEid(jObj_in);      // меняем местами DCE and OPPN_DCE
                sendMsgOppn(jObj_in);      // входящее сообщение отправляем оппонирующему клиенту
            }
        }
    }
}
//--------------------------------------------------------------------------------------------
bool ProtocolCHN::checkExchDCEFromClient(QJsonObject & jsonObj)
{
    bool res = false;

    if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString() && jsonObj.contains("data")){
        QJsonValue objVal = jsonObj.value("cmd");
        QJsonValue objVal_data = jsonObj.value("data");
        QJsonValue objVal_type = jsonObj.value("type");
        QString str_data = objVal_data.toString();
        if (objVal == "ExchangeDCE" && objVal_type == "Test" ) // проверка пришедшего пакета на принадлежность его к режиму тестирования
        {
            res = true;
        }
    }
    return res;
}
//------------------------------------------------------------------------------------------
void ProtocolCHN::slotParamErrorSet(QString text)
{
    if(text!="0"){
       m_param_err = pow(10,text.toInt());
    }
    else
        m_param_err = 0;
}
//--------------------------------------------------------------------------------------------
void ProtocolCHN::damagePerSeconBitInByte(QJsonObject & jsonObj)
{
    QByteArray ba;

    if(jsonObj.contains("cmd") && jsonObj.value("cmd").isString()){
        QJsonValue objVal = jsonObj.value("cmd");
        if (objVal == "ExchangeDCE" ){
            if(jsonObj.contains("data") && jsonObj.value("data").isString()){
                 QJsonValue objVal = jsonObj.value("data");
               //  ba.append(objVal.toString().toUtf8());
                 ba.append(QByteArray::fromBase64(objVal.toString().toUtf8().data()));
            }
        }
    }
    for(int i = 0; i < ba.count(); ++i){
       if(m_param_err != 0){      // если парам. ошибки = 0 => последовательность передаем без искажений
         if(!(m_param_count % m_param_err)){
             int a = ba[i];
              ba[i] = ~a;
              m_param_count = 0;
          }
          ++m_param_count;
       }
    }
     jsonObj.insert("data", ba.toBase64().data()); // помещаем обратно закодированную послед. с несколькоими поврежд символами
}
//--------------------------------------------------------------------------------------------
// в объекте принятой команде меняем местами номера УВС отправителя и УВС получателя
void ProtocolCHN::changeDCEid(QJsonObject & jsonObj)
{
    if(jsonObj.value("cmd")=="ExchangeDCE"){ // если через Сервер Обмена идет команда ExchangeDCE

        if(jsonObj.contains("DCE_ID_OPPN")){  // если в команде имеется это поле
            int dce_id = jsonObj.value("DCE_ID").toInt();
            int dce_oppn_id = jsonObj.value("DCE_ID_OPPN").toInt();
            jsonObj.insert("DCE_ID", dce_oppn_id);
            jsonObj.insert("DCE_ID_OPPN", dce_id);
        }
        else{
             jsonObj.insert("DCE_ID_SRC",jsonObj.value("DCE_ID").toInt());
             jsonObj.insert("DCE_ID",jsonObj.value("DCE_ID_DST").toInt());
             if(jsonObj.contains("DCE_ID_DST"))
               jsonObj.remove("DCE_ID_DST");
        }

    }
    else{                                    // если все остальные команды
          int dce_id = jsonObj.value("DCE_ID").toInt();
          int dce_oppn_id = jsonObj.value("DCE_ID_OPPN").toInt();
          jsonObj.insert("DCE_ID", dce_oppn_id);
          jsonObj.insert("DCE_ID_OPPN", dce_id);
    }
}
//--------------------------------------------------------------------------------------------
 // слот подготовки сообщения(ответного или инициативного) состоящего из объекта json для отправки
void ProtocolCHN::sendMsgOppn(const QJsonObject & jsonObj)
{
     QByteArray pack_to_send;
     pack_to_send = prepareMsg(jsonObj);    // если сообщение нужно отправить в xml формате -
                                            // преобразование json->xml происходит внутри preparePacket(jsonObj)
     QVector<QTcpSocket*> client_list;
     client_list = m_connectDCE->getSocketOpponents();   // список клиентов которым планируется рассылка сообщения

     for (int i = 0; i < client_list.count(); ++i){
        m_connectDCE->sendToClient(client_list.at(i)->socketDescriptor(), pack_to_send);
     }
}
//-----------------------------------------------------------------------------------------------
