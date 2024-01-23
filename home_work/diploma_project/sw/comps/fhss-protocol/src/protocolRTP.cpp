// заголовок rtp пакета
#include "protocolRTP.h"
//--------------------------------------------------------------------
ProtocolRTP::ProtocolRTP()
{
}
//--------------------------------------------------------------------
// запись пакета rtp в поток
QDataStream& operator << (QDataStream &out, const RtpHeaderS &any)
{
    quint8 val = ((any.V<<6)|(any.P<<5)|(any.X<<4)|(any.CC));
    out << val;
    quint8 val1 = ((any.M<<7)|(any.PT));
    out << val1;
    out << any.Sequence_number;
    out << any.Timestamp;
    out << any.SSRC;
    out << any.extId;
    out << any.extLen;
    out << any.flags;
    out << any.freq;
    out << any.SrcChannel;
    out << any.DstCahnnel;
    out << any.ChType;
    out << any.LenReserv;
    out.writeRawData(any.data, any.size);
    return out;
}

//--------------------------------------------------------------------
// считывание пакета rtp из потока
QDataStream& operator >> (QDataStream &in, RtpHeaderS &any)
{
    quint8 val, val1;
    in >> val;
    in >> val1;

    in >> any.Sequence_number;
    in >> any.Timestamp;
    in >> any.SSRC;
    in >> any.extId;
    in >> any.extLen;
    in >> any.flags;
    in >> any.freq;
    in >> any.SrcChannel;
    in >> any.DstCahnnel;
    in >> any.ChType;
    in >> any.LenReserv;    
    in.readRawData(any.data, any.size); // предваритьльно выделяется дин. массив any.data
    return in;
}
//-----------------------------------------------------------------------
