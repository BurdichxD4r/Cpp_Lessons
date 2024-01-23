#ifndef MATH_BIG_H
#define MATH_BIG_H

#include <QObject>
//#include <stdint.h>

/*******************************************************************************
 *   В данном файле определяются структуры для работы с большими целыми числами:
 *           uint128_t, uword64_t, word32_T, word16_T,
 *   а также функции: 
 *           add_128(), add_64() - сложения 128 и 64-разрядных чисел;
 *           sub_128(), sub_64() - вычитания 128 и 64-разрядных чисел;
 *           mult()              - беззнакового умножения 64-разрядных чисел;
 *           div()               - беззнакового деления нацело с остатком 128- 
 *                                 разрядного числа на 16-разрядное;
 *           inc_128(), inc_64() - увеличения 128 и 64-разрядных чисел на 1;
 *           dec_128(), dec_64() - уменьшения 128 и 64-разрядных чисел на 1.
 *
 *   В данном файле также определены операции сложения, умножения, деления и 
 * возведения в степень полиномов в поле Галуа GF(2^128), GF(2^64):
 *           xor_128(), xor_64()     - сложение полиномов;
 *           multp_128(), multp_64() - умножение полиномов;
 *           divp_128(), divp_64()   - деление полиномов;     
 *           powp_128(), powp_64()   - возведение в степень.
 ******************************************************************************/

/*******************************************************************************
 *   старшие разряды                                         младшие разряды
 *  -------------------------------------------------------------------------
 *  |XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX|
 *  -------------------------------------------------------------------------
 *  |           word32[1]               |             word32[0]             | 
 *  -------------------------------------------------------------------------
 *  |  word16[3]      |    word16[2]    |     word16[1]   |    word16[0]    |
 *  -------------------------------------------------------------------------
 *  | byte[7]| byte[6]| byte[5]| byte[4]| byte[3]| byte[2]| byte[1]| byte[0]|
 *  -------------------------------------------------------------------------
 ******************************************************************************/
typedef union{
  quint8 byte8[8];
  quint16 word16[4];
  quint32 word32[2];
} uword64_t;

typedef union{
  quint8 byte8[16];
  quint16 word16[8];
  quint32 word32[4];
  uword64_t word64[2];
} uint128_t;

typedef union {
  quint8 byte8[4];
  quint16 word16[2];
  quint32 word32;
} word32_t;

typedef union {
  quint8 byte8[2];
  quint16 word16;
} word16_t;

uint128_t init128(void);
uword64_t init64(void);
void add_128(const uint128_t *item1, const uint128_t *item2, uint128_t *result);
void addc_128(const uint128_t *item1, const quint32 item2, uint128_t *result);
void subc_128(const uint128_t *item1, const quint32 item2, uint128_t *result);
 
void add_64 (const uword64_t *item1 , const uword64_t *item2 , uword64_t *result);
void sub_128(const uint128_t *item1, const uint128_t *item2, uint128_t *result);
void sub_64(const uword64_t *item1, const uword64_t *item2, uword64_t *result);
void mult(const uword64_t *factor1, const uword64_t *factor2, uint128_t *result);
void multc(const uword64_t *factor1, const quint32 factor2, uint128_t *result);
void div_128(const uint128_t *factor1, const quint16 factor2,
             uint128_t *result_quot, quint16 *result_rem);

void inc_128(uint128_t *item1);
void inc_64(uword64_t *item1);
void dec_128(uint128_t *item1);
void dec_64(uword64_t *item1);
void xor_128(const uint128_t *item1, const uint128_t *item2, uint128_t *result);
void xor_64(const uword64_t *item1, const uword64_t *item2, uword64_t *result);
void multp_128(const uint128_t *pol1, const uint128_t *pol2, 
               const uint128_t *generic,  uint128_t *result);
void shl_128(const uint128_t *pol, uint128_t *result, quint8 shift);
void shr_128(const uint128_t *pol, uint128_t *result, quint8 shift);
void powp_128(const uint128_t *pol, uint128_t *degree,
              uint128_t *generic,  uint128_t *result);

#endif
