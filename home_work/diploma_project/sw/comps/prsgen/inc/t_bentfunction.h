#ifndef BENT_H
#define BENT_H
#include "t_bitstring.h"


class MaioranaMcFarland
{
public:
    MaioranaMcFarland(){}
    MaioranaMcFarland(BitString p_k, BitString p_p, int p_size){
        set(p_k, p_p, p_size);
    }
public:
    int size;
    BitString k;
    BitString p;

    std::vector<std::vector<int>> zheg;
    std::vector<int> perm;

private:
    BitString mask;
    BitString permut(BitString u);
public:
    //old
    void set_old(std::vector<int> p_perm, std::vector<std::vector<int>> p_sheg, int p_size);
    int f_old(BitString pol);
    int fromqt(BitString p_reg);

    //new
    void set(BitString p_k, BitString p_p, int p_size);
    int f(BitString pol);

};


union un_64_t{
    uint32_t u32[2];
    uint64_t u64;
};
#endif // BENT_H
