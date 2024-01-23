#ifndef SLOT_PARAMS_H
#define SLOT_PARAMS_H
#include <QString>
#include "eml.h"
#include "modempanel.h"

#define FFTDIM_3100 80
#define FFTDIM_4500 96
#define FFTDIM_12800 128
#define FFTDIM_9000 192
#define MAX_CHANNELS_NUMBER 89
#define VARS_NUMBER 29

//BER
const int ber_measure = 100000;
const float zero_fraction = 0.3;   // обнуляемая часть опорной посылки(для перестройки РЧ)
#define ZEROING      0    // 1 - обнуление  начала опорной посылки, 0 - без обнуления
#define SHIFTINGS    0    // 1- кривые BER со смещением от 0 до защ.интервала , 0 - без смещения
#define RS_CODE      1   // 1-с кодом Рида-Соломона, 0 - нет

enum ADAPT_ACTION_ENUM {
    THE_SAME_SPEED = 0,   //   без изменений
    SPEED_UP =1,               //    повышение скорости
    SPEED_DOWN = 2,        //    понижение скорости
    SPEED_DROP = 3,        //    сброс к базовому режиму
    REPLACE_FREQ = 4,                  // замена частоты в ППРЧ
    DOWN_TO_SILENT_MODE = 5,  // переход в деж.режим для НРЧ
    CHANNEL_CRASH = 6,   // канал авария - разрыв соединения в ФРЧ
};

const float radiansToGerz = 12.732395447351628;  // коэффициент перевода доп.набега фазы в сдвиг частоты
const int FUTURE_PSP = 16;

enum FrequencyDiscr // частота дискретизации, Гц
{
    fd_80=8000,
    fd_96=9600,
    fd_128=12800,
    fd_192=19200
};

enum SlotTime  // длительность слота, мс
{
    slot50=50,
    slot200=200
};

enum package_number  // количество пакетов в слоте
{
   packs_3 = 3,
   packs_9 = 9,
   packs_15 = 15,
   packs_16 = 16
};

enum subchan_number  // количество подканалов в посылке
{
   sub_29 = 29,
   sub_44 = 44,
   sub_89 = 89
};

enum service_bits  // количество сервисных бит в слоте
{
   sbits = 16,
   sbits28 = 173,
   sbits29 = 53
};

enum inform_bits  // количество информационных бит в слоте
{
   ibits_0 = 0,
   ibits_060 = 60,
   ibits_120 = 120,
   ibits_240 = 240,
   ibits_360 = 360,
   ibits_480 = 480,
   ibits_720 = 720,
   ibits_960 = 960
};

enum exchange_mode  // режим обмена
{
    duplex_and_half = 1,
    serv_package = 2,
    simplex = 3
};

typedef	struct	 Modem_stream_s
{
    quint32 send;   // отправлено бит
    quint32 recv;   // принято бит
    quint32 error;   // количество ошибок(бит) в приеме
    quint32 byte_error;   // количество ошибок(байт) в приеме
    quint32 speed_send;   // скорость в передаче, бит/сек
    quint32 speed_recv;   // скорость в приеме, бит/сек
    QString vartext;         // описание варианта сигнальной конструкции
    int varnum;              // номер варианта
    int snr;                    // индекс SNR(0-30)
    int rxSync;              // счетчик синхронизации
    quint8 rxCnt;               // счетчик байтов
}	Modem_stream;

typedef	struct	 Quality_Stat_s
{
    int indf;                     // номер индекса массива активных частот
    int freqNum;             // номер частоты
    int number;                // количество отсчетов
    float summa_snr;       // сумма SNR по всем отсчетам
    int  summa_rs;          // сумма ошибок RS по всем отсчетам
    int not_decoded;       // число слотов с ошибкой RS > порога
    float snr;                   // среднее значение  SNR
    float rs;                    // среднее значение ошибок RS
    float nodec;              // cр.значение not_decoded
//    float delta_rs;           // cр.кв.отклонение ошибок RS
}	Quality_Stat;

typedef	struct	 Frequency_Stat_s
{
    int indf;                     // номер индекса массива активных частот
    int freqNum;             // номер частоты
    int number;                // количество отсчетов
    float summa_snr;       // сумма SNR по всем отсчетам
    int  summa_rs;          // сумма ошибок RS по всем отсчетам
    int  summa_mx;          // сумма макс.ошибок RS по всем отсчетам
    int not_decoded;       // число слотов с ошибкой RS > порога
    float snr;                   // среднее значение  SNR
    float rs;                    // среднее значение ошибок RS
    float nodec;              // cр.значение not_decoded
    int rsMax;                // максимум исправляемых РС ошибок
    bool allow_reject = false;      // разрешение на смену частоты от speed_adapt
    bool reject = false;
    QList <int> rslist;     // список числа ошибок RS для браковки ЧК
    QList <int> mxlist;     // список максим.числа исправляемых ошибок
    QList <int> rsgood;   // список числа ошибок RS для  определения good channels
}	Frequency_Stat;

typedef	struct	 Speed_Stat_s
{
    int freqNum;             // номер частоты
    int  summa_rs;          // сумма ошибок RS по всем отсчетам
    int  summa_mx;          // сумма макс.ошибок RS по всем отсчетам
    int not_decoded;       // число слотов с ошибкой RS > порога
    int rsMax;                // максимум исправляемых РС ошибок
    int speed_advise;     // рекомендуемая скорость
    bool allow_reject = false;      // разрешение на смену частоты от speed_adapt
    bool reject = false;   // смена частоты
    QList <int> rslist;     // список числа ошибок RS для браковки ЧК
    QList <int> mxlist;     // список максим.числа исправляемых ошибок
    QList <int> rsgood;   // список числа ошибок RS для  определения good channels
}	Speed_Stat;


const int testCodes[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; // массив канальных символов для форм-ния тестовых последовательностей
const float SectWidthRec[] = {pi,pi/2,pi/4,pi/6};
const float Delta	= 0.23561944901923449288469825374596f*0.5;//(<-> 1.5 Гц)
// 100*2*pi*3/8000 == 120*2*pi*3/9600 == 240*2*pi*3/19200 -> 13.5 градусов
const float SNR_a			= -4.215792913f;
const float SNR_b			= 0.0f;

// сигнальные созвездия для 4-х типов модуляции соответственно
const float cs1 = 0.70710678118654752440084436210485f;           // cos(45°)
const float cs2 = 0.86602540378443864676372317075294f;           // cos(30°)

const fft_complex ofm1[] = {fft_complex(1.0f,0.0f),fft_complex(-1.0f,0.0f)};
const fft_complex ofm2[] = {fft_complex(1.0f,0.0f),fft_complex(0.0f,1.0f),
                                          fft_complex(0.0f,-1.0f),fft_complex(-1.0f,0.0f)};

const float cos3[] = {cs1,1.0f,0.0f,-cs1, 0.0f, cs1,-cs1,-1.0f};          // ОФМ-3
const float sin3[] =  {cs1,0.0f,1.0f, cs1,-1.0f,-cs1,-cs1, 0.0f};
const float cos4[] = {cs2, 0.5f, 1.0f, 0.26f,-0.5f,0.0f,-cs2,-0.26f, 0.5f, 0.0f, cs2,  0.26f, -cs2, -0.5f,-1.0f,-0.26f};    // АФМ-4
const float sin4[] =  {0.5f,cs2,  0.0f, 0.26f, cs2, 1.0f,0.5f, 0.26f, -cs2,-1.0f,-0.5f,-0.26f,-0.5f, -cs2,  0.0f,-0.26f};

const float one_parcel_time = 12.5;

const int	Errors[] =
{
    0,	1,	1,	2,	1,	2,	2,	3,	1,	2,	2,	3,	2,	3,	3,	4,
    1,	2,	2,	3,	2,	3,	3,	4,	2,	3,	3,	4,	3,	4,	4,	5,
    1,	2,	2,	3,	2,	3,	3,	4,	2,	3,	3,	4,	3,	4,	4,	5,
    2,	3,	3,	4,	3,	4,	4,	5,	3,	4,	4,	5,	4,	5,	5,	6,
    1,	2,	2,	3,	2,	3,	3,	4,	2,	3,	3,	4,	3,	4,	4,	5,
    2,	3,	3,	4,	3,	4,	4,	5,	3,	4,	4,	5,	4,	5,	5,	6,
    2,	3,	3,	4,	3,	4,	4,	5,	3,	4,	4,	5,	4,	5,	5,	6,
    3,	4,	4,	5,	4,	5,	5,	6,	4,	5,	5,	6,	5,	6,	6,	7,
    1,	2,	2,	3,	2,	3,	3,	4,	2,	3,	3,	4,	3,	4,	4,	5,
    2,	3,	3,	4,	3,	4,	4,	5,	3,	4,	4,	5,	4,	5,	5,	6,
    2,	3,	3,	4,	3,	4,	4,	5,	3,	4,	4,	5,	4,	5,	5,	6,
    3,	4,	4,	5,	4,	5,	5,	6,	4,	5,	5,	6,	5,	6,	6,	7,
    2,	3,	3,	4,	3,	4,	4,	5,	3,	4,	4,	5,	4,	5,	5,	6,
    3,	4,	4,	5,	4,	5,	5,	6,	4,	5,	5,	6,	5,	6,	6,	7,
    3,	4,	4,	5,	4,	5,	5,	6,	4,	5,	5,	6,	5,	6,	6,	7,
    4,	5,	5,	6,	5,	6,	6,	7,	5,	6,	6,	7,	6,	7,	7,	8,
};

const int ScramblerBuf[] =
{
    0x99,0xa8,0x57,0x6c,0x11,0x2a,0xc,0xac,0x4,0x3d,0xfd,0x2c,0xa1,0xae,0x5f,0xb3,
    0xce,0xae,0x32,0x1e,0x5d,0xad,0xf8,0x87,0xb4,0xe5,0xaa,0x96,0xb,0xae,0x60,0xe,
    0xed,0xe9,0x53,0x1,0xc,0x94,0x90,0x8f,0xbf,0x3b,0xde,0x85,0xd,0xd6,0x8c,0xac,
    0x1c,0x8,0x3e,0x59,0x83,0xa1,0x55,0xbf,0xdf,0xf4,0x25,0x48,0x30,0x93,0xac,0x5d,
    0x39,0x78,0x2d,0xe8,0xaf,0xcd,0x96,0x2a,0x85,0x76,0xa4,0x91,0x1,0x16,0xf0,0x23,
    0xae,0xd3,0x3d,0xcc,0x88,0x5d,0x61,0xcb,0xb9,0x96,0x3d,0xfa,0xaf,0,0x48,0xc4,
    0xa2,0xf9,0xf7,0xc3,0x17,0x59,0xe8,0xb6,0x2f,0xdd,0xcb,0x30,0xf5,0x65,0x5a,0x84,
    0xb5,0xfe,0x2a,0xf0,0xaf,0x8c,0xb5,0xd4,0xaf,0xd9,0x51,0x19,0x66,0x79,0x4a,0x88,
    0x93,0x3,0x2c,0x6,0xaf,0x84,0x4f,0x1a,0x40,0x8d,0xbe,0xcd,0xf1,0x22,0xd0,0x39,
    0xe6,0x74,0x1b,0xe,0xe1,0xcb,0x37,0xff,0x85,0x8b,0xdc,0xb8,0x70,0xe,0x5,0x68,
    0x10,0xe2,0x49,0xb1,0xdf,0x13,0x33,0x78,0x6b,0xa7,0x75,0xe3,0x3c,0,0x82,0xb0,
    0xcc,0x82,0x6f,0x38,0x29,0xe8,0x20,0xd3,0xf5,0x59,0x6,0x70,0x36,0x93,0x6a,0xa0,
    0x6f,0xc8,0x3b,0xb7,0xc4,0x8d,0x52,0x1f,0x9e,0x8f,0x96,0x64,0x85,0x98,0x68,0x17,
    0x94,0x9a,0xd6,0x57,0x73,0xe9,0xf,0x9,0x2b,0x33,0x67,0x26,0x6,0xfc,0xa7,0x5f,
    0x8e,0xa2,0xb2,0xc4,0x72,0x30,0x11,0xe6,0x72,0xe2,0xf0,0x51,0x58,0x9d,0x6a,0x73,
    0x21,0x7d,0x1c,0x32,0xac,0x6a,0xa7,0x6,0x35,0x35,0x48,0xe8,0xfa,0x6f,0xde,0xf,
};

#endif // SLOT_PARAMS_H
