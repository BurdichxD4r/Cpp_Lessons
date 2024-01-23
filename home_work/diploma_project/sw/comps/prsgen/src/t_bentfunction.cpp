#include "t_bentfunction.h"

using namespace std;

void MaioranaMcFarland::set_old(vector<int> p_perm, vector<vector<int> > p_zheg, int p_size)
{
    perm = p_perm;
    zheg = p_zheg;
    size = p_size;

    mask.clear();
    mask.setbit(size/2);
    mask = sub(mask, 1);
}

int MaioranaMcFarland::f_old(BitString pol)
{
    BitString u2 = pol & mask;
    BitString u1 = (pol >> size/2);

    u1 = u1 & u2.transpos(perm);

    int g = 0;
    for(int i = 0; i < int(zheg.size()); i++){
        int a = 1;
        for(int j = 0; j < int(zheg[i].size()); j++){
            a &= u2.test(zheg[i][j]);
        }
        g ^= a;
    }

    return (u1.count() ^ g) & 1;
}

BitString MaioranaMcFarland::permut(BitString u){
    return add(mult(u, k), p) & mask;
//    return mult(u, k) & mask;
}

void MaioranaMcFarland::set(BitString p_k, BitString p_p, int p_size)
{
    k = p_k;
    p = p_p;
    size = p_size;

    mask = 0;
    mask.setbit(p_size/2);
    mask = sub(mask, 1);

}

int MaioranaMcFarland::f(BitString pol)
{
    BitString u2 = pol & mask;
    BitString u1 = (pol >> size/2);

    int out = (u1 & permut(u2)).count()&1;

    return out ^ (u2.count()&1);
}


//==============================================

int MaioranaMcFarland::fromqt(BitString p_reg)
{
    int res;

    BitString u1;
    BitString u2;

    BitString mask;
    mask.setbit(size/2, 1);
    mask = sub(mask, 1);

    u1 = (p_reg>>(size/2));
    u2 = (p_reg & mask);

    u1 = u1 & u2.transpos(perm);          //левая часть

    int g = 0;
    for(int i = 0; i < int(zheg.size()); i++){              //правая часть
        int a = 1;
        for(int j = 0; j < int(zheg[i].size()); j++){
            a &= u2.test(zheg[i][j]);
        }
        g ^= a;
    }

    res = (u1.count() ^ g)&1;
    return res;
}
