#ifndef PSRGEN_H
#define PSRGEN_H

#include <iostream>

#include "t_bitstring.h"
#include "t_matrix.h"
#include "t_shiftregister.h"
#include "t_bentfunction.h"

class PSRGen
{
public:
    PSRGen();
    PSRGen(PSRGen &gen);
    PSRGen(std::string p_open, std::string p_closed);
private: // properties
    std::string _primary_open_key;
    std::string _primary_closed_key;

    BitString _prime;
    BitString _primitive;

    MaioranaMcFarland _bent[128];
    int _bent_init = 32;

    int  _bit_range = 4;
    bool _alternate_odd_even = false;

private: // data
    BitString _clock = 0;
    BitString _reg;

    BitString _powers[129];
    BitString _modulo[256];

    std::vector<int> _reg_info_mask = {1, 2, 3, 4};
    Matrix _equations = Matrix(128);

    bool _is_out_of_sync = false;
    BitString _sync_start_clock;

private: // math_functions

      void init_exp(BitString base);
      void init_quick_mod(BitString prime);
 BitString exp(BitString p_power);
 BitString quick_mod(BitString p_polynom);
      void add_bents(int p_ceiling);

public: // set properties
      void set_bit_range(unsigned int num);
      void set_alternate_odd_even(bool set);

      void set_clock(BitString p_clock);
      void set_open_key  (std::string primary_key);
      void set_closed_key(std::string primary_key);

      void set_prime(BitString p_prime);
      void set_primitive(BitString p_primitive);

public: // get properties

    int bit_range(){return _bit_range;}
    bool alternate_odd_even(){return _alternate_odd_even;}

    BitString prime(){return _prime;}
    BitString primitive(){return _primitive;}

    BitString clock(){return _clock;}
    BitString reg(){return _reg;}

    std::string open_key(){return _primary_open_key;}
    std::string closed_key(){return _primary_closed_key;}

public: // synchronize
    int   reg_info();
    bool  sync(unsigned int info);

public: // output
    BitString make_bstr(int size, bool tick = false);
    int current();

    int next();
    std::vector<int> next(unsigned int amount);

    void skip(uint64_t amount);
    void goback(uint64_t amount);

};

#endif // PSRGEN_H
