// Дочерний класс от класса TransportBase.
// является СЕРВЕРВНЫМ МОДУЛЕМ при установлении tcp соединения, обслуживает список подкл. клиентов

#ifndef TRANSPORTSERV_H
#define TRANSPORTSERV_H

#include <QTime>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QColor>

#include "transportBase.h"


class TransportServ: public TransportBase
{
    Q_OBJECT

public:
    explicit TransportServ(NetS &net_adr, TypeMsgEnum type_msg = JSON);
    ~TransportServ();

    void                   sendToClient(int descr,         // дискриптор подключенного клиета
                                       QByteArray& ba);
    QTcpSocket*            getSocket();                        // функция доступа к дескриптору сокера (m_socket);
    QVector<QTcpSocket*>   getSocketOpponents();               // функция доступа к списку socket-ов  которым необходимо передать сообщение
    void                   namedClient(int descr, QString client_name);
    QVector <NetConnectS>  formClientListDCE();  // формирование списка подключенных клиентов для передачи в главное окно для отображения
    // все сокеты подключенные к серверу, кроме того от которого сообщение было принято

public slots:
    void   slotNewConnection();         // подключение новых клиентов
    void   slotStopServer();            // остановка сервера
    void   slotReadClient();            // чтение сообщений от клиента
    void   slotDiscClient();            // отключение клиента

private:

    QVector<QColor>      m_colors;
    QTcpSocket          *m_socket;         // сокет для отправки пакета (ему присваивается сокет реально созданного соединения)
    QTcpServer          *m_server;         // сервер для получения пакета
    QMap <int,                             // дескриптор
          QTcpSocket*>   SClients;         // пул подключенных клиентов к данному серверу
    QMap <int,                             // дескриптор
          QString>       SClientsType;     // пул подключенных клиентов к данному серверу (по типам (МАС, АУ, УВС))

signals:
    void      sigClientListDCE(QVector <NetConnectS>);
    void      sigNetAdr(NetS &);   // передача адреса в сети на котором установлен сервер
};



#endif // TRANSPORTSERV_H
