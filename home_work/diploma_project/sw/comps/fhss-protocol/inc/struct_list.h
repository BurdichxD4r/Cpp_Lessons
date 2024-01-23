#ifndef STRUCT_LIST_H
#define STRUCT_LIST_H

#include <QString>
#include <QVector>
#include "enum_list.h"
#include <QJsonObject>
#include <complex>

typedef  std::complex<short> short_complex;
typedef  std::complex<int> int_complex;


// структура данных слота для передачи по udp
struct SlotFieldS{
    short_complex *signal = nullptr; //  модулированный слот
    int signal_len {0};               // его длинну
    int freq_num {0};                 // частота для передачи слота
};
// структура данных пакета (часть слота) для передачи по udp
struct PackFieldS{
    QByteArray ba_pack;        //  данные пакета для передачи
    int freq_num {0};         // частота для передачи слота
};

// структура настроек содединения
struct NetS{
    int     port    = 2323;
    QString address = "127.0.0.1";

};
// структура характеристик содединения клиента с сервером
struct NetConnectS{
    int     discr   = 0;           // внутр. дискриптор соединения
    int     port    = 2323;        // порт клиента
    QString address = "127.0.0.1"; // адрес клиента
    QString client_type = "UVS";   // тип клиента

};

struct TagFeatures{
    bool open{false};  // true -open, false -close
    bool repeat{false}; //true - повторяется, false - не повторяется (совпадение полное)
    bool pre_tag_same{false}; // true - предыдущий тег - то же имя, false - предыдущий тег - другое имя
    bool pre_tag_open{false};
    bool pre_tag_repeat{false};
    //(предыдущие теги могут быть открытыми или закрытыми, рассматривается совпадение по имени тега)
};

// структура для описания версии ПО протокола
struct Version{
   QString version{"0.1.3"};
   QString build{"0.1.2.3456]78"};
   QString vendor{"PJSC IN{}TELTECH"};
   QString description{"DTE-DCE protocol"};
};

// структура задающее состояние УВС
struct UVSParamS{
    quint32            DCE_ID{0};         // позывной модема
    quint32            DCE_ID_DST{0};     // позывной получателя (оппонента)
    quint64            create_time{0};    // время создания
    quint64            enable_time{0};    // время включения
    PACStatusEnum      stage{INIT};       // состояние модема на текущий момент (INIT = 0, ENABLE,  NET_SYNC, NET_CONNECT, EXCHENGE, NET_DISCON)
    int                msg_id{10};        // номер принятой команды ExchangeDCE
    QString            radio_role{"RX"};  // роль в радио передаче
    int                busy{0};           // занят -1, не занят - 0
    int                s_n{0};            // отношение сигнал/шум
    int                err_RS{0};         // кол-во ошибок Рида-Салам.
    QString            status{""};        // статус состояния (start,enqueued, recv, end, )
    QVector<quint32>   internalTime;     // массив внутр. времени модема
    int                freqRX{0};        // рекомендуемая частота приема
    int                freqTX{0};        // рекомендуемая частота передачи
    int                msgsInQue{0};      // сообщ. в очереди на передачу
    int                bytesInQue{0};    // байт в очер. на передачу
    QString            type{"raw"};      // тип передачи
    int                samplRate{8000};     // частота дискритизации
    int                modeMDM{5};       // вариант режима работы

};
// структура задающее состояние протокола
struct ProtocolParamS{
    Version            version;              // версия ПО протокола
    ClientTypeEnum     name_term;             // тип терминала
    NetS               net_adr;               // адрес в сети
    QString            radio_role{"RX"};      // роль в радио передаче

};

// структура содержания команды в привязке к дискриптору
struct MsgDescrS{
    int         descr;
    QJsonObject obj_msg_in;
    QByteArray  ba_msg_in;
    QJsonObject obj_msg_out;
    QByteArray  ba_msg_out;
};

#endif // STRUCT_LIST_H
