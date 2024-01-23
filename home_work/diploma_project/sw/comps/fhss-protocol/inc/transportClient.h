// Дочерний класс от класса TransportBase.
// является КЛИЕНТСКИМ МОДУЛЕМ при установлении tcp соединения

#ifndef TRANSPORTCLIENT_H
#define TRANSPORTCLIENT_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QTcpServer>
#include <QTime>
#include <QColor>
#include "transportBase.h"


class TransportClient: public TransportBase
{
    Q_OBJECT
public:

    explicit TransportClient(NetS &net_adr, TypeMsgEnum type_msg = JSON);
    ~TransportClient();
    void    sendToServer(QByteArray &pack_to_send);    // отправка пакета серверу
    quint16 getPortNumber();                           // получение номера порта к которому привязан сокет
    QColor  getColor();
    void    setColor(QColor cl);

public slots:

    void slotConnected();          // обработка подключения к серверу
    void slotDisconnect();         // обработка отключения от сервера
    void slotReadyRead();          // обработка события наличия сообщиеня в сокете для чтения
    void slotError(QAbstractSocket::SocketError err);  // ошибка, возмникшая в сокете при работе

private:
    QTcpSocket          *m_socket;   // сокет для отправки сообщения серверу
    QColor               m_color;

};

#endif // TRANSPORTCLIENT_H
