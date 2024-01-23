#include "transportUDP.h"

//--------------------------------------------------------
TransportUDP::TransportUDP(QObject *parent) : QObject(parent)
{
   qRegisterMetaType<MsgDescrS>();

   QString log_name{"journal_transport_udp.log"};
   QString main_title{"NUM Time_REC Module_Use TX/RX Time_CMD_TX/RX Transp_Message "};

   m_log_maker = new LogMaker(log_name, true, main_title);
   m_log_maker->logOpen();
}
//-------------------------------------------------------
TransportUDP::~TransportUDP()
{
    m_log_maker->logClose();
    delete m_log_maker;
}
//--------------------------------------------------------
void TransportUDP::addToLog(int k,  QByteArray & ba, QDateTime tm, LogUseEnum flag_use)
{
    QString data;
 //   QByteArray data_ba;
    //data_ba.append(ba);
    ++m_count;

    if(k)
      data = " TX";
    else
      data = " RX";

   data = data + " "+ QString::number(tm.currentMSecsSinceEpoch());//tm.toString("hh:mm:ss.zz");
   data = data + " ";
   data = data + ba.toHex(':'); // разделитель между байтами
  // if(k)  //  если сообщение исходящее (TX)
   //  data = data + " \n"; //отделяем его от следующего входящего(RX)

   m_log_maker->logWrite(m_count, flag_use, data);
}
//------------------------------------------------------------------
// запуск прослушки порта по кнопке с формы
void TransportUDP::slotBindingPort(int port)
{
    // create a QUDP socket
    m_socket = new QUdpSocket(this);
    bool b_open = m_socket->open(QIODevice::Append| QIODevice::ReadOnly);
    qDebug()<<"Socket is opened: "<< b_open;
    m_bind_port = port;       
    bool bind_res = m_socket->bind(QHostAddress::Any, m_bind_port);
    //   qDebug()<<"Regim of socket open: "<< m_socket->openMode();
    //   qDebug()<< "Port binding: "<< m_bind_port;

    if(bind_res){
       connect(m_socket, &QUdpSocket::readyRead, this, &TransportUDP::slotReadyRead);
     //  connect(m_socket, &QUdpSocket::errorOccurred, this, &TransportUDP::slotError);  // для версии Qt 5.15.2
      connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(slotError(QAbstractSocket::SocketError))); // старые версии Qt

    }
}
//----------------------------------------------------------------
// установка адреса корреспондента
void TransportUDP::setCorrAdr(NetS net_adrs)
{
    m_adr_corr.port = net_adrs.port;
    m_adr_corr.address = net_adrs.address;
}
//--------------------------------------------------------
// текущая рабочая функция считывания из сокета
int TransportUDP::readSocket_simpl(QByteArray &signalIQ, QUdpSocket & udp_socket) // возвратим число пакетов в перем. signalIQ
{
    // when data comes in
    QByteArray   buffer;
    QHostAddress sender;
    quint16      senderPort;
    QDataStream in(&buffer, QIODevice::ReadOnly|QIODevice::Append);
    in.setVersion(QDataStream::Qt_5_11);
    RtpHeaderS  rtp_pack;
    quint64     size_pack{0} ;   // размер UDP пакета считаного из сокета
    signalIQ.clear();

    size_pack = udp_socket.pendingDatagramSize();  // размер данных считываемых из сокета
    buffer.resize(size_pack);
    udp_socket.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    QDateTime tm = QDateTime::currentDateTime();
    addToLog(0,  buffer, tm, TRANSP_UDP);            // добавляем запись в лог записанном в  сокет сообщ.

    if(size_pack > 0){  // если из сокета считали кол-во бит >0
       rtp_pack.size = size_pack - SZ_HDR_PACK;  // размер поля данных RTP пакета
       rtp_pack.data = new char[rtp_pack.size];  // выделение массива для данных
       in >> rtp_pack;
       m_pass_num = rtp_pack.Sequence_number;    // номер пакета принятого из заголовка RTP

               // контроль пропуска пакетов
                if((m_pass_num - m_pass_num_pre)>1  /*&& (m_pass_num - m_pass_num_pre) <20*/){ // регистрация пропуска пакетов

                    switch (m_pass_num - m_pass_num_pre) {

                    case 2:    // пропуск в 1 пакет
                        ++m_err_pack[0];
                        break;
                    case 3:   // пропуск в 2 пакета
                        ++m_err_pack[1];
                        break;
                    case 4:  // пропуск в 3 пакета
                        ++m_err_pack[2];
                        break;

                    default:  // пропуск более чем в 3 пакета
                        ++m_err_pack[3];
                        break;
                    }
                    //qDebug()<<"Seq_get_num and pre: "<< m_pass_num << m_pass_num_pre;
                    //qDebug()<<"bytesAvailable: "<< udp_socket.bytesAvailable();
                }
               m_pass_num_pre = m_pass_num;
              //
               QString err_journ{""};
               err_journ = "lost_I: "+QString::number(m_err_pack[0])+" lost_II: "+
                                      QString::number(m_err_pack[1])+" lost_III: "+
                                      QString::number(m_err_pack[2])+" lost_>_III: "+
                                      QString::number(m_err_pack[3]);
               emit sigErrLost(err_journ);
       signalIQ.append(rtp_pack.data, rtp_pack.size);
       //delete[] rtp_pack.data;    //переносим освобождение выделенной памяти в protocolRadio
    }

    if (signalIQ.length() > 0)
        return m_pass_num; // номер принятого пакета из заголовка RTP
    else
        return 0;          // или 0 пакетов
}
//----------------------------------------------------------------
void   TransportUDP::slotZpchChanged(int value)        // установка ЗПЧ в пакете UDP
{
   m_index_zpch = value;
}
//----------------------------------------------------------------
//  обработчик ощибок возникших при работе соединения
void TransportUDP::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Ошибка: " + (err == QAbstractSocket::HostNotFoundError ?
                     "Сервер не обнаружен." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "Удаленный сервер закрыт или отключился." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "Отказано в соединении." :
                     QString(m_socket->errorString())
                    );
     QString error ="error";
     emit sigOkError(error, QTime::currentTime());  // передача сведений об ошибке
     qDebug()<<"Udp socket error: "<<err;
   //  QMessageBox::critical(0, "Сообщение", strError + " Приложение клиента будет закрыто!","");
 }
//--------------------------------------------------------
void TransportUDP::writeSocket(QByteArray & ba, QUdpSocket & udp_socket, quint8 freq)
{
    Q_UNUSED(freq);
    RtpHeaderS  rtp_pack;                        // транспорт RTP
    ++m_pass_num;                                // номер очередного формируемого пакета
    rtp_pack.Sequence_number = m_pass_num;
    rtp_pack.data = ba.data();
    rtp_pack.size = ba.count();                  // поле(размер поля данных) не передается в UDP пакете, служит для удобства
    rtp_pack.Timestamp += 400;//static_cast<quint32>((ba.count()+1)/2);// проверить у Гука
                         // POINT_PACK*16/SAMP_RATE

    quint8 freq_num =static_cast<quint8>(m_index_zpch);//freq;          // значение запрашиваетс в модуле CoreDCE у модема// 16 бит (старший байт - частота, младший 0x80)

    rtp_pack.freq = (freq_num << 8) | 0x80;            //?

    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_11);
    out << rtp_pack;

    QDateTime tm = QDateTime::currentDateTime();
    udp_socket.writeDatagram(datagram, QHostAddress(m_adr_corr.address), m_adr_corr.port);/*QHostAddress::LocalHost*/
 //   qDebug()<<"Сообщение корреспонденту (порт/ адрес:)"+QString::number(m_adr_corr.port)+"/"+m_adr_corr.address;
     addToLog(1,  datagram, tm, TRANSP_UDP);            // добавляем запись в лог записанном в  сокет сообщ.

    emit sigIQToRadio(ba, m_pass_num);  // сигнал об отправке пакета клиенту под номером m_pass_num

}
//-----------------------------------------------------------------------
// основная функция отправки пакета Серверу
void TransportUDP::sendToCorr(QByteArray & ba, quint8 freq)
{
    if(m_socket->isOpen())
       writeSocket(ba, *m_socket, freq);
}
//--------  основные слоты модуля TransportUDP -------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//  обработчик события наличия  в сокете данных от корреспондента для считывания
void TransportUDP::slotReadyRead()
{   
    m_signalIQ.clear();
    int count_pack{0};
    count_pack = readSocket_simpl(m_signalIQ, *m_socket);

    emit sigIQFromRadio(m_signalIQ, count_pack);  // передаем считанные данные в модуль ProtocolRadio
}
//---------------------------------------------------------------------
// обработка сообщения о разрыве соединения
void TransportUDP::slotDisconnect()
{   
    m_socket->close();
    qDebug()<<"Socket is closed ";
}
//=====================================================================
//=====================================================================
//---------------------------------------------------------------------
// эксперемент!!! не используется
int TransportUDP::readSocket_compl(QByteArray &signalIQ, QUdpSocket & udp_socket) // возвратим число пакетов в перем. signalIQ
{
    // when data comes in
    QByteArray   buffer;
    QHostAddress sender;
    quint16      senderPort;
    QDataStream  in(&buffer, QIODevice::ReadOnly|QIODevice::Append);
    RtpHeaderS   rtp_pack;
    quint64      size_pack{0} ;   //
    int          num_pack{0};

    in.setVersion(QDataStream::Qt_5_11);
    size_pack = udp_socket.pendingDatagramSize();
    buffer.resize(size_pack);
    udp_socket.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

       num_pack = size_pack / SZ_PACK;
       int p = size_pack % SZ_PACK;
      if (p){
        ++num_pack;
        //qDebug()<<"Packets_num: "<< num_pack;
        }
       if(num_pack >1 )
          qDebug()<<"Packets_num: "<< num_pack;

       for (int i = 0 ; i < num_pack; ++i){ // цикл по всем принятым пакетам

        rtp_pack.size = SZ_PACK - SZ_HDR_PACK;  // размер поля данных RTP пакета
        rtp_pack.data = new char[rtp_pack.size]; // выделение массива для данных
        in >> rtp_pack;
        m_pass_num = rtp_pack.Sequence_number;
        if((m_pass_num - m_pass_num_pre)!=1){
           qDebug()<<"Seq_get_num and pre: "<< m_pass_num << m_pass_num_pre;
           qDebug()<<"bytesAvailable: "<< udp_socket.bytesAvailable();
        }
        m_pass_num_pre = m_pass_num;
        signalIQ.append(rtp_pack.data, rtp_pack.size);
        delete[] rtp_pack.data;
    }

    if (signalIQ.length() > 0)
        return m_pass_num;    // номер принятого пакета из заголовка RTP
    else
        return 0;          // или 0 пакетов
}
//--------------------------------------------------------------------------------------



