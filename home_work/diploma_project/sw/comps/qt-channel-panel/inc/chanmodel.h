#ifndef CHANMODEL_H
#define CHANMODEL_H

#include <QtWidgets>
#include "modempanel.h"
#include "eml.h"
#include "fadings.h"

class ColorGrid;
class TFadings;

static const float ln10_20 = 0.11512925464970229f;   // = log(10)/20
static const float afm4 = 0.8853247991556545f;   // (~sqrt(12.54/16)) уменьшение уровня сигнала в АФМ4
const int ROW_NUM = 16;
const int COL_NUM = 16;

const int COLOR_MIN = -50;
const int COLOR_MAX = 50;
const int COLOR_ZERO_SNR = 0; // значение цвета при ОСШ = 0 дБ
const float  SNR_SCALE= 1.7; // градаций цвета на 1 дБ ОСШ

const float ms = 4;
const float ss = 100;
const float snr1[]={ss,ms,ms,ms,ms,ms,ms,ms,ss,ms,ms,ms,ms,ms,ms,ms,
                             ss,ss,ms,ms,ms,ms,ms,ms, ss,ss,ms,ms,ms,ms,ms,ms,
                             ss,ss,ss,ms,ms,ms,ms,ms, ss,ss, ss,ms,ms,ms,ms,ms,
                             ss,ss,ss,ss,ms,ms,ms,ms,  ss,ss,ss,ss,ms,ms,ms,ms,
                             ss,ss,ss,ss,ss, ms,ms,ms, ss,ss,ss,ss,ss,ms,ms,ms,
                             ss,ss,ss,ss,ss,ss,ms,ms,   ss,ss,ss,ss,ss,ss,ms,ms,
                             ss,ss,ss,ss,ss,ss, ss, ms,  ss,ss,ss,ss,ss,ss,ss, ms,
                             ss,ss,ss,ss,ss,ss, ss, ss,  ss,ss,ss,ss,ss,ss,ss,ss};

class ChanModel : public QWidget
{
    Q_OBJECT
public:
    explicit ChanModel(QWidget *parent = nullptr);
    ~ChanModel();

private:
    QLabel *lbSnr;
    QComboBox *cbSnr;  // выбор отношения сигнал/шум АБГШ - канала

    QComboBox *cbSnrScale;  // шкала задания SNR
    ColorGrid* colorGrd;
    QGroupBox *channelGroup;
    QLineEdit* freqShiftSet;    // в Гц
    QLineEdit* bfShiftSet;    // смещение BestFrame за время одного слота

    QComboBox *fadingRateBox; // коэффициент замираний
    QListView *listView; // указатель на список элементов
    QLineEdit *waveDelay; // задержка ионосф.волны, мс
    QLineEdit *fadingRate; // число замираний в минуту
    QPushButton *applyButton;  // кнопка "Применить"

    QPushButton *m_activBut;
    QPushButton *m_emulBut;
    QPushButton *m_colorBut;
    QSpinBox    *m_colorSp;
    QSpinBox    *m_cellSp;
    TFadings* heavens;
    int          m_cell_num = 0;    // количество ячеек в таблице (сквозная нумерация от 0 до n)

    float snr_db=0;   // отношение с/ш в децибелах
    float EbN0_delta = 4.41;   // сдвиг шкалы SNR при переходе к метрике EbN0, дБ
    float EsN0_delta = 0   ;     // сдвиг шкалы SNR при переходе к метрике EsN0 , дБ
    int bandNumber=1;   // номер частотного диапазона (1-3100Гц , 2- 4500Гц, 3-9000Гц)
    int fft_dim = 80;       // размерность FFT
    float slot_mc = 200; // длительность слота,мс
    int subchan_number=29; // число подканалов в посылке
    float afm_adjust = 1;        // коэффициент уменьшения сигнала для АФМ4
    int modulationRate=1;   // кратность модуляции
    float awgnSigma=1;     // корень из дисперсии шума в канале при заданном ОШС
    int slotLength = SLOT_LEN_MAX;  // число отсчетов в массиве сигнала слота
    float koef_to_band=1; // коэфф.приведения SNR к EbN0
    int freqAll = 128;  // всего частот
    int countDistribution=0; // счетчик для переключения таблицы ОСШ по каналам

    int referenceParcel=0;  //
    float freqShift = 0;   //2*pi*freq_discr       (freq_discr==8000,9600,19200)
    float frqCnt=0.0;
    QVector<fft_complex>* nWavesSlot[num_chan]; // буфер длиной 2 слота для складывания лучей в модели замираний

    QVector <fft_complex> *chanSignal[num_chan];
    QVector<int_complex>* chanExit[num_chan];  // на выходе 24-х разрядный сигнал модели канала
    QVector<fft_complex>* doubleSlt; // буфер длиной в два слота

    float bf_drift_velocity=0.0;  // скорость смещения BestFrame
    const float fSpread[11] = {0,0.5, 1.5, 10.0, 0.1, 0.5, 1.0, 1.0, 0.5, 10.0, 30.0}; // разброс частот,Гц, в вариантах замирания
    const float fDelays[11] = {0,0.5, 2.0, 6.0, 0.5, 1.0, 2.0, 7.0, 1.0, 3.0, 7.0}; // времена задержек для 2-ой ионосф.волны,мс

    void initInterface();
    void createWidgets();
    void setTextDefinitions();
    float sigma_awgn();   // уровень помехи для генератора АБГШ при SNR=0
    void  snrMeasure(fft_complex *chanS); // оценка значения SNR
    int getReferentParcel(SLOT_VARIANT *vars);
    qint32 goToInt24( float value);  // преобразование float в qin32, имея в виду, что старшие 8 бит будут нулевые
    int replaceOffset();
    void applyChanges();
    void wavesAddition(QVector<fft_complex> *doubleSlot,QVector<fft_complex> *nW);
    void renewCellsCounters();
    float sigma_awgn_chan(int chan); // возвращает уровень АБГШ в канале chan

signals:
    void passSnrMetric(const QString& str);
    void   activeChSig(int);
    void   colorChSig(int, int);
    void chan_exit_send(QVector<int_complex>*);
    void   gridTimeOutSig(int);
    void   flashToCell(int);   // обращение к ячейке

private slots:
    void  setSnrValue(const QString &str);
    void   activeChange();
    void   colorChange();
    void   setSnrDistribution();
    void   setSnrDistributionNext();
    void   setSingleSnr(int cell,float snr);  // установить значение snr в ячейку таблицы с номером cell
    void setShiftValue();
    void setBfShiftValue();
    void applyFadingSwitching(int index);
    void setChannelsParams(int chanNum, float snr, float freqShift);
    void showSelectedChannelInfo(int chNum);
    void setIndexFadingBox();

public slots:
    void setSlotToChannel(short_complex* inChan, int chan);
    QVector<int_complex>* getChanOutput(int chan); // получить сигнал с выхода модели канала
    void  calculateChannels(QList<int> *flist);   // расчет частотных каналов, в которых были посылки
    void  cleanChannels(QList<int> *flist);   // очистка вых.массивов после recv
    void calculateChannel(QVector<fft_complex> *chanSignal1, QVector<int_complex>* chanExit1, int chan);
    void  calculateChannelTest(short_complex* inChan);
    void  setOptions(SLOT_VARIANT *vars);
    void setSnrIndex(int index);             // переключение индекса в комбобоксе, задающем SNR
    void setSnrIndexToGrid(int index, int cell);
    int getSlot_msec(); // возвращает длину слота в мсек
    void setAllFreqsNum(int num);

};

#endif // CHANMODEL_H
