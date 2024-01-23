#ifndef STRUCTS_LIST_H
#define STRUCTS_LIST_H

#include <QString>
#include <QVector>
#include <QList>
#include "enums_list.h"

// структура настроек содединения
/*struct NetS{
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

// структура для описания версии ПО
struct Version{
   QString version{"0.1.3"};
   QString build{"0.1.2.3456]78"};
   QString vendor{"PJSC IN{}TELTECH"};
   QString description{"DTE-DCE protocol"};
};

// структура задающее состояние УВС
struct UVSParams{
    uint32_t           DCE_ID{0};         // позывной УВС
    uint32_t           DCE_ID_OPPN{0};    // позывной получателя (оппонента)
    QString            create_time{""};   // время создания
    QString            enable_time{""};   // время включения
    PACStatusEnum      status{INIT};      // состояние на текущий момент (INIT = 0, ENABLE,  NET_SYNC, NET_CONNECT, EXCHENGE, NET_DISCON)
    Version            version;           // версия ПО загруженная в ПАК УВС
    ClientTypeEnum     name_term;         // тип терминала UVS
    int                msg_id{10};        // номер принятой команды ExchangeDCE
};
*/
//---------------------------------------------------------------
// структура данных для команды "enableDCE"
//---------------------------------------------------------------
struct RadioParams{
    QString    name;
    QString    ipv4;
    int        tcp_port;
    int        udp_port1;
    int        udp_port2;
};

struct EnableParamStruct{
    int                 dce_id;
    int                 mode_bw;    // полоса пропускания
    ModeSwitchEnum      modeSw;     // наименование режима
    int                 tm_switch;  // время перестройки частоты
    int                 mode_var;       // главный режим работы модема
    int                 freq_discr; // частота дискритизации
    ModeExchEnum        modeEx;     // уточнение режима (симплекс, дуплекс, полудуплекс
    int                 stage;      // этап
    int                 fr_offset;  // смещение частот
    int                 fr_all;     // кол-во частот
    int                 fr_active;  // кол-во активных частот
    ModeOffOnEnum       fr_adapt;   //
    ModeOffOnEnum       cons_adapt;  //
    bool                use_freq_list;     // используется ли список частот
    QList <int>         freq_list;  // список частот
    QList <int>         freq_list_prim;  // список частот НПП
    QList <int>         freq_list_add;  // список частот ЗЧ
    QList <RadioParams> radio_param;   // список радисредств
    int                 pcp_key;        // ключ ПСП
    int                 initPcpTime;    // нач. момнет времени для ПСП
    int                 modeFreqPcp;    // номер режима для рассчета РАб. частоты по ПСП
    QList <int>         input_port_list;  // список входных частот
    QList <int>         ppf_ind_list;  // список номеров ppf частот
    int                 ppf_number;    // число ppf
    QString             ppf_switch;    //
    QString             radio_dir_name;  // имя радио направления
    QString             input_prot;    // протокол взаимодействия на входе
    int                 output_port;    // порт на выходе
    QString             output_prot;    // протокол взаимодействия на выходе
    QString             output_adr;     // адресс на выходе
    QString             rcvr_prot;     // протокол взаимодействия на приеме
    int                 rcvr_port;     // порт на приеме
    QString             rcvr_adr;      // адресс на приеме
    int                 param_port;    // переменная для передачи порта ПАК МАС
};
//----------------------------------------------------------------
// структура данных для команды "disableDCE"
//---------------------------------------------------------------
struct DisableParamStruct{
    int dce_id;
};
//----------------------------------------------------------------
// структура данных для команды "netSynchroDCE"
//----------------------------------------------------------------
struct NetSYNCParamStruct{
    int              dce_id;
    int              dce_id_dst;
    uint32_t         imitIns;
    QVector<uint8_t> syncPack;
};
//----------------------------------------------------------------
// структура данных для команды "netConnectDCE"
//----------------------------------------------------------------
struct NetCONNECTParamStruct{
   int                 dceId;         // позывной сервера
   int                 dceIdCorr;     // позывной корреспондента
   ModePriorityEnum    priority;      // приоритет оппонента
   int                 modeBW;        // полоса пропускания
   ModeSwitchEnum      modeSwitch;    // наименование переключения
   ModeSyncExchEnum    modeTrans;     // уточнение режима (синхро, асинхро)
   ModeExchEnum        modeEx2;       // уточнение режима (симплекс, дуплекс, полудуплекс
   int                 modeChangeFreq; // режим переключения частоты РС (75,3)
   int                 modeRateSwitch;   // скорость переключения (200, 50)
   int                 rate;          // скорость (auto=0,1200,2400,4800, 9600,19200,38400,64000);
   ModeOffOnEnum       modeAdapt;     // адаптация по частоте
   ModeOffOnEnum       autoDiscon;    // авто разрыв соединения
   ModeYesNoEnum       haveInfo;      // есть ли информ.для передачи
   ModeNetConnectEnum  radioNetType;  // тип комбинированной сети
   int                 freeParam;     // предлагаемый канал передачи
   uint32_t            imitIns;       // имитовставка
   int                 factorNet;     // полоса пропускания
   int                 factorUser;    // полоса пропускания
   QVector<uint8_t>    syncPack;   // синхропакет
};
//-----------------------------------------------------------------
// структура данных для команды "netDISCONDCE"
//-----------------------------------------------------------------
struct NetDISCONParamStruct{
    int dce_id;
    int dce_id_corr;      // позывной корреспондента
};
//-----------------------------------------------------------------
// структура данных для команды "ExchangeDCE"
//------------------------------------------------------------------
struct ExchangeParamStruct{
    int dce_id;
    int dce_id_corr;
    int msg_id{1};
    QByteArray data_parity;
    QByteArray data_noparity;
    QByteArray data_general;
    int type;
};
//------------------------------------------------------------------
//
//------------------------------------------------------------------
#endif // STRUCT_LIST_H
