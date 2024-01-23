#include "eml.h"
#include <QDebug>

float normrand(){   // нормальное распределение с единичной дисперсией
    float x=0;
    for(short i=0;i<12;i++)
        x+=(rand()%1001)*0.001;
    return x-6;
}

fft_complex awgn_noise()  // комплексный AWGN с единичной дисперсией
{
    float module = normrand();
    float angle = (rand()%3601)*0.1*pi/180;
    fft_complex res(module*cos(angle), module*sin(angle));
    //fft_complex res(normrand(), normrand());
    return res;
}

void awgn_test()
{
    float sum2 = 0;
    fft_complex x = awgn_noise();
    int N =10000;
    for(int ii=0;ii<N;ii++){
        x = 77.1f*awgn_noise();
        sum2 = sum2 + abs(x)*abs(x);
    }
    sum2 = sum2/(N-1);
    sum2 = sqrtf(sum2);
    qDebug() << "awgntest "<< sum2;
}

void normrand_test()
{
    float x,sr=0;
    float srr=0;
    for(int ii=0;ii<1000;ii++){
        sr += normrand();
    }
    sr /= 1000;
    for(int ii=0;ii<1000;ii++){
        x = normrand();
        srr = srr + (x-sr)*(x-sr);
    }
    srr /= 1000;
    srr = sqrtf(srr);
    qDebug() << "normtest "<< sr << srr;
}

fft_complex sigma(fft_complex *val, int number)
{
    fft_complex sr, x,res;
    float sig,x2, srr;
    sr = sig = 0;
    for (int ii=0;ii < number;ii++){
        sr += *(val+ii);
    }
    sr = sr / (float)number;
    srr = real(conj(sr)*sr);
    for (int ii=0;ii < number;ii++){
        x = (*(val+ii)) - sr;
        x2 = abs(x)*abs(x);
        sig = sig + x2;
    }
    sig = sig/(float)(number-1);
    sig = sqrtf(sig);
    res = fft_complex(sig,srr);
    qDebug() << real(res) << imag(res);
    return res;
}


