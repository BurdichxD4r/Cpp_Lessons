#ifndef PROTOCOLRTP_H
#define PROTOCOLRTP_H

#include <QObject>
#include <QDataStream>

// Структура RTP заголовка протокола "Созвездия"
struct RtpHeaderS
{
    // Заголовок
    quint8  V                = 2;      // 2 бит  Версия протокола
    quint8  P                = 0;      // 1 бит  Всегда 0
    quint8  X                = 1;      // 1 бит  Признак наличия расширенного заголовка 1
    quint8  CC               = 0;      // 4 бит  Всегда 0

    quint8  M                = 0;      // 1 бит  Всегда 0
    quint8  PT               = 0;      // 7 бит  Всегда 0

    quint16 Sequence_number  = 0;      // 16 бит Номер RTP пакета

    quint32 Timestamp        = 0;      // 32 бит ?

    quint32 SSRC             = 0;      // 32 бит Всегда 0
    quint16 extId            = 0x00FA; // 16 бит По умолчанию 0x00FA
    quint16 extLen           = 5;      // 16 бит По умолчанию 5
    quint16 flags            = 0x0001; // 16 бит Для АПД (было 0x0800, ИСПРАВЛЕНО В ПРОЦЕССЕ СОГЛАСОВАНИЯ!)
    quint16 freq             = 0x80;   // 16 бит (старший байт - частота, младший 0x80)
    quint32 SrcChannel       = 0;      // 32 бит Всегда 0
    quint32 DstCahnnel       = 0;      // 32 бит Всегда 0
    quint32 ChType           = 0;      // 32 бит Всегда 0
    quint32 LenReserv        = 0;      // 32 бит Всегда 0
    // Данные
    quint64 size             = 0;     // поле не передается (размер поля data)
    char*   data             = 0;
};
// Переопределение операторов для HDR_RTP структуры

QDataStream& operator << (QDataStream &out, const RtpHeaderS &any) ;
QDataStream& operator >> (QDataStream &in, RtpHeaderS &any) ;

class ProtocolRTP:public QObject
{
    Q_OBJECT
public:
    ProtocolRTP();    

private:
    RtpHeaderS m_rtp_header;
};

#endif // PROTOCOLRTP_H
