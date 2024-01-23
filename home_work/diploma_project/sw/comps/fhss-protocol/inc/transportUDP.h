#ifndef TRANSPORTUDP_H
#define TRANSPORTUDP_H

#include <QObject>
#include <QTime>
#include <QtNetwork/QUdpSocket>
#include <QColor>

#include "enum_list.h"
#include "struct_list.h"
#include "protocolRTP.h"
#include "logmaker.h"

Q_DECLARE_METATYPE(MsgDescrS)

const qint32  SZ_PACK = 1236;     // размер пакета с заголовком
const qint32  SZ_HDR_PACK = 36;   // размер заголовка пакета
const qint32  SAMP_RATE = 8;      // частота дискритизации
const qint32  POINT_PACK = 200;   // кол-во точек в пакете I,Q)

class TransportUDP : public QObject
{
    Q_OBJECT
public:
    explicit TransportUDP(QObject *parent = nullptr);
            ~TransportUDP();

    void    setCorrAdr(NetS net_adrs);                   // установка адреса корреспондента
    void    setRecvAdr(NetS net_adrs);                  // установка адреса приёмника (только на стороне ПРМ)
    void    setRadioDirName(QString dir_name);           // установка имени радионаправления
    void    sendToCorr(QByteArray & codo_to_send, quint8 freq); // отправка кодограммы корреспондену
   // quint16 getPortNumber();                           // получение номера порта к которому привязан сокет

public slots:
    void    slotZpchChanged(int);         // установка ЗПЧ в пакете UDP
    void    slotBindingPort(int port);    // сигнал из протокола о начале прослушке сокетом порта

    // void slotConnected();               // обработка подключения к корреспонденту
    void    slotDisconnect();             // обработка отключения от корреспондента
    void    slotReadyRead();              // обработка события наличия кодограммы в сокете для чтения
    void    slotError(QAbstractSocket::SocketError err);  // ошибка, возмникшая в сокете при работе


protected:
       // поля из команды EnableDCE--------------------------
    int             m_bind_port{0};           // номер порта прослушки
    int             m_index_zpch{0};          // индекс ЗПЧ (в пакете UDP)
    NetS            m_adr_corr;               // сетевой адресс корреспондента

    //QString       m_radioDirName{""};       // имя радио направления
    //NetS          m_adr_rcvr;               // сетевой адрес приёмника (только на стороне ПРМ)для управление частотой приёма
      //------------------------------------------------------

    QUdpSocket     *m_socket{nullptr};        // сокет для отправки/ приёма кодограммы
    QByteArray      m_signalIQ;               // сообщение целиком в виде signalIQ ()    

    quint8          m_freq;                   // значение частоты, запрашивается в CoreDCE, передается в заголовке RTP

    quint16         m_pass_num{0};                // номер посылки для передачи
    quint16         m_pass_num_pre{0};            // номер посылки для передачи => для оценки потеряных пакетов
    qint32          m_err_pack[4]{0,0,0,-1};      // массив с данными о пропуске пакетов

    LogMaker        *m_log_maker;                 // логирование сообщений
    int             m_count{0};                   // счетчик сообщений для лога


    int             readSocket_simpl(QByteArray &signalIQ, QUdpSocket & udp_socket);
    int             readSocket_compl(QByteArray &signalIQ, QUdpSocket & udp_socket);
    void            writeSocket(QByteArray & ba, QUdpSocket & udp_socket, quint8 freq);
    void            addToLog(int k,  QByteArray & ba, QDateTime tm, LogUseEnum flag_use);
signals:

    //void  sigMsg       (int,  MsgDescrS &, QTime);    // сигнал о ПРИНЯТОМ/ОТПРАВЛЕННОМ сообщении
    //void  sigMsgToJourn(int,  QByteArray &, QTime);   // сигнал с принятым/отправленном сообщением для отображения в Журнале
    void  sigOkError   (QString &, QTime );            // сигнал о состоянии соединения
    void  sigReadyRead (QByteArray & buf);             // сигнал с буф. считанным из радиоканала
    void  sigIQFromRadio( QByteArray &dataIQ, int num_pack, int flag_pass = RX);  // данные из радио канала обмена, (flag_pass = RX)
    void  sigIQToRadio(QByteArray &ba, int num_pack, int flag_pass = TX );       // данные в радио канал (flag_pass = TX)
    void  sigErrLost(QString & err_journ);     // отправка статистики потерянных пакетов
};

#endif // TRANSPORTUDP_H
