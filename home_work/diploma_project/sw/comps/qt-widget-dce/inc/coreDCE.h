#ifndef COREDCE_H
#define COREDCE_H

#include <QObject>
#include <QWidget>
#include <QJsonObject>
#include <QMap>
#include "enum_list.h"
#include "struct_list.h"
#include <QQueue>
#include "QTimer"
#include "modempanel.h"
#include <QTimer>
#include "logmaker.h"

Q_DECLARE_METATYPE(SlotFieldS)

typedef  std::complex<short> short_complex;
typedef  std::complex<int> int_complex;

// предельные значения и допустимый интервал для BF
const   qint32 BF_LIMIT_MAX = 319; //для 5 режима работы
const   qint32 BF_LIMIT_MIN = 0;
const   qint32 BF_GOOD = 160;
const   qint32 BF_DELTA = 10;      // защитный интервал (20 точек сигнала)

class CoreDCE :public ModemPanel/*QObject*/
{
     Q_OBJECT
public:
    CoreDCE(QWidget *parent = nullptr/*NetS &net_adrs,
            TypeMsgEnum type_msg = JSON,      // тип сообщения
            bool format_msg = false           // формат сообщение
            */);
    ~CoreDCE();

     void     setRatePass(int rate);         // установка скорости передачи
     void     setQueueDepth(int depth);      // установка глубины очереди     
     void     setSetID_DST(int id_dst);      // установка dce_id_dst

public slots:        

  //  void      slotCmdFromClient(MsgDescrS & msg, int cmd_type);  //!! из ProtocolServer c командой клиента
    void      slotTempExchDCE(MsgDescrS & msg);                  //!! временый слот иммитирует работу модема
    void      slotIQFromCorr(QVector<int_complex> *signal);    // !! IQ из радиоканала от корреспондента
    void      slotDisconnect();
    void      slotUseDST(bool);   // использование в ком "ExchangeDCE" полей DCE_ID_DST

private slots:
    void      slotPassTimerAlarm();  // обработка таймера отправки слота в радиоканал (сервер обмена)
    void      slotSlotNumPass(qint32 num);
//  void      slotGetTimerAlarm();   // обработка таймера приема слота из радиоканала (сервер обмена)

private:    

    int                      m_mode_modem;                // номер режима работы модема
    int                      m_dce_id;                    // номер модема
    int                      m_ppf_ind[4];                // тек. инд. ЗПЧ для 4-х каналов
    int                      m_ppf_num;                   // кол-во ЗПЧ для переключения
    int                      m_sampl_rate;                // частота дискретизации



    QString                  m_err;                       //  для  хранения ошибок обработки сообщений json  
   // UVSParamS                m_uvs_obj;                   // сервер-УВС. Состояние УВС (структура задается выше)
    QQueue<SlotFieldS>       m_queue_slot;                // очередь слотов после модулятора для отправки
    QQueue<MsgDescrS>        m_queue_msg;                 // очередь сообщений для отправки
    QQueue<int>              m_queue_slot_count;          // очередь размерности сообщиния в слотах
    bool                     m_msg_pass{true};            // флаг для сигнализирования отправки очередного пакета
    int                      m_curr_slot_num{0};          // номер слота передаваемого в тек. момент (сообщение разбито на слоты)
    int                      m_slot_count{0};             // количество слотов, на которое должно быть разбито передаваемое сообщешние
    int                      m_time_slot_pass = 200;      // период запуска модулятора для обработки сообщения (mсек)
   // QTimer                   *m_slot_timer_pass;          // таймер передачи слота в радио канал
    MsgDescrS                m_current_msg;               // текущее сообщение для отправки клиенту
   // int                      m_time_slot_get = 1000;      // время приема слота(json объекта) из радио канал (mсек)
  //  QTimer                   *m_slot_timer_get;           // таймер приема слота из радио канал
 //   int                       m_com_wr[4]{0,0,0,0};     // счетчик на записи сообщения в Радио Канал.(m_com_wr[0] ~ NetSYNC, m_com_wr[1] ~ NetCONNECT,m_com_wr[2] ~ NetDISCON, m_com_wr[3] ~ ExchangeDCE)
    int                       m_rate;                     // скорость передачи информ (бит/сек)
    int                       m_queue_depth{1};           // глубина очереди информ. сообщений от ПАК МАС
    bool                      m_use_dst{true};
                // обработка значения bf
    bool                      m_flag_bf_use{false};       // флаг оценки bf (выполняется однократно)
    quint32                   m_flag_shift_left{0};   // cдвиг буф. для демод. влево
    quint32                   m_flag_shift_right{0};   // cдвиг буф. для демод. вправо
    QTimer                   *m_slot_timer;           // таймер отправки слота в радио канал
   // int                       m_num_log_m{0};         // номер записи в логе для модул.
  //  int                       m_num_log_d{0};         // номер записи в логе для демодул.
    QFile                     m_log_file;             // лог файл

    LogMaker        *m_log_maker_b;      // логирование слотов перед демод.
    LogMaker        *m_log_maker_a;      // логирование слотов после демод.
    int             m_count_b{0};       // счетчик сообщений для лога
    int             m_count_a{0};       // счетчик сообщений для лога

    void    addToLog_b(int k,  QVector<int_complex> *signal, QDateTime tm, LogUseEnum flag_use);
    void    addToLog_a(int k,  QByteArray & ba, QDateTime tm, LogUseEnum flag_use);
    void   createLog_time(const QString fl_name, int num, QString &title);    // log для контроля таймера
    bool   removeLog_time(const QString fl_name);                           // удалить лог для перезаписи
  /*  void   cmdStatusProc(MsgDescrS & msg);
    void   cmdVerionProc(MsgDescrS & msg);
    void   cmdGetDCEInfoProc(MsgDescrS & msg);
    void   cmdGetRadioDirInfoProc(MsgDescrS & msg);
    void   cmdEnableDCEProc(MsgDescrS & msg);
    void   cmdDisableDCEProc(MsgDescrS & msg);
    void   cmdNetworkSYNCProc(MsgDescrS & msg);
    void   cmdNetworkCONNECTProc(MsgDescrS & msg);
    void   cmdNetworkDISCONProc(MsgDescrS & msg);
    void   cmdExchangeDCEProc(MsgDescrS & msg);
    void   cmdReplyDCEProc(MsgDescrS & msg);

    void   cmdVerionAnsw(MsgDescrS & msg);
    void   cmdStatusAnsw(MsgDescrS & msg);
    void   cmdGetDCEInfoAnsw(MsgDescrS & msg);
    void   cmdGetRadioDirInfoAnsw(MsgDescrS & msg);
    void   cmdEnableDCEAnsw(MsgDescrS & msg);
    void   cmdDisableDCEAnsw(MsgDescrS & msg);
    void   cmdNetworkSYNCAnsw(MsgDescrS & msg);
    void   cmdNetworkCONNECTAnsw(MsgDescrS & msg);
    void   cmdNetworkDISCONAnsw(MsgDescrS & msg);
    void   cmdExchangeDCEAnsw(QByteArray data_exch);
    void   cmdReplyDCEAnsw(MsgDescrS & msg);
    */
    int         decodingGettingObject(QJsonObject json_in);    // декодирование принятого от клиента сообщения
    int         decodingDataLength(QJsonValue val);           // размер передаваемых данных после декодирования
    QByteArray  decodingData(QJsonObject json_in);            

    void        processObject(QJsonObject & obj_in,        // обработка объектов команд, входящ.(принятая послед.) и эталонной из m_com_list
                              QJsonObject & obj_sampl,
                              ModeEnum mode);
    void        queueMsgDequeue();                         // процедура опустошения очереди сообщений
    void        queueSlotDequeue();                         // процедура опустошения очереди сообщений
    void        modulatorStart(QByteArray & ba);           // процедура модуляции переданного сообщения
   // void        prepareEndAnsw(MsgDescrS & msg);
  //  void        prepareStartAnsw(MsgDescrS & msg);
  //  void        prepareEnqueuedAnsw(MsgDescrS & msg);

    qint32      getMsgSize(QString &data_field);
    QByteArray  getDecodingMsg(QString &data_field);
    void        logFileClose();
    void        logFileOpen(QString file_name);
    void        logFileWrite(int num, QString &title);

signals:
    //////   Сигналы для ModemPanel о пришедших командах от клиента
  /*  void    sigEnableDCE(MsgDescrS & msg);
    void    sigDisableDCE(MsgDescrS & msg);
    void    sigSYNC(MsgDescrS & msg);
    void    sigCONECT(MsgDescrS & msg);
    void    sigDISCON(MsgDescrS & msg);
    void    sigExchDCE(MsgDescrS & msg);
    */

    void    sigEnqueueAnsw(MsgDescrS & msg, int msg_in_que);
    void    sigStartAnsw(MsgDescrS & msg);
    void    sigEndAnsw(MsgDescrS & msg, int msg_in_que);
    void    sigExchDCEAnsw(QByteArray data_corr);
   ///////////////////////////////////////////////////////////////
    //void    sigCmdToClient(MsgDescrS & msg, QJsonObject & obj_param);  // сиг. в ProtocolServ о готовности данных для формирования ответа клинету
   // void    sigCmdToClientFromRadio(QString cmd_name, QJsonObject &param_list_out); // инициативное сообщение из радио канала
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void    sigIQSignalReady(SlotFieldS & slot );  // IQ сигнал для отправки в радио канал (должен быть в ModemPanel)
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void    sigBfShift(qint32 lf_shift, qint32 rt_shift);      // сигнал в "ProtocolRadio" (через Widget-Exch-IQ)о сдвиге буф. для коррекции bf
    void    sigSlotNumPass(qint32);     // номер отправленного слота для контроля конца отправки сообщения
    void    sigIQEnergy(qreal energy);
    void    sigStartBuff(bool);
};

#endif // COREDCE_H
