#ifndef FASTFOURIER_H
#define FASTFOURIER_H

/*
  Fast Fourier Transform
*/
#include <complex>
#include <QObject>

#define FFTW_FORWARD (-1)
#define FFTW_BACKWARD (+1)
#define M_2PI (6.283185307179586476925286766559f)
#define RAND_MAX_LOCAL 0x7fff // верхняя граница интервала генерации равномерно распределенных случ.чисел

static const float pi = 3.141592653589793238462643383279502884f;

typedef  std::complex<float> fft_complex;
typedef  std::complex<short> short_complex;
typedef  std::complex<int> int_complex;

class FastFourier : public QObject
{
    Q_OBJECT
public:
    explicit FastFourier(QObject *parent = nullptr,int n=128, fft_complex *in=nullptr, fft_complex *out=nullptr, int sign=FFTW_FORWARD);
    ~FastFourier();

    void fft_execute();

private:
    int fftDim;   // размерность FFT(число точек) должно быть четным
    int fftSign;  // направление (прямое, обратное)
    unsigned fftFlags;
    fft_complex *buffer_in;   // указатель на входной массив
    fft_complex *buffer_out;  // указатель на выходной массив

    void FFT(fft_complex *signal, fft_complex *buffer, const unsigned int size);
    void IFFT(fft_complex *signal, fft_complex *buffer, const int size);
    void flip(fft_complex *temp1, int temp_size);
    void fft2(fft_complex *in, fft_complex *x, int N, int complement);

    fft_complex *createWstore(unsigned int L, int complement);
    int fft_step( fft_complex *x, unsigned int T, unsigned int M, const  fft_complex *Wstore);

signals:

};

#endif // FASTFOURIER_H
