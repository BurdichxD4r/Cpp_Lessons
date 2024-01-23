#ifndef MODEMPANEL_H
#define MODEMPANEL_H

#include <QtGui>
#include <QtWidgets>
#include <QDialog>
#include "fastfourier.h"

enum FrequencyBand // полоса частот, Гц
{
    fBand_3100=3100,
    fBand_4500=4500,
    fBand_9000=9000
};

typedef struct {   // Описание варианта сигнальной конструкции слота
  int  num;					//  номер варианта
  QString freq_mode;    // режим работы по частоте
  int f_change_time;      // смена РЧ, мс ( -1, если смены нет)
  int slot_time;              // длительность слота, мс
  int  freq_band;				// полоса частот
  int  freq_discr;				// частота дискретизации
  QString mode_type;    // тип модуляции
  int  rate;						// кратность модуляции
  int  pack_number;		// количество посылок в слоте
  int  subchan_number;		// количество подканалов в посылке
  int serv_bits;              // число служебных бит в слоте
  int info_bits;               // число информационных бит в слоте
  int N;                         // длина кодового слова, байт
  int K;                          // длина сообщения, байт
  int duplex;                  // режим обмена
  int stage;                   // этап разработки(очередность)
  int symbol_bits;         // число бит в символе
  int section;                // номер секции в пределах которой идет адаптация по скорости
} SLOT_VARIANT;

typedef struct {   // Задание параметров канального уровня
  bool data_exchange;    // true, если modulate() запускается в режиме обмена данными
  bool snd;                    // true, чтобы формировать слот независимо от режима дуплекс/полудуплекс
} MODEM_PROP;

QT_BEGIN_NAMESPACE
class DceSettings;
class TStream;
class TDataFile;
QT_END_NAMESPACE

class ModemPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ModemPanel(QWidget *parent = nullptr);
    ~ModemPanel();

    bool isActive = false;
    int Number;  // номер модема

    void setModemNumber(int num); // присваивает модему номер
    int   getModemNumber();    // возвращает номер модема

    void setChannelLevel(bool state);  // установка модема для работы на канальном уровне
    void setSamplingRate(int vars,int sampling);   // установка частоты дискретизации sampling для варианта vars
    void setPanelVariant(int vars);      // установка варианта СКК
    /*
     * Внимание!!!
     * Метод setPanelVariant()
     * должен вызываться
     * после вызова методов setChannelLevel() и setSamplingRate()
     */

    bool modulate();                           // модулятор
    short_complex* getOutput();       // возвращает ссылку на слот, сформированный модулятором
    int getSlotLength();                    // возвращает длину слота(размер массива)
    int chanNumber_send();           // возвращает номер ЧК для отправки слота
    int chanNumber_recv();            // возвращает номер ЧК получения слота
    bool getChanExit(QVector<int_complex>*signal);   // получает указатель на входной массив демодулятора
    void demodulate(bool rcv);           // демодулятор

    QByteArray* slotExitData(int* rs, int* bf ); // возвращает декодированное сообщение слота
              // а также rs-число исправленных ошибок Ридом-Соломоном и bf-оптимальный сдвиг
    QList <fft_complex>* getSlotSpectr(); // возвращает массив точек амлитудного спектра
    float getSlotSNR();  // возвращает вычисленное демодулятором значение SNR
    int getSlotMsgLen(); // возвращает длину сообщения,передаваемого в одном слоте(в байтах)
    int getSkkNumber(); // возвращает номер варианта сигнально-кодовой конструкции(СКК)

    void setRandomReference(bool rnd);  // уст-ка режима случайных кодов в опорном пакете(false - no rand())

    void set_psp_clock(quint32 tm);   //установка tm для генератора ПСП
    int get_psp_channel(int t_go,int psp); // возвращает номер ЧК (t_go = t_current-tm, psp - номер ПСП канала)
    /*
     * Пример использования переключения частот от ПСП:
     *  setActiveFrNum("16");        - установить число ЧК
     *  set_psp_clock( tm); - установить время в генераторе ПСП( tm = время в мсек / 50)
     *
     *  цикл по слотам
     * {
     *      Номер канала = get_psp_channel(t_current-tm, psp); // t_current - тек.время в мсек/50
     *                                                                                         psp - номер ПСП канала
     * }
     *
    */
    void setModemReset(); // сброс параметров

private:

    QTabWidget *tabWidget;
    DceSettings *settings;
    TStream *streams;
    TDataFile *files;
    QGroupBox *modemGroupBox;

    QByteArray msg;
    MODEM_PROP *mprops;
    int initialTime=0;
    int lastSlotTime=0;
    void setTexts();
    void time_to_psp(int t_go);   // значение ПСП для момента t_go(число мсек с момента tm, деленное на длину слота в мсек)

signals:
    void setPause(bool mode);
    void setDataOutput(bool mode);
    void setCycle(int index);
    void  to_work_the_channel(short_complex* inChan);
    void pass_over_variant(SLOT_VARIANT *vars);
    void setPauseFlag(bool mode);
    void changeSnr(int index);
    void passSnrMetric(const QString& str);
    void transmitConnectState(int addr1,int addr2);
    void loop_on_off(int);
    void allFreqsNumber(int num);  // изменение числа доступных для ППРЧ частот

private slots:
    void setActiveSign(bool state);

public slots:

    void set_turnOn_State(bool state);      // вкл/выкл модема
    void set_turnOn_Plots(bool state);      // вкл/выкл отображения графиков
    void setFrchFrequencies(int f1,int f2); // задание частот для режима ФРЧ
    void setPspChannels(int txCh,int rxCh); // задание PSP-каналов для направления(передача и прием)

    void define_modem_properties(MODEM_PROP *mProp);  // установка параметров
    void send_data(char *data, int length);   // посылка сообщения

    void changeTabWidget();
    void getChanExitTest(QVector<int_complex>*signal);
    void setPauseThread(bool mode);
    bool fromFile();
    int getAllFrequenciesNumber();  // возвращает число доступных для ППРЧ частот
    void setAllFrNum(const QString allF);
    void setActiveFrNum(const QString freq);

protected:
    void changeEvent(QEvent *event);

};

#endif // MODEMPANEL_H
