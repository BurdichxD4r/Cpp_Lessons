// Базовый класс TransportBase -модуль отвечающий за установление соединения на уровне сокетов и представления
// сообщения в виде пакетов (при помощи модуля Packet)

#include "transportBase.h"
#include "translatorXml.h"
#include <QMessageBox>
#include "packet.h"

//--------------------------------------------------------
TransportBase::TransportBase(TypeMsgEnum type_msg):
    m_type_msg (type_msg)
{
 //  qRegisterMetaType<MsgDescrS>();

    QString log_name{"journal_transport_tcp.log"};
    QString main_title{"NUM Time_REC Module_Use TX/RX Time_CMD_TX/RX Transp_Message "};

    m_log_maker = new LogMaker(log_name, true, main_title);
    m_log_maker->logOpen();
}
//--------------------------------------------------------
void TransportBase::addToLog(int k,  QByteArray & ba, QDateTime tm, LogUseEnum flag_use)
{
    QString data;   
    ++m_count;

    if(k)
      data = " TX";
    else
      data = " RX";

   data = data + " "+ QString::number(tm.currentMSecsSinceEpoch());//tm.toString("hh:mm:ss.zz");
   data = data + " ";
   for (int i = 0; i <ba.count(); ++i){
       data = data + ba.at(i) ;
   }

   //if(k)  //  если сообщение исходящее (TX)
   //  data = " //--\n"; //отделяем его от следующего входящего(RX)

   m_log_maker->logWrite(m_count, flag_use, data);
}
//--------------------------------------------------------
void TransportBase::readSocket(QByteArray &msg_whole, QTcpSocket & tcp_socket,
                               TypeMsgEnum type_msg, QVector<QByteArray> & vec_data)
{
    bool empty = false;
    Packet packet;
    QByteArray  b;
    MsgDescrS   msg;

    do{
         msg_whole.append(tcp_socket.readAll());  // получили ли сообщение целиком (закодированное)
         QDateTime tm = QDateTime::currentDateTime();
         //addToLog(0,  msg_whole, tm, TRANSP_TCP);            // добавляем запись в лог о прочитанном из сокета сообщ.
         cutToPack(msg_whole);
         empty = static_cast<uint64_t>(tcp_socket.bytesAvailable())  < sizeof(quint16);  // не пустой ли буф. сокета

    } while(!(checkMsgAll(msg_whole, type_msg) || empty));

   for(int i = 0; i < vec_data.count(); ++i){
       QByteArray clear_ba;

       if(type_msg == JSON){
           TypeErr err = packet.getClearMsg (clear_ba, vec_data.at(i));
           if(err!=NO_ERR)
             QMessageBox::critical(0, "Сообщение","В процессе распаковки пакета произошла ошибка" ,"");
        }
        else{   // сообщение XML
             clear_ba.append( vec_data.at(i));
        }
          // здесь необходимо отправить сигнал MW что пришли данные и какие (в виде строки)
        emit sigMsgToJourn(RX, clear_ba, QDateTime::currentDateTime());  // сигнал о передачи сообщения в журнал
        b = getJsonPack(clear_ba);//
        msg.descr = tcp_socket.socketDescriptor();
        msg.ba_msg_in = b;
        emit sigMsg(RX, msg, QDateTime::currentDateTime());  // сигнал о передачи сообщения на уровень verificationLevel
    }//!!!
        vec_data.clear();
  // }  //!!!
}
//---------------------------------------------------------
TransportBase::~TransportBase()
{
   m_log_maker->logClose();
   delete m_log_maker;
}
//--------------------------------------------------------
void TransportBase::writeSocket(QByteArray & ba, QTcpSocket & tcp_socket,TypeMsgEnum type_msg)
{
    QByteArray  ba_out;   // закодированный пакет для отправке серверу
    QByteArray  b;
    Packet      packet;

     // кодируем текстовую часть сообщения перед запаковыванием в транспортрый пакет
    if(type_msg == JSON){  // если формат сообщения JSON создаем доп транстпортный пакет
       b = getJsonPack(ba);//
       emit sigMsgToJourn(TX, b, QDateTime::currentDateTime());  // сигнал о передачи сообщения в сокет
       packet.setMsg(b);
       ba_out = packet.packetCreate();
    }
    else{
        b = getXmlMsg(ba);//
        emit sigMsgToJourn(TX, b, QDateTime::currentDateTime());  // сигнал о передачи сообщения в сокет
        ba_out.append(b);
        }
    QDateTime tm = QDateTime::currentDateTime();
    tcp_socket.write(ba_out);                       // отправляем закодированное сообщение
    //addToLog(1,  ba_out, tm, TRANSP_TCP);            // добавляем запись в лог об отправленном в сокет сообщ.
    ba.clear();

}

//--------------------------------------------------------
// функция проверки имеется ли в переданной последовательности байт сообщение целиком
// или еще необходимо ждать окончание сообщения в следующей порции принимаемой информ.
bool TransportBase::checkMsgAll(QByteArray ba, TypeMsgEnum type_msg)
 {
     int pos_beg = -1;
     int pos_end = -1;
     bool res = false;
     quint32 size_mess{0};  // считанная длина сообщения
     quint32 size_get{0};   // измеренная длина пакета
   //  QTime tm = QTime::currentTime();
   //  addToLog(0,  ba, tm);            // добавляем запись в лог о прочитанном из сокета сообщ.
     if(type_msg == JSON){
       if(ba.length()>8){    // длина принятой последовательности более чем длина заголовка
        for (int i = 0; i < ba.length(); ++i){   // ищем начало пакета
            if ((ba.at(i) == (char) 0xC0)&&(ba.at(i+1) == (char) 0xAB)) {  // начало сообщения
                   ba = ba.right(ba.count() - i );  // отбрасываем всё до позиции начала пакета, а так же байты C0, AB
                   if(ba.count() > 6){   // если в последовательности все еще больше данных чем просто заголовок
                      QByteArray sa;
                      sa.append(ba.mid(i+2, 4));  // копируем данные из послед. для расшифровки длины принятого пакета
                      size_mess = *(quint32 *)(sa.data()); // получаем длину из заголовка
                    }
                break;
             }
        }
       }
        size_get = (quint32) ba.count() - 8; // -8 - отнимаем  заголовок и оцениваем длину оставщейся
                                             // последовательности (пакет без транспорт. заголовка)
        if(size_mess == size_get)            // если теоретическая и практическая длины пакета совпадают
            res = true;                      // заключаем что пакет получен полностью и может быть обработан
     }
     else{  // XML
        // ba_decoding.append(QByteArray::fromBase64(ba)); // декодируем закодированное сообщение
        pos_beg = ba.indexOf("<message>");  // далее работаем с разкодированным сообщением
        if (pos_beg > -1){
             pos_end = ba.indexOf("</message>");
             if(pos_end > -1)
               res = true;
         }
        else
             res = false;
     }
     return res;               // если пакет получен не полностью ф-ция вернет false
 }
//-----------------------------------------------------------------------
QByteArray TransportBase::getJsonPack(QByteArray & xmlJsonPack)
{
   QByteArray json_ba;
    if(m_type_msg == XML){
       TranslatorXML trans_obj;
       json_ba = trans_obj.translateToJson(xmlJsonPack);
     }
    else if (m_type_msg == JSON){
       json_ba = xmlJsonPack;
    }
    return json_ba;
}
//------------------------------------------------------------------------
// создадим команду по переданному объекту
// (сейчас объект obj заполен данными из переданного по сети пакета)
QByteArray TransportBase::getXmlMsg(QByteArray & xmlJsonPack)
{
    QByteArray xml_ba;
     if(m_type_msg == XML){
        TranslatorXML trans_obj;
        xml_ba = trans_obj.translateToXml(xmlJsonPack);
      }
     else if (m_type_msg == JSON){
        xml_ba = xmlJsonPack;
     }
     return xml_ba;      // возвращаем сообщение в xml формате
}
//-------------------------------------------------------------------
// очистка принятого сообщения от транспортного заголовка
// необходима для анализа полученного сообщения на целостность
void TransportBase::cutToPack(QByteArray &ba)
{
    QByteArray ba_beg;
    QByteArray cur_pack;

    // начало транстортного пакета
    ba_beg.append(0xC0);
    ba_beg.append(0xAB);

    if(m_type_msg == JSON){
      for(;;){
        int pos = ba.indexOf(ba_beg);
        ba = ba.right(ba.count() - pos); // последовательность начиная с 0x0C
        ba = ba.right(ba.count() - 2);   // последовательность после с 0x0C
        pos = ba.indexOf(ba_beg);
        cur_pack.clear();
        if (pos!=-1)
           cur_pack.append(ba.left(pos));        // получили первый пакет
        else
           cur_pack.append(ba);

        cur_pack.insert(0, ba_beg);
        if(checkMsgAll(cur_pack, m_type_msg)){
            m_vec_data.append(cur_pack);
            ba = ba.right(ba.count()-(cur_pack.size()-2)); // остаток после отрезания полного пакета
        }
        else{  // в переменной cur_pack нет собранного пакета
            return;
         }
       }
    }

    else if(m_type_msg == XML){ // не проверялся!!!!
       QString end_str("</message>");

       for(;;){
           int pos_beg = ba.indexOf("<message>");
           int pos_end = ba.indexOf("</message>") + end_str.size();
           cur_pack.clear();
           cur_pack = ba.mid(pos_beg, (pos_end -pos_beg));
           if(checkMsgAll(cur_pack, m_type_msg)){
              m_vec_data.append(cur_pack);
              ba = ba.right(ba.size() - pos_end);
           }
           else{
               return;
           }
       }
    }
}
//-----------------------------------------------------------------------------------
