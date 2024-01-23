#ifndef EML_H
#define EML_H

#include "fastfourier.h"

#define SLOT_LEN_MAX 2560    // 16*(128+32)
const int snr_index_max = 30;
const int scaleFactor[] = {1100, 720, 360};

fft_complex awgn_noise();   // комплексный AWGN с единичной дисперсией
float normrand();  // нормальное распределение с единичной дисперсией
void normrand_test();  // проверка на единичность дисперсии normrand()
void awgn_test();          // проверка на единичность дисперсии awgn_noise()
fft_complex sigma(fft_complex* val, int number); // считает энергию сигнала(val - массив, соотв.посылке)

#endif // EML_H
