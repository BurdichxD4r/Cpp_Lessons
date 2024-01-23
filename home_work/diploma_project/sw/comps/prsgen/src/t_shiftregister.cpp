#include "t_shiftregister.h"

using namespace std;

ShiftRegister::ShiftRegister(BitString p_fb, int size)
{
    set_fb(p_fb, size);
}

void ShiftRegister::shift(uint32_t amount)
{
    for(uint32_t i = 0; i < amount; i++){
        reg.setbit(size, (reg&fb).count()&1);
        reg = (reg>>1);
    }
}

inline int ShiftRegister::bit()
{
    return reg[0]&1;
}

void ShiftRegister::set_fb(BitString p_fb, int p_size)
{
    if(p_size <= 0)
        size = p_fb.bsize();
    else{
        size = p_size;
    }

    fb = p_fb;
    start(1);
}

inline void ShiftRegister::start(BitString p_reg)
{
    reg = p_reg;
}

uint32_t ShiftRegister::next(int bits)
{
    if(bits > 32){
        cout << "use BitString ShiftRegister::next_bstr(int bits) instead" << endl;
        assert(bits <= 32);
    }
    uint32_t out = 0;
    for(int i = 0; i < bits; i++){
        out <<= 1;
        out += bit();
        shift();
    }
    return out;
}

BitString ShiftRegister::next_bstr(int bits)
{
    BitString out;
    for(int i = 0; i < bits; i++){
        out.setbit(i, bit());
        shift();
    }
    return out;
}

//MY_RAND===============================================================
ShiftRegister shift_reg;

void smrand(string p_data)
{
    smrand(md5_bstr(p_data));
}
void smrand(BitString p_data)
{
//    shift_reg.set_fb(BitString("3531c2ee187ce6faa8d3b55a59573093"), 128);  //не взаимопростой с x
    shift_reg.set_fb(BitString("e22bfd2ec91534daeb4b03bdc44498db"), 128);
    shift_reg.start(p_data);
    shift_reg.shift(128);
}

BitString mrand_bstr(int len)
{
    return shift_reg.next_bstr(len);
}
int mrand()
{
    return shift_reg.next(16);
}
