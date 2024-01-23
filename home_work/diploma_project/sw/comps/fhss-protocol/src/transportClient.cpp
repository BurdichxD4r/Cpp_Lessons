// модуль коннектора на стороне КЛИЕНТА, осуществляющего связь на уровне сокетов tcp между клиентом и сервером

#include "transportClient.h"
#include <QDataStream>
#include <QTime>
#include <QMessageBox>
#include <QHostInfo>

//const int TX = 1;   // передача
//const int RX = 0;   // прием

//-----------------------------------------------
TransportClient::TransportClient(NetS & net_adr, TypeMsgEnum type_msg):
    TransportBase(type_msg)
{

    m_socket = new QTcpSocket(this);
    m_socket->connectToHost(net_adr.address, net_adr.port);  // присоединение к серверу
       // создаем обработчики основных функций соединения
    connect(m_socket, &QTcpSocket::connected, this, &TransportClient::slotConnected);  // слот соединения сокета с сервером
    connect(m_socket, &QTcpSocket::readyRead, this, &TransportClient::slotReadyRead);  // слот сокета когда в нем имеются данные для считывания
    //connect(m_socket, &QTcpSocket::errorOccurred, this, &TransportClient::slotError);  // для версии Qt 5.15.2
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(slotError(QAbstractSocket::SocketError))); // старые версии Qt

}
//----------------------------------------------------------------
TransportClient::~TransportClient()
{
}
//----------------------------------------------------------------
void TransportClient::setColor(QColor cl)
{
    m_color = cl;
}
//----------------------------------------------------------------
QColor TransportClient::getColor()
{
    return m_color;
}
//--------   Основные процедуры коммуникации(приема / передачи)---
//       вызываются из MWind -------------------------------------
//----------------------------------------------------------------
// основная функция отправки пакета Серверу
void TransportClient::sendToServer(QByteArray & ba)
{   
    writeSocket(ba, *m_socket, m_type_msg);
}
//--------  основные слоты модуля Connector -------------------------
//--------------------------------------------------------------------
//-------------------------------------------------------------------
//  обработчик события наличия  в сокете данных от сервера для считывания
void TransportClient::slotReadyRead()
{
    readSocket(m_msg_whole, *m_socket, m_type_msg, m_vec_data);
}
//--------------------------------------------------------------------------------
// обработка события соединеия с севером с высылков сообщения о штатном состоянии соединия в VerificationClient
void TransportClient::slotConnected()
{
    QString str = "ok";
    emit sigOkError(str, QTime::currentTime());
}
//-------------------------------------------------------------------
// обработка сообщения о разрыве соединения пришедшая из VerificationClient
void TransportClient::slotDisconnect()
{    
    m_socket->close();
}
//----------------------------------------------------------------
//  обработчик ощибок возникших при работе соединения
void TransportClient::slotError(QAbstractSocket::SocketError err)
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
     QMessageBox::critical(0, "Сообщение", strError + " Приложение клиента будет закрыто!","");
 }
//------------------------------------------------------------------
// получение номер порта из данных сокета
quint16 TransportClient::getPortNumber()
{
    if (m_socket->isOpen())
       return m_socket->localPort();
    else
       return 0;
}
//-------------------------------------------------------------------
