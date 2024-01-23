#ifndef ENUMS_LIST_H
#define ENUMS_LIST_H

/*const int TX = 1;   // передача
const int RX = 0;   // прием
//--------------------------------------------------------------------------------------
enum NetRoleEnum{  // роль в сети
    SERVER = 0, CLIENT
};
//-------------------------------------------------------------------------------------
enum TypeMsgEnum{  // формат сообщения
    JSON = 0, XML
};
//--------------------------------------------------------------------------------------
enum ConnectTypeEnum{  // тип протокола
    TCP = 0, UDP
};

//--------------------------------------------------------------------------------------
enum ClientTypeEnum{  // тип клиента в сети
    MAS = 0,
    AU,
    UVS,
    UNKN
};
//-------------------------------------------------------------------------------------
enum ModeEnum{ // тип для задания режима оброда JSON объекта
    CHECK = 0,     // проверка
    FILL_SAME       // заполнение совпадающих ключей
};
//-------------------------------------------------------------------------------------
enum PACStatusEnum{// тип для задания состояния в котором находится объект УВС, АУ, МАС
    INIT = 0,
    ENABLE,    
    NET_SYNC,
    NET_CONNECT,
    EXCH,
    NET_DISC
};
//-------------------------------------------------------------------------------------
enum ExchangeModeEnum{ // тип для задания состояния ПАК МАС (тест на поврежд. символы или обмен информ.)
    TEST_EXCH = 0,
    INFO_EXCH
};
//-----------------------------------------------------------------------------------
*/
//-------------------------------------------------------------------------------------
/*enum MarkTypeEnum{ // тип метки для задания состояния в котором находится объект УВС в данный момент
    WARING = 0,
    DAMAGE,
    CORRECT,
    INFORM
};
*/
enum ModeSwitchEnum{
    FRCH = 0,
    NRCH,
    PPRCH
};
enum ModeExchEnum{
    HALF_DUPLEX = 0,
    DUPLEX,
    SIMPLEX
};
enum ModeOffOnEnum{
    OFF = 0,
    ON
};
enum ModeYesNoEnum{
    NO = 0,
    YES
};
enum ModeNetConnectEnum{
    POINT = 0,
    MULTIPOINT
};
enum ModeSyncExchEnum{
    SYNC = 0,
    ASYNC
};
enum ModePriorityEnum{
    M=0, V, R, C, O
};

enum StageMarkerButEnum{
    PING = 0, VER, STATUS, ENBL, DCE_INFO,
    DIR_INFO,DISBL,  SYNCH, CONNECT_START,
    CONNECT_END, USER_BUSY, NET_BUSY,ERROR,
    DISCON,EXCH_DATA, REPLY_ENQ, REPLY_END
};

#endif // ENUM_LIST_H
