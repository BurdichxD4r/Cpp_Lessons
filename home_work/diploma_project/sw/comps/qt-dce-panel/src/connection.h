#ifndef CONNECTION_H
#define CONNECTION_H

#include "math_big.h"
#include <QObject>

#define SET_SA                          30
#define SET_FA                          63
#define ERR_SIZE	8
#define CODOGRAMM_LEN   22
#define CALL_FRAME_ID	                   0x00
#define CALL_CONFIRM_FRAME_ID    0x01
#define SOS_FRAME_ID                    0x02
#define SINCHRO_FRAME_ID             0x03
#define NO_SIGNAL_THRESHOLD      50
#define FIX_FREQ_NUMBER1             3
#define FIX_FREQ_NUMBER2             4
#define CONNECT_TIME_NRCH           16
#define CONNECT_TIME_NO_NRCH     4
#define BALL_TRANSFER_TIME           10
#define PASS_ADAPT_BALL 0x83


typedef union {
    /******************************************************************************
    *  Кодограммы запроса и подтверждения соединения
    *****************************************************************************/
    struct {
        quint8 id:             3 ;   // тип кодограммы (0-запрос 1-подтверждение)
        quint8 duplex:         1;   // 1- дуплекс, 0- полудуплекс
        quint8 fr_num_older:    4 ;   // 4 ст.бита номера кодограммы

        quint8 fr_num_minor ;                //8 младших бит номера кодограммы
        quint16 sender_address;     // адрес отправителя
        quint16 recipient_address;  // адрес получателя

        quint8 band:            2; //полоса 0-3100, 1-4500, 2-9000, 3-40000
        quint8 switch_mode:     2; // режим переключения 0-режим 1,1-режим 2,2-режим 3
        quint8 speed:           4;   // скорость передачи  0 - auto
                                     //                    1 - 600
                                     //                    2 - 1200
                                     //                    3 - 1800
                                     //                    4 - 2400
                                     //                    5 - 3600
                                     //                    6 - 4800
                                     //                    7 - 9600
                                     //                    8 - 19200
        quint8 speed_adaptation: 1; // 1-режим адаптации по скорости включен
        quint8 freq_adaptation: 1;  // 1 - режим адаптации по частотам включен
        quint8 break_enabled:   1;  // разрешает автоматическое завершение связи
        quint8 sinhro_mode:     1;  //1 - синхронный режим передачи
        quint8 auto_end:        1;  // 1 - разрешено завершение при отсутствии приема инфо
        quint8 combi_net:       1;  // 0 - КРС точка-точка, 1-КРС точка-многоточка
        quint8 have_info:       1;  //1 - наличие информации у абонента, 0 –отсутствие
        quint8 rs1:             1;  //0 - РС1(перестройка РЧ до 3 мс), 1 - РС2(до 75 мс)

        quint8  chan_number[2];           // предлагаемый номер канала (в дуплексе 2)
        quint32  fake_insert;              //  имитовставка
        quint64 fake_insert_sp;           // синхропакет имитовставки

    } call;
    /******************************************************************************
    *  Кодограммы сетевой синхронизации
    *****************************************************************************/
    struct {
        quint8 id:                           3 ;     // тип кодограммы (2-запрос 3-выдача)
        quint8 reserv:                    1 ;     // резерв
        quint8 fr_num_older:          4 ;     // 4 ст.бита номера кодограммы
        quint8 fr_num_minor ;                //8 младших бит номера кодограммы
        quint16 sender_address;            //  адрес отправителя
        quint16 recipient_address;        // адрес получателя
        qint32 counter;                       // счетчик базового звена
        quint32  fake_insert;              //  имитовставка
        quint64 fake_insert_sp;           // синхропакет имитовставки
    } sos;

    quint8 bytes[CODOGRAMM_LEN];
} frame_T;

typedef union {
    struct {
        quint8 id:             2 ;
        quint8 tspeed:           6;   // скорость при передаче
        quint8 rspeed;                // скорость в приеме
                                     //                    1 - 600
                                     //                    2 - 1200
                                     //                    3 - 1800
                                     //                    4 - 2400
                                     //                    5 - 3600
                                     //                    6 - 4800
                                     //                    7 - 9600
                                     //                    8 - 19200
    } speed_adapt;    // адаптация по скорости
    struct {
        quint8 id:              2 ;
        quint8 f_number:   6;   // Информация о замене 1-ой радиочастоты(номер новой частоты)
        quint8 f_shift;              // Дополнительный сдвиг 1-ой радиочастоты
    } add_shift;  // дополнительный сдвиг радиочастоты
    struct {
        quint8 id:                2 ;
        quint8 f_number1:   6;   // Информация о замене 1-ой радиочастоты(номер новой частоты или подтверждение старого)
        quint8 f_number2;         // Информация о замене 2-ой радиочастоты(номер новой частоты или подтверждение старого)
    } f_adapt;  // адаптация по радиочастоте
    struct {
        quint8 id: 2;
        quint8 cap1: 6;
        quint8 cap2;
     } connect_down;
    quint8 bytes[2];
} head_Info;

/*******************************************************************************
 *                        Возможные состояния аппаратуры
 ******************************************************************************/
enum STATES_ENUM {
    CALL_WAITING_STATE = 0,				// 0 - ожидание запроса соединения или синхронизации
    CALL_TRANSMISSION_STATE,			// 1 - передача запроса соединения
    CALL_RECEIVING_STATE,				// 2 - прием запроса соединения

    CALL_CONFIRM_WAITING_STATE,			// 3 - ожидание подтверждения соединения
    CALL_CONFIRM_TRANSMISSION_STATE,	// 4 - передача подтверждения соединения
    CALL_CONFIRM_RECEIVING_STATE,		// 5 - прием подтверждения соединения

    SOS_TRANSMISSION_STATE,				// 6 - передача запроса сетевой синхронизации
    SOS_RECEIVING_STATE,				// 7 - прием запроса сетевой синхронизации

    SINCHRO_WAITING_STATE,				// 8 - ожидание сетевой синхронизации
    SINCHRO_TRANSMISSION_STATE,			// 9 - передача сетевой синхронизации
    SINCHRO_RECEIVING_STATE,			// 10 - прием сетевой синхронизации

    DATA_RECEIVING_STATE,				// 11 - прием информации

    SEND_DISCONNECT_REQUEST,		            // 12 -Отправлен запрос на разъединение
    DISCONNECT_CONFIRM_WAITING_STATE, // 13 -  Режим ожидания подтверждения разъединения
    RECV_DISCONNECT_CONFIRM,		            // 14 - Прием подтверждения разъединения

    RECV_DISCONNECT_REQUEST,		            // 15 - 2 Прием запроса на разъединение
    SEND_DISCONNECT_CONFIRM,                // 16 - 4 Отправлено подтверждение разъединения

    ALIEN_SOS_RECEIVING_STATE,				// 17 - прием "чужого" запроса сетевой синхронизации
    ALIEN_SINCHRO_RECEIVING_STATE,			// 18 - прием чужой сетевой синхронизации
    ALIEN_CALL_RECEIVING_STATE,				    // 19 - прием "чужого" запроса соединения
    ALIEN_CALL_CONFIRM_RECEIVING_STATE,		// 20 - прием чужого подтверждения соединения

    CHANNEL_BREAKDOWN,                          // состояние КАНАЛ АВАРИЯ при отсутствии приема
    SPEED_DOWN_FALL,                               // падение на нижнюю скорость, если пропал канал на большой скорости
};

enum INFO_HEAD_ENUM {
    SPEED_ADAPTATION = 0,             // 0 - Адаптация по скорости
    ADDITIONAL_FREQUENCY_SHIFT,  //1- дополнительный сдвиг радиочастоты
    FREQUENCY_ADAPTATION,            //2- Адаптация по радиочастоте
    CONNECTION_BREAK,                  //3- Разрыв соединения
};

enum SPEED_ADAPT_ENUM {
    WAIT_EVENTS = 0,                           // 0 - ожидание событий
    STATISTICS_CALCULATION,             // 1 -расчет статистики декодирования
    SPEED_CHANGE_TRANSMIT,             //2- передача запроса на изменение скорости
    WAITING_ANSWER_CHANGE,            //3- ожидание ответа на смену скорости
    NEW_SPEED_STATISTICS_CALC,       // 4- расчет статистики на измененной скорости
    ROLLBACK,                                    //5- откат в случае плохой статистики на новой скорости
    PASSING_ACTIVITY_BALL,           // 6- передача хода
    NO_ADAPT,                      // адаптация выключена
};

enum STATES_ADAPT_ENUM {
    TX_SPEED_ANSWER = 0,         // переключение СКК в передаче в ответ на запрос удаленного модема
    //SPEED_DOWN,
    NONE_ACTION,
};

enum ADAPT_PHASE_ENUM {
    SPEED = 0,         // по скорости
    FREQUENCY,       // по частоте
    NONE,               //  адаптация отсутствует
};

enum COMMON_ADAPT_ENUM {
    WAITING_MODE = 0,                // дежурный режим
    SPEED_ADAPT_REQUEST ,         // 1 запрос на адаптацию по скорости
    WAIT_SPEED_ANSWER,                 //2  ожидание ответа на вкл.адаптации по скорости
    FREQUENCY_ADAPT_REQUEST,       //3  запрос на адаптациюпо частоте
    WAIT_FREQUENCY_ANSWER,          //4 ожидание ответа на вкл.адаптации по частоте
    NO_ADAPTATION,                        //5  адаптация отсутствует
    NO_ACTION,                              // 6
};

/*******************************************************************************
 *  Структура псевдослучайной последовательности
 ******************************************************************************/
typedef struct {
  uint128_t generic_pol;        // - образующий элемент поля
  uint128_t prime_pol;          // - примитивный полином (ключ ПСП)
  uint128_t shift_register;		// - состояние сдвигового регистра
} prs_T;

typedef struct {
    uint128_t	number;				// номер слота
    quint32	local;				// локальное время при обмене данными
    qint8		cntFrame;			// счетчик фрейма
} slot_T;

typedef struct {
    quint8 id;
    qint32 time;
} state_T;

typedef struct {
    quint8 id;
    quint8 txVar; // вариант СКК для передачи(0-28)
    quint8 rxVar; // вариант СКК для приема(0-28)
} state_aT;

/*******************************************************************************
 *  Структура хранящая времена, используемые при синхронизации
 ******************************************************************************/
typedef struct {
    qint32  out_counter[0xFFF];
    qint32  in_counter;
    qint32  in_now_counter;
    quint16  in_frame_num;
    qint32  in_min_err_offset;
} sincro_time_T;

typedef struct {
    //************************ Длительности (в слотах)**************************
    quint16 call_trans_time;          //  время передачи кодограммы установления соединения
    quint8 call_confirm_trans_time;  // время передачи кодограммы подтверждение соединения
    quint8 synchro_trans_time;       //  время передачи кодограммы синхронизации
    quint8 synchro_query_trans_time; //  время передачи кодограммы запроса синхронизации
    quint8 call_confirm_waiting_time;//  время ожидания кодограммы подтверждения соединения
    quint8 synchro_waiting_time;     //  время ожидания кодограммы синхронизации
    quint8 data_waiting_time;        //    макс. допуст. время отсутствия приема
    quint8  disconnect_confirm_waiting_time;   //  время ожидания кодограммы подтверждения на запрос разъединения
    quint8  disconnect_confirm_time;  //  время задержки отправки кодограммы подтверждение разъединения

    quint16 addressm;                // двухбайтовый адрес звена, младшие 4 бита
    quint16 synchro_shift;           // временное решение !!!!!!!!!!!!!!!
    uint128_t code_key;              // 128 битный ключ ПСП (16 байтов)
} s_reg_T;

typedef struct {
    int server_num; // номер модема
    quint8 band;  //полоса 0-3100, 1-4500, 2-9000, 3-40000
    quint16 speed; // скорость передачи 0- auto,600,1200,1800,2400,3600,4800,9600,19200
    quint8 switch_mode;  //режим переключения 0-режим 1,1-режим 2,2-режим 3
    quint8 rs1;         //0 - РС1(перестройка РЧ до 3 мс), 1 - РС2(до 75 мс)
    quint8  chan_number[2]; // предлагаемый номер канала (в дуплексе 2)
    quint8 fixFrequencies[3];   // номера частотных каналов  для ФРЧ(в дуплексе 2+1запас)
    quint8 fChannels[32];  // массив фактических номеров ЧК(от 0 до 127)
    quint8 txChan;
    quint8 rxChan;
    bool fixFrequencyMode;   // режим работы на фиксированной частоте
    bool wait_best_frequency_in_transmission;
    bool search_nrch_mode;    // состояние поиска наилучшей частоты
    quint8 duplex; // 1- дуплекс, 0- полудуплекс
    quint8 half_duplex_num; // номер для обеспечения обмена в полудуплексе
    bool simplex;  // признак работы в симплексе
    quint8 break_enabled;					// разрешение на разрыв соединения
    quint8 break_on_50;                         // разрешено завершение при отсутствии приема инфо
    quint8 repeat;   // повторение запроса синхр-ии для случая,когда номера частот модемов не пересекаются
    bool startup;   // признак модема, инициирующего соединение
    quint8 txVariant[32]; // индекс варианта СКК для передачи(0-28)
    quint8 rxVariant[32]; // индекс варианта СКК для приема(0-28)
    int slot_counter;
    bool adapt_speed_ball_is_mine; // у кого этот признак есть True, тот и проводит адаптацию
    bool logged;
    bool freq_adapt=false;
    bool speed_adapt=false;
    int adapt_current; // текущий тип адаптации (SPEED_ADAPTATION ,FREQUENCY_ADAPTATION,NO_ADAPT)
    bool change_adaptation_type;
    int cycle_before_switch_adapt;
    int adaptFreqIndex=0;  // индекс частоты адаптации для текущего слота
    int newRxVariant=0;   // номер устанавливаемого варианта в приеме
    int newTxVariant=0;   // номер устанавливаемого варианта в передаче
    int rxPspChannel=0;   // номер ПСП- канала в приеме(номер строки ЧВ-матрицы)
    int txPspChannel=0;   // номер ПСП- канала в передаче
    int startFreqNumber=16; // число стартовых частот режима ППРЧ
    int allFreqNumber=128; // общее число частот режима ППРЧ
    QList<int> futureSendIndexes1;
    QList<int> futureRecvIndexes1;
} connect_Pars_T;
Q_DECLARE_METATYPE(connect_Pars_T);

typedef struct {
    bool enable = false; // true - адаптация включена
    bool active = false;  // true - адаптация активна
} adapt_T;



#endif // CONNECTION_H
