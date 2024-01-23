#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#include "t_bitstring.h"
#include "t_md5.h"

class ShiftRegister
{
public:
    ShiftRegister(){};
    ShiftRegister(BitString p_fb, int size = -1);

public:
    unsigned int size;
    BitString fb;
    BitString reg = 1;
public:

    void shift(uint32_t amount = 1);
    int bit();

public:
    void set_fb(BitString p_fb, int p_size = -1);
    void start(BitString p_reg);

    uint32_t next(int bits = 1);
    BitString next_bstr(int bits);

};
//MY_RAND=============================================================
void smrand(std::string p_data);
void smrand(BitString p_data);
BitString mrand_bstr(int len);
int mrand();

#endif // SHIFTREGISTER_H
