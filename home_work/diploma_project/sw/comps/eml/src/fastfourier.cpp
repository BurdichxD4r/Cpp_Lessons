#include "fastfourier.h"
#include <math.h>

static unsigned char reverse256[]= {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

//This is array exp(-2*pi*j/2^n) for n= 1,...,32
//exp(-2*pi*j/2^n) = Complex( cos(2*pi/2^n), -sin(2*pi/2^n) )
static fft_complex W2n[32]={
    {-1.00000000000000000000000000000000f,  0.00000000000000000000000000000000f}, // W2 calculator (copy/paste) : po, ps
    { 0.00000000000000000000000000000000f, -1.00000000000000000000000000000000f}, // W4: p/2=o, p/2=s
    { 0.70710678118654752440084436210485f, -0.70710678118654752440084436210485f}, // W8: p/4=o, p/4=s
    { 0.92387953251128675612818318939679f, -0.38268343236508977172845998403040f}, // p/8=o, p/8=s
    { 0.98078528040323044912618223613424f, -0.19509032201612826784828486847702f}, // p/16=
    { 0.99518472667219688624483695310948f, -9.80171403295606019941955638886e-2f}, // p/32=
    { 0.99879545620517239271477160475910f, -4.90676743274180142549549769426e-2f}, // p/64=
    { 0.99969881869620422011576564966617f, -2.45412285229122880317345294592e-2f}, // p/128=
    { 0.99992470183914454092164649119638f, -1.22715382857199260794082619510e-2f}, // p/256=
    { 0.99998117528260114265699043772857f, -6.13588464915447535964023459037e-3f}, // p/(2y9)=
    { 0.99999529380957617151158012570012f, -3.06795676296597627014536549091e-3f}, // p/(2y10)=
    { 0.99999882345170190992902571017153f, -1.53398018628476561230369715026e-3f}, // p/(2y11)=
    { 0.99999970586288221916022821773877f, -7.66990318742704526938568357948e-4f}, // p/(2y12)=
    { 0.99999992646571785114473148070739f, -3.83495187571395589072461681181e-4f}, // p/(2y13)=
    { 0.99999998161642929380834691540291f, -1.91747597310703307439909561989e-4f}, // p/(2y14)=
    { 0.99999999540410731289097193313961f, -9.58737990959773458705172109764e-5f}, // p/(2y15)=
    { 0.99999999885102682756267330779455f, -4.79368996030668845490039904946e-5f}, // p/(2y16)=
    { 0.99999999971275670684941397221864f, -2.39684498084182187291865771650e-5f}, // p/(2y17)=
    { 0.99999999992818917670977509588385f, -1.19842249050697064215215615969e-5f}, // p/(2y18)=
    { 0.99999999998204729417728262414778f, -5.99211245264242784287971180889e-6f}, // p/(2y19)=
    { 0.99999999999551182354431058417300f, -2.99605622633466075045481280835e-6f}, // p/(2y20)=
    { 0.99999999999887795588607701655175f, -1.49802811316901122885427884615e-6f}, // p/(2y21)=
    { 0.99999999999971948897151921479472f, -7.49014056584715721130498566730e-7f}, // p/(2y22)=
    { 0.99999999999992987224287980123973f, -3.74507028292384123903169179084e-7f}, // p/(2y23)=
    { 0.99999999999998246806071995015625f, -1.87253514146195344868824576593e-7f}, // p/(2y24)=
    { 0.99999999999999561701517998752946f, -9.36267570730980827990672866808e-8f}, // p/(2y25)=
    { 0.99999999999999890425379499688176f, -4.68133785365490926951155181385e-8f}, // p/(2y26)=
    { 0.99999999999999972606344874922040f, -2.34066892682745527595054934190e-8f}, // p/(2y27)=
    { 0.99999999999999993151586218730510f, -1.17033446341372771812462135032e-8f}, // p/(2y28)=
    { 0.99999999999999998287896554682627f, -5.85167231706863869080979010083e-9f}, // p/(2y29)=
    { 0.99999999999999999571974138670657f, -2.92583615853431935792823046906e-9f}, // p/(2y30)=
    { 0.99999999999999999892993534667664f, -1.46291807926715968052953216186e-9f}, // p/(2y31)=
};

FastFourier::FastFourier(QObject *parent, int n, fft_complex *in, fft_complex *out, int sign) : QObject(parent)
{
    fftDim = n;
    fftSign = sign;
    buffer_in = in;
    buffer_out = out;
}

FastFourier::~FastFourier()
{

}

//Функции БПФ:
void FastFourier::flip(fft_complex *temp1, int temp_size)
{
    for (int i = 0, j = (temp_size - 1); i < j; i++, j--)
    {
        fft_complex	k = temp1[i];
        temp1[i] = temp1[j];
        temp1[j] = k;
    }
}

void FastFourier::FFT(fft_complex *signal, fft_complex *buffer, const unsigned int size)
{
    //Если указатели одинаковые,
    //то ничего переписывать не нужно
    if (signal != buffer)
    {
        for (unsigned int i = 0; i<size; i++)
        {
            buffer[i] = signal[i];
        }
    }

    unsigned int t = 0;

    //Битовая перестановка:
    for (unsigned int i=0; i<size; i++)
    {
        if (t > i)
        {
            fft_complex tmp1 = buffer[t];
            buffer[t] = buffer[i];
            buffer[i] = tmp1;
        }

        unsigned int mask = size / 2;
        while ((t&mask) != 0)
        {
            t &= (~mask);
            mask = mask / 2;
        }
        t = t | mask;
    }


    //Бабочки:
    int step = 1;
    fft_complex tt, w, factor;

    for (int k=0; k<log2(size); k++)
    {
        fft_complex i(0.0, 1.0);
        w = std::exp(i*(-pi/step));
        factor = fft_complex(1.0, 0.0);
        for (int j=0; j<step; j++)
        {
            for (unsigned int i=j; i<size; i=i+(step * 2))
            {
                int idx = i + step;

                tt = buffer[idx] * factor;

                buffer[idx] = buffer[i] - tt;

                buffer[i] = buffer[i] + tt;
            }
            tt = w * factor;
            factor = tt;
        }
        step = step * 2;
    }
}

void FastFourier::IFFT(fft_complex *signal, fft_complex *buffer, const int size)
{
    FFT(signal,buffer, size);
    flip(&buffer[1], size - 1);
}

void FastFourier::fft_execute()
{
    if (fftSign == FFTW_FORWARD)
    {
        if((fftDim == 80)||(fftDim == 96)||(fftDim == 192)||(fftDim == 128)){
            fft2(buffer_in, buffer_out, fftDim, 0);
        }
        else FFT(buffer_in, buffer_out, fftDim);
    }

    else if (fftSign == FFTW_BACKWARD)
    {
        if((fftDim == 80)||(fftDim == 96)||(fftDim == 192)||(fftDim == 128)){
            fft2(buffer_in, buffer_out,fftDim, 1);
        }
        else IFFT(buffer_in, buffer_out, fftDim);
    }
}

/*
  x: x - array of items
  N: N - number of items in array. Must be odd
  complement: false - normal (direct) transformation, true - reverse transformation
*/
void FastFourier::fft2(fft_complex *in, fft_complex *x, int N, int complement)
{
    int r, sL, m, rpsL, mprM, widx, step, step2, h, L, T, M;

    //Если указатели одинаковые,
    //то ничего переписывать не нужно
    if (in != x)
    {
        for (int i = 0;i < N;i++)
        {
            x[i] = in[i];
        }
    }


    if (N < 4)
    {
        return;
    }
    //N is odd: decrease sequence
    if (N & 1)
    {
        N--;
        x[0]  = fft_complex(0.0, 0.0);
    }

    //find L, M and T
    for(L = 1, T = 0; L < N && N % L == 0; L += L, T++)
    {
    }

    //return to nearest good L, T
    if (L != N)
    {
        L >>= 1;
        T--;
    }

    //find M
    M = N / L;

    //find rotation multipliers
     fft_complex *Wstore = createWstore(L, complement);

    //make usual FFT
    for (h = 0; h<M; h++)
        fft_step(x + h, T, M, Wstore);

    //remove multipliers
    delete[] Wstore;

    if (M != 1)
    {
        //allocate temporary array       
       fft_complex *X = new fft_complex[N];
       fft_complex one;
       float arg;

        //rotation multiplier array allocation       
       fft_complex *mult = new fft_complex [N];
       fft_complex *multEnd = mult + N;
       fft_complex *multPtr;
       mult[0] = fft_complex(1.0, 0.0);

        //step2 is
       for(step2=1; step2<N; step2+=step2)
        {
        }

        //rotation multiplier array evaluation
        for(step = step2 >> 1; step > 0; step2 = step, step >>= 1)
        {
           arg = (complement ? M_2PI : -M_2PI) * step / N;
           fft_complex i(0.0, 1.0);
           one = std::exp(i*arg);
           for(multPtr=mult+step; multPtr<multEnd; multPtr+=step2)
                *multPtr = (*(multPtr-step))*one;
        }

        fft_complex *pX;
        for(pX = X, rpsL = sL = 0; sL < N; sL += L)
        {
            for(r=0; r<L; r++, rpsL++, pX++)
            {
                mprM = r*M;
                *pX = x[mprM++];
                widx = rpsL;
                for(m=1; m<M; m++, mprM++, widx+=rpsL)
                {
                    if (widx >= N)
                        widx -= N;
                    one = mult[widx];

                    one *= x[mprM];

                    *pX += one;
                }
            }
        }        
        delete[] mult;

        //copy result from temporary array and free it
        for (h=0; h<N; h++)
            x[h] = X[h];       
        delete[] X;
    }

    //adjust values for complement transformation
    if (complement)
    {
        for( h=0; h<N; h++)
            x[h] /= static_cast <float>(N);
    }
}


fft_complex *FastFourier::createWstore(unsigned int L, int complement)
{
    unsigned int N, Skew, Skew2;
    fft_complex  *Warray, *WstoreEnd;
     //fftwf_complex_t  *Wstore;
    fft_complex WN, *pWN;
    Skew2 = L >> 1;
    //Wstore = new  fftwf_complex_t[Skew2];  
      fft_complex *Wstore = new fft_complex[Skew2];
    WstoreEnd = Wstore + Skew2;
    Wstore[0] = fft_complex(1.0, 0.0);

    for(N=4, pWN=W2n+1, Skew=Skew2>>1; N<=L; N+=N, pWN++, Skew2=Skew, Skew>>=1)
    {
        //WN = W(1, N) = exp(-2*pi*j/N)
        WN = *pWN;
        if (complement)
            WN = conj(WN);
        for(Warray = Wstore; Warray < WstoreEnd; Warray += Skew2)
            *(Warray + Skew) = *Warray * WN;
    }
    return Wstore;
}

int FastFourier::fft_step( fft_complex *x, unsigned int T, unsigned int M, const  fft_complex *Wstore)
{
    unsigned int L, I, J, MI, MJ, ML, N, Nd2, k, m, Skew, mpNd2;
    unsigned char *Ic = (unsigned char*) &I;
    unsigned char *Jc = (unsigned char*) &J;
     fft_complex S;
    const  fft_complex *Warray;
    fft_complex Temp;

    L = 1 << T;
    ML = M * L;

    //first interchanging
    for(I = 1, MI = M; I < L - 1; I++, MI += M)
    {
        Jc[0] = reverse256[Ic[3]];
        Jc[1] = reverse256[Ic[2]];
        Jc[2] = reverse256[Ic[1]];
        Jc[3] = reverse256[Ic[0]];
        J >>= (32 - T);
        if (I < J)
        {
            MJ = M * J;
            S = x[MI];
            x[MI] = x[MJ];
            x[MJ] = S;
        }
    }

    //main loop
    for(Nd2=M, N=M+M, Skew=L>>1; N<=ML; Nd2=N, N+=N, Skew>>=1)
    {
        for(Warray=Wstore, k=0; k<Nd2; k+=M, Warray+=Skew)
        {
            for(m=k; m<ML; m+=N)
            {
                Temp = (*Warray);
                mpNd2= m + Nd2;                
                Temp *= x[mpNd2];
                x[mpNd2] = x[m];                
                x[mpNd2] -= Temp;
                x[m] += Temp;
            }
        }
    }

    return 1;
}



