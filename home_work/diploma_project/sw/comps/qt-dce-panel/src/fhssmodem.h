#ifndef FHSSMODEM_H
#define FHSSMODEM_H

#include <QObject>
#include "slot_params.h"
#include "fastfourier.h"
#include "clogger.h"

class FhssModem : public QObject
{
    Q_OBJECT
public:
    explicit FhssModem(QObject *parent = nullptr, int num=0);
    ~FhssModem();

    int Number; // номер модема

    void setConfiguration(SLOT_VARIANT* vars);
    void modulate(QList<int> *codes, int pos);
    QList<int> *demodulate(int parcel);

    void set_reference_parcel(); // установить опорную посылку
    void off_reference_parcel(); // убрать опорную посылку
    void clearSignal();
    void createSlotSignal();
    short_complex *getOutput();
    QList <fft_complex>* getDemodSpectr();   // указатель на тек.спектр посылки после FFT демодулятора

private:
   QList< fft_complex>* signal;
   QList <fft_complex>* parcelSpectrum;  // сигнальное созвездие посылки
   short_complex outputSignal[SLOT_LEN_MAX];    // модулированный сигнал (один слот) на выходе
   QVector<fft_complex>*inputSignal; // модулированный сигнал (два слота)на входе
   SLOT_VARIANT *slot;    // параметры текущей сигнальной конструкции

   int fftdim[4] {FFTDIM_3100, FFTDIM_4500, FFTDIM_9000, FFTDIM_12800} ;  // размерность fft для 4-х частот дискретизации
   fft_complex infftm1[FFTDIM_9000];   // входной массив для ifft (одна из посылок)
   fft_complex infftm2[FFTDIM_9000];   // входной массив для ifft (другая из посылок)
   fft_complex outfft[FFTDIM_9000];    // выходной массив ifft
   fft_complex* infft_op;        // указатель на опорную посылку
   fft_complex* infft_cur;        // указатель на текущую посылку

   FastFourier *ifft1[4]; //  ifft для 4-х ЧД для вх.буфера  infftm1
   FastFourier *ifft2[4]; //  ifft для 4-х ЧД для вх.буфера  infftm2

   fft_complex infftd[FFTDIM_9000];   // входной массив для fft
   fft_complex outfft1[FFTDIM_9000];    // выходной массив fft  (одна из посылок)
   fft_complex outfft2[FFTDIM_9000];    // выходной массив fft  (другая из посылок)
   fft_complex* outfft_op;        // указатель на опорную посылку в демодуляторе
   fft_complex* outfft_cur;        // указатель на текущую посылку в демодуляторе
   fft_complex outfft_mem[FFTDIM_9000];  // fft посылки для запоминания опорного сигнала(фишка режимов 1-4)

   FastFourier *fft1[4];  //  fft для 4-х ЧД для вых.буфера outfft1
   FastFourier *fft2[4];  //  fft для 4-х ЧД для вых.буфера outfft2
   FastFourier *fftmem;  //  fft для режимов 1-4 вых.буфера outfft_mem

   int first_chan = 81;
   int fft_dim;
   int freq_adjust_points;  // кол-во точек оп.посылки, идущее на перестройку РЧ
   int guard_interval;
   int sozvez_length; // число точек в созвездии
   int slotParcelsNumber;
   int begin_to_zero=0;  // номер отсчета, с которого в опорной посылке начинается обнуление freq_adjust_points

   int testIndex;  // текущее положение индекса массива тестовых канальных символо

   int sendCodes[MAX_CHANNELS_NUMBER];  // массив канальных символов одной посылки в передаче
   int recvCodes[MAX_CHANNELS_NUMBER];  // массив канальных символов одной посылки в приеме

   fft_complex constellation[17];  //сигнальное созвездие,2,4,8,16 значений в зависимости от кратности модуляции
                                                // + 17 - ая точка - для генерации посылок с нулевым сигналом
   int bandNumber=1;   // номер частотного диапазона (1-3100Гц , 2- 4500Гц, 3-9000Гц)
   int begin_point_shift = 0; // сдвиг начала осчета сигнала в слоте для демодулятора
   float freqCnt=0.0;
   bool frequencyCompensation = false;
   CLogger mlog;
   QList<int>* channelCds;  // сообщение слота в канальных кодах
   float maxMod;

   void constellation_diagram_set();   // установка сигн.созвездия в соотв.с типом модуляции

   void one_parcel_modulation_s(QList<int> *codes, int pos);   // относительная фазовая модуляция посылки
   void reference_parcel();                 // создание опорной посылки
   void one_parcel_ifft();
   void one_parcel_fft();
   void get_channel_symbols(); // Получение канальных символов в демодуляторе
   void demod_ofm1();
   void demod_ofm2();
   void demod_ofm3();
   void demod_afm4();
   qint16 goToShort(int koef, float value);
   void restoreReference(int parcel); // восстановить опорную фазу для режимов ФРЧ
   void create_infftd(int parcel); // создать входной массив для fft
   fft_complex get_spectrum_point(int ii);

signals:
   void symbolsSended(QString title,  int* codes, int len);

public slots:
   void setModemNumber(int num);
   int getModemNumber();
   void freqAdjustInterval();  // 3 мс начала опорного слота на перестройку РЧ
   bool shift_begin_point();
   void setCompensationMode(bool state);
   void setModemVariant(QVector<fft_complex> *signal);  // сигнал слота в одном из 80-ти вариантов сдвига
   void memoryParcelsFft(int parcel);  // сохранение посылки в качестве опоры для режимов 1-4
};

#endif // FHSSMODEM_H
