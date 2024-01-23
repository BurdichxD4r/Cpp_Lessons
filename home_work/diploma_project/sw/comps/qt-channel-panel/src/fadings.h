#ifndef FADINGS_H
#define FADINGS_H

#include <QObject>
#include "eml.h"
#include "clogger.h"
#include "filters.h"

typedef	struct 	ChannelParams_s
{
    float	frequency_spreading;			// Растекание частоты, Гц
    float	gwave;					// Масштабный коэффициент земной волны
    int		Length;					// Длина входного сигнала
    int		n_sky;					// Число ионосферных волн
    int		orthInt;				// Интервал ортогональности
    float	SNR;					// Отношение сигнал/шум
    float	t1;						// Задержка первой ионосферной волны в мс
    float	t2;						// Задержка второй ионосферной волны в мс
    float	t3;						// Задержка третьей ионосферной волны в мс
    float	Shift;					// Максимальный частотный сдвиг
    float slot_mc;           // длительность слота,мс
    int freq_distr;            // частота дискретизации
    int ih_number;           // номер импульсной хар-ки замирания  (-1 - без замираний)
    const float* variant_ih;  // указатель на ИХ фильтра Релея
    int ih_length;              // длина фильтра ИХ
    int var_number;        // номер варианта замираний(0-10)
    void*			Private;			// закрытая структура
}	ChannelParams;

typedef struct channel_private_s
{
    float*			Q[6];				// Сигнал модуляции замираний
    int				dt[3];				// Массив задержек ионосферных волн
} channel_private;

static const	int		num_chan	= 256;

typedef enum Channel_Error_Codes_e
{
    CHANNEL_RET_OK,
    CHANNEL_RET_CANNOT_ALLOCATE_MEMORY,
} Channel_Error_Codes;

class TFadings : public QObject
{
    Q_OBJECT
public:
    explicit TFadings(QObject *parent = nullptr);
    ~TFadings();
private:
    float bf_drift_velocity=0.0;  // скорость смещения BestFrame
    int slotLength = SLOT_LEN_MAX;  // число отсчетов в массиве сигнала слота
    QList<fft_complex>* depart[256][3];  // три последовательных слота на выходе канала
    int slotsIndex=0;  // индекс, позволяющий организовать поток слотов по принципу FIFO
                        // без перезаписи слотов в модели канала
    int skyIndex[3] = {0,0,0}; // для ионосферных волн
    float offset[3];  // смещение слота в интервале входного массива двойной длины

    ChannelParams* cs[num_chan] = {nullptr}; // массив указателей на каналы

    CLogger flog;
    int slotNumber=0;
    QVector<float>* randArray[6];       // массив случайных чисел для вычисления свертки

    int replaceOffset(int dt, int nw);
    int	 mdm_channel_init(ChannelParams* pub);

    void GenerQ(ChannelParams* pub, bool init);
    void get_impulse_response(ChannelParams *pub, int variant);
    void doppler_add(QVector<fft_complex>* sig, float sh);  // добавить доппл.сдвиг (sh = 2*pi*df_gerc/f_discr) в сигнал
    void interpol(ChannelParams* pub, float  q0, float q1, int ii);  // интерполяция коэффициентов замирания до интервала дискретизации сигнала

signals:

public slots:
   void  form_gwave_output(QVector<fft_complex>* doubleSlot, QVector<fft_complex>* chanSignal, int chan,float sh);
   void  form_skywave_output(QVector<fft_complex>* doubleSlot,QVector<fft_complex>* chanSignal, int chan,int nwave,float sh);
   void setBfDrift(float drift);
   void setSlotLength(int len);
   int ctrl_channel_init( ChannelParams cp, int ind );
   void mdm_channel_close( int ind);
   void getChannelParams(int chanNum, ChannelParams* cp );
   int getSkyWaves(int chan);
   float getgWave(int chan);
   float getSNR(int chan);

};

#endif // FADINGS_H
