
#ifndef PROTOCOLRADIO_H
#define PROTOCOLRADIO_H

#include <QObject>
#include <complex>
#include "enum_list.h"
#include "struct_list.h"
#include "transportUDP.h"
#include "protocolRTP.h"
#include <QQueue>
#include <iostream>
#include <math.h>
#include <QTimer>
using namespace std;
//---------------------------------------------------------------------

//const int BUFF_SLOTS = 2;  // буфферизация слотов перед отправкой пакетов по udp

// Циклический буф.
template <class T>
class CQueue: public QQueue <T>{
private:
    int m_count;
public:
    inline CQueue(int cnt) : QQueue<T>(), m_count(cnt){

    }
    inline void enqueue(const T &t){    // метод добавления в очередь
        if (m_count==QQueue<T>::count()){
            QQueue<T>::dequeue();   // удаление старейшего элемента
        }
        QQueue<T>::enqueue(t);
    }
 /*   inline void dequeue(){    // метод удаления
            QQueue<T>::dequeue();   // удаление старейшего элемента

    }
    */
};
//-------------------------------------------------------------------
// Макросы для 24-х разрядной укладки отсчётов информационного сигнала
#define Q16_SIZE (4)
#define Q24_SIZE (6)
#define Q32_SIZE (8)
#define LOBYTE(w) ( (w)       & 0xFF)
#define HIBYTE(w) (((w) >> 8) & 0xFF)
#define MAKEDWORD(hiword, loword) ((((unsigned int)(hiword) & 0xFFFF) << 16)|(loword & 0xFFFF))

//Q_DECLARE_METATYPE(MsgDescrS)
Q_DECLARE_METATYPE(NetS)
Q_DECLARE_METATYPE(PackFieldS)

typedef  std::complex<short> short_complex;
typedef  std::complex<int> int_complex;
typedef  std::complex<float> f_complex;

#define M_2PI (6.283185307179586476925286766559f)
//------------------------------------------------------------------
struct BuffVal{
    int         seq_num {0};
    int_complex sig {0,0};
};
//-------------------------------------------------------------------
//--------------------------------------------------------------------
#define CBIFF_SIZE (28*200)
//#define CBIFF_SIZE (18*200)       //24пак.*200точек на пакет размер циклического буффера в принятых пакетах длит. 25 мс()
#define CPACK_COUNT_MIN (16)     // мин. кол-во пакетов в цикл. буф. для работы демодулятора
#define SLOT_SIZE (1600)          // размер слота в точках (IQ)
#define SLOT_WIDTH (200)          // для режима 5
#define BF_SHIFT ((SLOT_WIDTH * 1.6)/2) // сдвиг для BF

 class ProtocolRadio :public QObject
{
    Q_OBJECT
public:
    ProtocolRadio(NetS &net_adrs, int slot_size = 0 );

    ~ProtocolRadio();
    void sendIQToCorr(SlotFieldS &current_slot/*short_complex * signal, int len, int freq_num*/);     // отправка кодограммы корреспонденту
    void setBufShift(qint32 lf, qint32 rt); // установка полей m_shift_bf_left, m_shift_bf_right    
public slots:

    void slotDiscon();                               // завершение сеанса прослушки
    void slotGetIQFromCorr(QByteArray & buf, int num_pack);        // прочитан буф. из UDP сокета в transportUDP
    void slotStopServer();
    void slotUseBuffSlot(bool state);   // использоваине буфферизации потока слотов перед отправкой пакетов udp

private slots:

                           // удалить лог для перезаписи
    void slotPassTimerAlarm();  // обработка таймера отправки части слота в радиоканал

private:

    QVector<int_complex> *m_signal_IQ{nullptr};
    QQueue<SlotFieldS>   m_queue_slot;                // очередь слотов после модулятора для отправки
    bool                 m_slot_proc{true};
    bool                 m_slot_pass{false};          // флаг для начала отправки слотов с учетом буффериз.
    QQueue<PackFieldS>   m_queue_pack;                // очередь пакетов для отправки

    NetS                 m_net_adr;                   // адрес корреспондента
    TransportUDP        *m_transportUDP{nullptr};     // транспорт UDP
    CQueue<BuffVal>      m_cbuff{CBIFF_SIZE};         // циклический буф.
    int                  m_point_num{0};              // номер принятой точки комплекского графика
    QVector<QByteArray>  m_data_slot;                 // массив из элементов слота для отправки в радиоканал
    int                  m_curr_slot_part{0};         // номер текущей части отправляемого слота
    int                  m_time_part_slot_pass = 10;  // время передачи части слота в радио канал (mсек)
    quint8               m_freq_slot{0};              // частота на котором должен передаваться полученный от модулятора слот
    qint32               m_shift_bf_left{0};          // флаг сдива буф. перед демод. влево
    qint32               m_shift_bf_right{0};         // флаг сдива буф. перед демод. вправо
    int                  m_step_in{0};                // для частотного сдига сигнала перед демодул.
    int                  m_step_out{0};               // для частотного сдвига сигнала после модулятора
    QTimer               *m_timer_pack;               // таймер отправки пакетов в UDP
    bool                 m_format16{true};            // флаг для отправки 16 разрядных данных

    float                m_energy{0.0};              // энергия сигнала перед демодул
    int                  m_buff_slot{1};             // буфферизация исходящего потока слотов
    int                  m_num_log_p{0};             // номер записи в логе для отпра.пакетов
    int                  m_slot_size{0};             // размер слота модема

    void          nullingIQarray(QVector<int_complex> &null_data, // формирование массив для демодуляции (двойного размера)
                                 int num);   // количество добаляемых элементов (=0)
    void          nullingCBuff(CQueue<BuffVal> &buff, int num);
    void          denullingCBuff(CQueue<BuffVal> &buff, int num);
    quint16       conversion24to32(char* pBuff24, char* pBuff32, qint64 sBuff24, qint16 gain);
  //  quint16       conversion32to16(char* pBuff32, char* pBuff16, qint64 sBuff32, qint16 gain);
    qint64        conversion16to24(char* pInp, char* pOut, qint64 sizeByte);
  //  quint16       conversion16to32(char* pBuff16, char* pBuff32, qint64 sBuff16);
    void          createLog(CQueue<BuffVal> &buff, int k, int num);
    void          makePacksFromSlot(SlotFieldS & slot);
    void          queueSlotDequeue();
    void          createLog_time(const QString fl_name, int num, QString &title);    // log для контроля таймера
    bool          removeLog_time(const QString fl_name);

signals:
     void    sigIQFromCorr(QVector<int_complex>*);        // сигнал для CoreDCE с кодограммой полученной ОТ корреспондента
     void    sigIQEnergy(qreal);                          // подсчет энергии сигнала перед демодул.
     void    sigStopServOrDiscon();
     void    sigBindPort(int);                               // начать прослушку порта с номером
     void    sigErrLost(QString & err_journ);     // отправка статистики потерянных пакетов
     void    sigIQFromRadio(QByteArray & buf, int num_pack, int fl_pass = RX);  // для лога и журнала
     void    sigIQToRadio(QByteArray & buf, int num_pack, int fl_pass = TX);    // для лога и журнала     
     void    sigZpchChanged(int); // установка частоты в пакете UDP
};

#endif // PROTOCOLRADIO_H
