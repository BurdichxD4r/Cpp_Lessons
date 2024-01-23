#ifndef TCONNECT_H
#define TCONNECT_H

#include "connection.h"
#include "clogger.h"
#include "math_big.h"
#include "adapt_speed.h"
#include "tadaptfreq.h"
#include <QObject>

class Tprs;
class PSRGen;

class TConnect : public QObject
{
    Q_OBJECT
public:
    explicit TConnect(QObject *parent = nullptr);
    ~TConnect();
    quint16 klient_address;
    quint16 server_address;

private:
    //******************* Параметры устанавления связи ***************************
      quint8  enabled;
      quint16  master;						// - инициатор обмена (ведущий/ведомый)

      quint8  hopping_mode;					// - режим переключения частот
      quint8  band_width;					// - полоса пропускания (3100Гц, 4500Гц, 9000Гц)
      quint16 address;						// - двухбайтовый адрес звена, с которым ведется
                                              // - обмен информацией.
                                              // - младшие 4 бита - номер канала с ЧВР
    //************* Параметры, устанавливаемые во время сеанса связи *************
      adapt_T freq_adaptation;     // управление адаптацией по частотам
      adapt_T speed_adaptation;     // управление адаптацией по скорости

      quint8 info_head_type;         // тип заголовка инфокадра
      quint8 info_head_par1;         // значение первого параметра заголовка
      quint8 info_head_par2;         // значение второго параметра заголовка
      int notDecodedSlots=0;   // число слотов в приеме без декодирования, подряд
      int breakConnectKdgNumber=0;  // число кодограмм подряд с командой BREAK_CONNECTION,  чтобы разорвать соединение
      int newFreqKdgNumber=0;  // число кодограмм подряд с новой частотой
      int newFreq=0;                     // новое значение номера частоты по адаптации
      bool slow_down_on_break=true; // разрешение попытки восстановить канал снижением скорости
     /*============================================================
      * , если в информационном кадре не поступало признака разрыва соединения,
      *   это значит идет прием информационного сообщения, при этом возможно 2 варианта:
                     - слоты декодируются, необходимо продолжить сеанс связи;
                     -слоты не декодируются, т.е. сеанс связи был прерван по каким либо причинам.
      В этом случае опять 2 варианта:
            •	если в кадре запроса соединения был признак разрешения автоматического
                завершения сеанса связи, то через 50 слотов  сеанс связи прерывается нештатно
                с формированием в ПАК АУ АСРЦ сообщений «сеанс связи завершён» и
                «канал авария» (при исправности аппаратуры);
            •	если в кадре запроса соединения признак разрешения автоматического
                завершения сеанса связи отсутствовал, то через 50 слотов  формируется
                сообщение «канал авария» в ПАК АУ АСРЦ (при исправности аппаратуры),
                сеанс вязи не прекращается и  сообщение «сеанс связи завершён» не формируется.
                (разъяснение Ю.С. от 11.08.2022)
      ===============================================================*/
      quint8 asinxro_mode;					//асинхронный режим работы

      quint16 data_transmit_time;			//длина фазы передачи данных во врмемя полудуплекса, в слотах
      quint16 data_receive_time;				//длина фазы приёма данных во врмемя полудуплекса, в слотах

    //******************* Статистика сеанса связи ********************************
      quint32 transmit_data_counter;			// счетчик объема передаваемой информации
      quint32 receive_data_counter;			// счетчик объема принимаемой информации

      qint8   nets_synchro;					// наличие сетевой синхронизации
      qint16  sinchro_offset;				// используются при установлении
      quint32 sinchro_offset_count;			// сетевой синхрнизации

    //******************* Другие параметры ********************************
      quint8			next_state;				//следующее состояние для режима полудуплекс
      state_T			state;					// Состояние модема
      state_T			duplex_state;			// Состояние модема для коннекта в дуплексе
      state_T			adapt_speed;			// Состояние адаптации по скорости
      state_T			common_adapt_state; // Состояние всей адаптации
      sincro_time_T	sincro_time;			// Хранится история времен по отправлению кадров

      prs_T*			prs_ptr;
      s_reg_T			s_reg;
      slot_T			slot;
      quint32		txShift;				// Сдвиг канала передачи
      quint32		rxShift;				// Сдвиг канала приема

      quint32		chan_num;				// Число каналов

      quint32		rsSum;					// Суммарная ошибка декодера Рида-Соломона
      quint32		txCode;					// Передаваемый код
      quint32		rxCode[4];				// Принимаемый код

      quint8			rsErrors[128][ERR_SIZE];// Массив ошибок декодера Рида-Соломона
      quint8			rxChannels[128];		// Каналы приемника
      quint8			txChannels[128];		// Каналы передатчика
      quint8			chDl[2];				// Линия задержки каналов

      bool is_kdg_to_transmit=false;  // модем находится в процессе изменения статуса соединения
      CLogger tcong;
      connect_Pars_T *parsm;
      bool mSending; // передача слота в режиме полудуплекса
      bool mRecieving; // прием слота в режиме полудуплекса
      PSRGen *generator;
      bool no_nrch; // признак того, что НРЧ  еще на рассчитывался
      TAdaptSpeed* ads=nullptr;
      MODEM_PROP* cprop;
      TAdaptFreq* adfreq=nullptr;
      int adaptation_for_init = SPEED;
      int freq_cycle_before_switch_to_speed=0;
      head_Info	capMemo; // заголовок с предыдущего слота
      int newSpeedKdgNumber=0;  // число кодограмм подряд с новой скоростью
      QList <int> *future_psp_numbers; // список FUTURE_PSP чисел ПСП на предстоящие  посылки

      int pspNumber=0;   // очередное значение генератора ПСП
      int slotCounter=0;
      Frequency_Stat *stat_reject;
      Speed_Stat *stat_result;
      bool marks[2][32];               // отметки на замену частот(0-передача и 1-прием)
      int cycle_counter=0;
      bool fcase = true;

      void sos_frame_create(void* data,  int ind);//создание кодограммы запроса сетевой синхронизации (клиент -> сервер)
      void sos_frame_analisys(frame_T *frame_ptr, int best_fr);//Функция анализа кодограммы запроса сетевой синхронизации (сервер)
      void sinchro_frame_create(void *data);//Функция создания кодограммы сет. синхр-ции (сервер -> клиент)
      void sinchro_frame_analisys(frame_T *frame_ptr,int best_fr); //Функция анализа кодограммы сетевой синхронизации (клиент)

      void call_frame_create(void* data);
      int call_frame_analisys(frame_T *frame_ptr); // анализ запроса на соединение
      void call_confirm_frame_create(void* data);
      int call_confirm_frame_analisys(frame_T *frame_ptr); // анализ ответа на запрос соединения
      void info_head_create(void* data, int htype,quint8 par1,quint8 par2);

      void controlSimplex();
      void controlHalfDuplex();
      void controlDuplex();
      void setDuplexMode(connect_Pars_T* pars);
      void logConnectParams(connect_Pars_T* pars);

      int freqIndex_send(int shift);
      int freqIndex_recv(int shift);
      void analysis_of_decoding_fixed(int sInd, int freqIndex1);  // для работы на фикс.частотах
      bool analysis_of_decoding_statistics(int sInd);  // определение параметров инфозаголовка посылаемого слота
                                                         // по данным статистик декодирования слотов
      bool speed_validation_to_replace_freq(int fIndex);  // если есть адаптация по скорости,то смена частоты только если скорость минимальна
      bool getEitherStatValid(int index, bool *good_chan);
      bool stats_for_send(int pos);
      void logSpeedSet(int curf);
      void logFrequencySet(int curf);
      void log_good(bool st);
      void freqsMarqing(bool state);
      void renewFreqSet(quint8 index, quint8 fNum);

      bool speed_stat_processing(int freqIndex, int sInd);
      void switchRxSpeed(int ch, int var);  // переключить кратность модуляции в приеме канала ch на var
      void switchTxSpeed(int ch, int var);  // переключить кратность модуляции в передаче канала ch на var
      void setBaseMode(int fnum);  // установка базового режима(минимальная скорость)на канале fnum

signals:
      void synchrStatus(QString msg);
      void synchrFailed(int num);
      void connectStatus(quint16 addr1,quint16 addr2,quint32 tChan,quint32 rChan); // индикация ЧВМ
      void connectMode(connect_Pars_T* pars);
      void calcNrch();  // сигнал для расчета НРЧ по собранной статистике
      //void setNewTxMode(int,int,int);
      void setWorkingVariant(int);
      void cbAdapt_f_turn(bool);
      void cbAdapt_s_turn(bool);
      void setNewTxSpeed(int ch, int var); // новую txSpeed по получению команды в инфозаголовке
      void setNewRxSpeed(int ch, int var); // новую rxSpeed по результатам сбора статистики

private slots:
    void  endOfConnection(QString status);
    void breakTheSpeed(QString status);
    bool speedAdaptElevated();

public slots:
    void start_synchro(connect_Pars_T *pars);
    void start_connect(connect_Pars_T* pars);
    bool generatingServiceKdg();
    bool tx_process(quint8* bytes, int ind);
    quint32 rx_process(char *bytes2, int bf);
    int tx_heading(quint8* hbytes, int chanIndex, int chanIndex1);
    void rx_heading(char *hbytes, QList<int> *head, QList<int> *mess);   //чтение заголовка инфопакета
    int state_refresh();
    quint32 getDoneProcent(QString &msg);

    void set_mdm_time(quint32 tm);   //установка tm как собств.времени модема
    void time_increment();
    void sendRecvControl();
    void init_mdm_time();   //нач.установка времени при включении модема
    uint128_t get_mdm_time();
    void setParams(int addr=0 , quint32 freqs_number=2, connect_Pars_T *pars=nullptr);
    void chan_refresh(int* chan, int chans_all, int shift=1);
    void chan_refresh1(int* chan);
    bool snd();
    bool rcv();
    void set_info_head_type(quint8 htype, quint8 par1, quint8 par2, quint8 id=0);
    void setConnectBreakdown(); // переход в состояние КАНАЛ АВАРИЯ
    void clearBreakdown();     // очистка счетчика отсутствия декодирования слотов
    void setRecvNrchFreqNumber(int num1, int num2);
    void setConnectTime(int num);
    void setFrchConnect();
    bool serviceKdg();
    void init_adapt_s(int st);
    void init_adapt_f(int st);
    bool ballIsMine();    // модем-инициатор адаптации по скорости
    bool newAdaptStep();    // попытка изменения скорости
    bool testPreviousChange();   // тест предыдущего изменения скорости
    void setAdaptState(quint8 htype);
    void setAdapt(TAdaptSpeed* adapt=nullptr);
    void addAdaptStat(int err, int max, int ind, int ch);
    void setSlowDownOnBreak(bool state);
    void setModemProp(MODEM_PROP* pr);
    void setLoop_fixFrequecyMode(bool nrch);
    void setAdaptFreq(TAdaptFreq* adapt=nullptr);
    void msg_to_Status();
    void setDataMode(bool st);

};

#endif // TCONNECT_H
