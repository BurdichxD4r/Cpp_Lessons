#include "verificationBase.h"
#include <QFile>
#include <QJsonParseError>
#include <QMessageBox>
#include <QJsonArray>


//-------------------------------------------------------------------------
VerificationBase::VerificationBase(TypeMsgEnum type_msg, bool format_msg):
    m_type_msg (type_msg), m_format_msg(format_msg)
{
    QString log_name{"journal_cmd.log"};
    QString main_title{"NUM Time_REC Module_Use TX/RX Time_CMD_TX/RX CMD_Message "};

    m_log_maker = new LogMaker(log_name, true, main_title);
    m_log_maker->logOpen();
}
//--------------------------------------------------------------------------
VerificationBase::~VerificationBase()
{
    m_log_maker->logClose();
    delete m_log_maker;
}
//--------------------------------------------------------------------------
// обработка сигнала из MW о необходимости разрыва соединения
 void VerificationBase::slotDisconnect()
 {
     emit sigDisconToConnector(); // сигнал в Connector
 }
//--------------------------------------------------------------------------
 // обработка сигнала из Connector о состоянии соединения
void VerificationBase::slotGetOkError(QString & st, QTime tm )
 {
     emit sigChangeWorkStatus(st, tm ); // сигнал в MW о состоянии соединения
 }
//--------------------------------------------------------
void VerificationBase::addToLog(int k,  QByteArray & ba, QDateTime tm, LogUseEnum flag_use)
{
    QString data;
    QByteArray data_ba, ba_64_e, ba_64_d;
    QJsonObject obj;
    QJsonDocument doc_in;
    QJsonParseError parseErr;
    QString data_str;

    doc_in = QJsonDocument::fromJson(ba,&parseErr);
    if (parseErr.error == QJsonParseError::NoError){  // если в последоват. нет ошибок
        obj = doc_in.object();
        if(obj.contains("data")){    // если это не ответная команды Replay...
           data_str = obj.value("data").toString();  // данные в кодировке Base64
           ba_64_e.insert(data_str.length(),data_str.toUtf8());
           ba_64_d.append(QByteArray::fromBase64(ba_64_e.data()));
           data_str="";
           data_str.append(ba_64_d.toHex(':'));
           obj.insert("data", data_str);
           QJsonDocument doc(obj);
           data_ba = doc.toJson(QJsonDocument::Compact);
         }
        else{
            //data_ba.append("//---\n");  // содержимое команды без поля data - не выводим в лог!!
           data_ba = doc_in.toJson(QJsonDocument::Compact);
        }
    }
           ++m_count;

          if(k)
             data = " TX";
          else
             data = " RX";

          data = data + " "+ QString::number(tm.currentMSecsSinceEpoch());//toString("hh:mm:ss.zz");
          data = data + " ";
          data.append(data_ba.data());
          m_log_maker->logWrite(m_count, flag_use, data);

}
 //-------------------------------------------------------------------------
// обработка сигнала из Connect о полученном/отправл сообще
 void VerificationBase::slotGetMsgToJourn(int k,  QByteArray & ba, QDateTime tm)
 {
     // сигнал в Widget о принятом/отправл.сообщении(последо. байт)k=0 -принятое сообще; k=1-отправляемое
     emit sigAddMsgToJournal(k, ba, tm );
     addToLog(k,  ba, tm, VERIFY);    // делаем запись в лог файл
 }
 //--------------------------------------------------------------------------
 QJsonObject VerificationBase::getObjectSampl(const QString cmd, const QString &suffix)
{      //_о - клиент, _i - сервер
    QJsonObject jObj;
         // получим шаблон сообщения из m_com_list (по данным cmd);
    jObj = m_com_list.value(cmd + suffix).toJsonObject();
    return jObj;// возвращаем частично заполненный шаблон ответной команды
    }
 //-------------------------------------------------------------------------
 bool VerificationBase::isCmdIdExist(QJsonObject  & jObj)
{
    if(jObj.contains("test")){   // объект является тестом и не изучается на наличие cmd.
        return true;
    }
    if((jObj.contains("cmd") && jObj.value("cmd").isString() && jObj.value("cmd")!="" )){ //&&
        // (jObj.contains("id") && jObj.value("id").isString() && jObj.value("id")!="" )){

        return true;  // поля cmd, id - присутствуют, имеют правильный тип, не пустые
    }
    else{
        m_error = "В сообщении нет обязательных полей, они пусты или их значения не совпадают по типу с ожидаемыми !";
        return false;
    }
 }
 //-----------------------------------------------------------------------
 // создаение json-объекта из json-последовательности байт
 QJsonObject VerificationBase::createObj(QByteArray & jsonPack)
{
     QByteArray json_ba;
     QJsonDocument   doc;            // json док. для загрузке в него принятой послед.
     QJsonParseError parseErr;       // ошибки возникающие при парсинге принятой послед. в json док.
     QJsonObject     jObj;

     doc = QJsonDocument::fromJson(jsonPack, & parseErr);
     if(doc.isObject()){
         jObj = doc.object();
     }
   return jObj;
 }
 //-------------------------------------------------------------------------
  // проверка последовательности пакета на наличие ошибок (json)
bool VerificationBase::jsonCheck(QByteArray & jsonPack)
{
    QJsonDocument   doc_in;            // json док. для загрузке в него принятой послед.
    QJsonParseError parseErr_in;       // ошибки возникающие при парсинге принятой послед. в json док.
    QJsonObject     jObj_in;           // json объект созданый из принятой по сети последовательности
    QString         err;
    QByteArray json_ba;

    json_ba.append(jsonPack);
    if(json_ba.count() > 0){          // принятое сообщение не пустое
       doc_in = QJsonDocument::fromJson(json_ba, & parseErr_in);
       if (parseErr_in.error != QJsonParseError::NoError){         // если есть ошибки JSON - выдаем сообщение
           err = parseErr_in.errorString();                        //"JSON-ошибка";
           m_error = "В сообщении JSON ошибка! ("+ err +")";
           return false;
        }
        else
           return true;
    }
    else {
          m_error = "Принятое сообщение пустое !";
          return false;
    }
}
//--------------------------------------------------------------------------
// процедура создания списка доступных команд в рамках действующего протокола
// создается по данным файла из ресурсов "command_list.txt"
void VerificationBase::createCommandList()
{
    Q_INIT_RESOURCE(com_list);
    QString name{":/file/inc/command_list.txt"};

    QFile file (name);
    QString json_str;
    QJsonParseError parseErr;
    QString error = "";

    if (!file.exists()){
     QMessageBox::critical(0, "Сообщение","Файл: command_list.txt - не существует!","");
    }
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) // Проверяем, возможно ли открыть наш файл для чтения
            return; // если это сделать невозможно, то завершаем функцию
       json_str = file.readAll(); //считываем все данные с файла в объект data
        //qDebug() << json_str;
    file.close();

    QJsonDocument doc;

    doc = QJsonDocument::fromJson(json_str.toUtf8(), & parseErr);
    if (parseErr.error != QJsonParseError::NoError){
       error = parseErr.errorString();//"JSON-ошибка";
       QMessageBox::critical(0, "Сообщение","В файле: command_list.txt - JSON ошибка! ("+error+")" ,"");
      }
    else if(doc.isObject()){
         QJsonObject root = doc.object();
    // делаем обход всего документа json,считывая имя команды и получая соответствующий объект команды
    // записываем все это в QVariantMap<QString, QVariant(QJsonObject)> m_com_list
         QString comName;
         QVariant comObj;
         for(int i = 0; i < root.count(); ++i){
            comName = root.keys().at(i);
            comObj = root.value(comName).toObject();
            m_com_list.insert(comName,comObj);
        }
    }
}
//---------------------------------------------------------------------
QMap<QString, QVariant> VerificationBase::getCmdList()
{
    return m_com_list;
}

//---------------------------------------------------------------------
// функция подготовки сообщения по данным объекта json
QByteArray VerificationBase::prepareMsg(const QJsonObject & objectCom)
{
    QByteArray ba;
    ba = createJsonMsg(objectCom, m_format_msg);
    return ba;
}
//--------------------------------------------------------------------
// создадим команду - сообщение  по данным объекта
// (сейчас объект obj заполен данными из переданного по сети пакета)
QByteArray VerificationBase::createJsonMsg(const QJsonObject  & obj, bool format_msg)
{
    QByteArray ba;
    QJsonDocument jsonDoc(obj);
    if(!format_msg)
        ba = jsonDoc.toJson(QJsonDocument::Compact);
    else
        ba = jsonDoc.toJson(QJsonDocument::Indented);
    return ba;  // возвращаем сообщение в json формат
}
//---------------------------------------------------------------------
// процедура рекурсивной обработки json объектов()
void VerificationBase::processObject(QJsonObject & obj_in,   // объект принятый, сформированный из входящей последовательности
                                QJsonObject & obj_samp,  // объект эталонный, сформированный по данным m_com_list т.е.из файла command_list
                                ModeEnum mode)           // режим обработки (CHECK - проверяется входящий объект на соответствие с эталонным,
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
               m_error = " Верификация. Структура команды: " + key_samp;
             }
           } // выдаем ошибку структуры JSON сообщения
       }   // обход эталонного объекта завершен
     }     // входной и эталонный объекты, переданные в функцию не пустые
 }
//---------------------------------------------------------------------------------------------
