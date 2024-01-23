#ifndef FHSSSLOT_H
#define FHSSSLOT_H

#include <QObject>
#include "fhssmodem.h"
#include "slot_params.h"
#include "treedsol.h"
#include "clogger.h"

class FhssSlot : public QObject
{
    Q_OBJECT
public:
    explicit FhssSlot(QObject *parent = nullptr);
    ~FhssSlot();

    void setSlotVariant(SLOT_VARIANT *vars);
    int getSlotMessageLength();
    void setSlotMessage(QList<int> *mess);
    QList<int> *getSlotMessage();
    short_complex *getOutput();
    SLOT_VARIANT* getVarslot();
    int getRefParcel();
    bool getShiftingState();
    QList <fft_complex>* getSlotSpectrum();  // выдает указатель на набор спектральных отсчетов,принятых в слоте
    float getDoppler(); // выдать значение сдвига частоты,Гц
    float getSNR(); // выдать значение SNR,определяемое через дисперсию фазы,дБ
    int getRSerr();  // выдать число ошибок фильтра Рида-Соломона
    int getmaxRSerr(); // максимум исправляемых РС - фильтром ошибок
    int getBestFrame(); // выдать BestFrame
    int getBestFrameShifts(); // число сдвигов BestFrame
    void setRR2(bool rnd);

private:
    SLOT_VARIANT *varslot;
    int slotParcelsNumber;       // число посылок в слоте
    int referenceParcel;           // номер опорной посылки(1-16) -1, если таковой нет
    QList<int>* referenceCodes;  // канальные коды для опорной посылки
    TReedSol *rscodec;
    bool rsValid = true;

    FhssModem *modems;
    CLogger slog;

    QList <fft_complex>* slotSpectrum;  // сигнальное созвездие слота
    QList <fft_complex>* bestSpectrum;  // спектр лучший из всех фреймов
    QList<int>* decodedWord;  // декодированное сообщение слота
    QList<int>* bestDecoded; // декодированное сообщение лучшее

    float SNR_from_phase_variance;  // SNR по эмпирич.формуле через дисперсию фазы в слоте
    float phase_shift;   // мат. ожидание доп. набега фазы (статистика слота)
    float phase_variance;   // дисперсия доп.набега фазы
    float frequencyShift = 0.0;  //    0.0015708 <-> 2 Гц
    int slotLength = SLOT_LEN_MAX;  // число отсчетов в массиве сигнала слота
    QVector<int_complex>* doubleSlot; // входной массив длиной в два слота
    QVector<fft_complex>* floatDoubleSlot; // входной массив длиной в два слота, но Float
    float freqCnt=0.0;
    bool frequencyCompensation = false;

    int RS_error_number;  // число исправленных фильтром Рида-Соломона ошибок
    //int RS_error_max; // максимальное число исправляемых Ридом-Соломоном ошибок
    int MinError; // минимальное число ошибок Рида-Соломона среди N сдвигов на слоте
    int maxErrorsRS; // максимальное число исправляемых РС ошибок
    int BestFrame=40;  // номер сдвига входного сигнала, при котором достигается синхронизация
    int bestFrameShifts = 80; // число сдвигов, используемых в демодуляторе для определения BestFrame
    int DecShift;  // число отсчетов, на которые происходит сдвиг при поиске BestFrame

    QList<int>* codedWrd;   // закодированное сообщение для слота
    QList<int>* channelCds;  // сообщение слота в канальных кодах
    int activeFreqNumber=32;  //число активных частот ППРЧ
    bool statisticsMode=false;  // режим сбора статистики по ЧК
    Quality_Stat qu_stat[32];  // массив статистик по ЧК для входа в НРЧ
    QList<Quality_Stat> stat;
    int statSlotCounter;       // счетчик слотов при наборе статистики
    int number_of_counts;  // число слотов для получения итоговой статистики по ЧК
    bool randReferenceParcel = true;  // для снижения пик-фактора в слоте

    void Rewrite(QList<int> *Input, int Input_Size, QList<int> *Output, int Output_Size);
    void referenceCodesCreate();  // создание массива канальных символов для опорной посылки
    void GetShift(QList <fft_complex>* sozv );
    float getPhaseAdd(float phase,float sector);  // получение дополнительного набега фазы
    void compensate_for_the_doppler();
    void calculateBestFrame();
    void getBestCode();
    void seachBestVariance(int errNum, int kk, bool *success, float *disp);
    int   frameDecoding(int frameNumber);
    void scrambler_symbols(QList<int>* cword);

signals:

public slots:
    void setCompensationMode(bool state);
    void setDoubleSlotInput(QVector<int_complex>*signal);
    int getSlotLength(); // длина слота(количество отсчетов в массиве)
    void setModemNumber(int num);
};

#endif // FHSSSLOT_H
