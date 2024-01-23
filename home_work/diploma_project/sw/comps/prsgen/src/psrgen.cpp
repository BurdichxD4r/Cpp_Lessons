#include "psrgen.h"
#include <math.h>
#include <algorithm>

using namespace std;

template <typename T>
bool vec_contains(vector<T> &vec, T element){
    for(int i = 0; i < int(vec.size()); i++){
        if(vec[i] == element)
            return true;
    }
    return false;
}
///CONSTRUCTORS=====================================================================
///=================================================================================
PSRGen::PSRGen()
{
    BitString::initMulttable();

    set_open_key  ("This key can be any string (even empty)!");
    set_closed_key("This key can be any string as well!");
}

PSRGen::PSRGen(PSRGen &gen)
{
    *this = gen;
}

PSRGen::PSRGen(std::string p_open, std::string p_closed)
{
    BitString::initMulttable();

    set_open_key(p_open);
    set_closed_key(p_closed);
}
///GENERATE==========================================================================
///==================================================================================

/* current()
 *      Функция, применяющая набор бент-функций к текущему регистру.
 *      Возвращает текущее значение генератора ПСП.
 *
 *      Бент-функции из массива bent[32] применются к текущему регистру
 *      bit_range раз. На выходе имеем случайное число, длиной bit_range битов.
 *      Ясно, что bit_range не может быть больше 32.
 *
*/
int PSRGen::current()
{
    int out = 0;

    for(int i = 0; i < _bit_range - int(_alternate_odd_even); i++){
        out<<=1;
        out |= _bent[i].f(_reg);
    }

    if(_alternate_odd_even){
        out <<= 1;
        out |= _clock[0]&1;
    }

    return out;
}


/* next()
 *      Функция, увеличивающая внутренний счетчик на 1
 *      и возвращающая следующее значение генерации
*/
int PSRGen::next()
{
    _reg = quick_mod(_reg * _primitive);
    _clock.increment();

    return current();
}

/*  next(unsigned int amount)
 *      Выдает следующие amount значений генерации, записанные
 *      в std::vector<int>
*/
vector<int> PSRGen::next(unsigned int amount)
{
    vector<int> out(amount);
    for(unsigned int i = 0; i < amount; i++)
        out[i] = next();
    return out;
}


/* skip(unsigned int amount)
 *      Пропускает amount циклов генерации.
*/
void PSRGen::skip(uint64_t amount)
{
    this->set_clock(add(this->clock(), amount));
}


/* goback(unsigned int amount)
 *      Возвращается на amount циклов генерации назад.
*/
void PSRGen::goback(uint64_t amount)
{
    this->set_clock(sub(this->clock(), amount));
}


/* make_bstr(int size, bool tick)
 *      Возвращает большое сгенерированное число
 *
 *      int size  - количество битов в числе. До 128 бит.
 *      bool tick - увеличение счетчика времени на 1. По умолчанию false.
*/
BitString PSRGen::make_bstr(int size, bool tick)
{
    if(size > 128)
        return BitString();

    add_bents(size);
    BitString out;

    for(int i = 0; i < size; i++){
        out.setbit(i, _bent[i].f(reg()));
    }

    if(tick) next();

    return out;
}

///MATH FUNCTIONS====================================================================
///==================================================================================

/* init_quick_mod()
 *      Расчет массива _modulo. В него записываются остатки от деления
 *      полиномов вида x1x2x3...x8<<128 (8 бит и 128 нулей) на _prime для последующего
 *      использования функции quick_mod()
*/
void PSRGen::init_quick_mod(BitString prime)
{
//    _modulo.clear();

    prime.push_back(1);
    for(int i = 0; i < 256; i ++){
        _modulo[i] = ((BitString(i)<<128) % prime);
    }
}

/* quick_mod(BitString polynom)
 *      Функция, возвращающая остаток от деления параметра polynom на _prime.
 *      Работает быстрее, чем BitString::operator%, поскольку пользуется заранее
 *      рассчитанными данными в массиве _modulo
*/
BitString PSRGen::quick_mod(BitString p_polynom)
{
    BitString res = p_polynom;
    uint16_t byte;

    for(int i = p_polynom.size() - 1; i > 3; i--){
        for(int j = 3; j >= 0; j--){
            byte = res[i]>>(8*j)&0b11111111;
            res = res ^ (_modulo[byte]<<((i-4)*32 + j*8));
        }
    }
    while(res.size() > 4){
        res.pop_back();
    }
    return res;
}


/* init_exp(BitString base)
 *      Рассчет массива _powers. В него записываются степени порождающего полинома base,
 *      для последующего использования дихотомического возведения в степень.
*/
void PSRGen::init_exp(BitString base)
{
    _powers[0] = base;
    for(int i = 1; i < 129; i++){
        BitString add = _powers[i-1];
        _powers[i] = quick_mod(add * add);
    }
}

/* exp(BitString power)
 *      Функция для быстрого возведения полинома primitive в степень power (здесь BitString power рассматривается как число)
 *      Дихотомический алгоритм, не работает без изначальной инициализации (init_exp)
*/
BitString PSRGen::exp(BitString p_power)
{
    BitString res = 1; //Единица
    for(int a = 0; a < p_power.bsize(); a++){
        if(p_power.test(a))
            res = quick_mod(res * _powers[a]);
    }
    return res;
}
///SET FUNCTIONS=====================================================================
///==================================================================================
void PSRGen::set_bit_range(unsigned int num)    // Задает количество значащих бит в результате работы генератора
{
    if(num > 32){
        cout << "Bitrange cannot be greater than 32!!!" << endl;
        cout << "Bitrange is not set. Remaining value: " << _bit_range << endl;
        return;
    }
    _bit_range = num;
}
void PSRGen::set_alternate_odd_even(bool set)   // Задает параметр генератора - чередовать четные/нечетные числа
{
    _alternate_odd_even = set;
}

/* set_clock(BitString p_clock)
 *      Функция, задающая текущее время в генераторе.
 *      Время, по сути, является номером текущей итерации и увеличивается на 1 при каждом вызове функции next()
*/
void PSRGen::set_clock(BitString p_clock)
{
    _clock = p_clock;
    _reg = exp(p_clock);
}

/* set_open_key(string primary_key)
 *      Функция, задающая открытый ключ генератора.
 *      Поскольку открытый ключ представляет собой пару полиномов prime/primitive,
 *      в этой функции реализован алгоритм их нахождения. Для генерации "случайных" чисел в функции используется
 *      метод mrand(), также реализованный в данной библиотеке (см. t_shiftregister)
 *      primary_key, представляющий из себя произвольную строку, используется для инициализации mrand()
*/
void PSRGen::set_open_key(string primary_key)
{    
    _primary_open_key = primary_key;

    smrand(primary_key);
    BitString powers_mask [8];

    // Вспомогательные данные
    powers_mask[0] = BitString("55555555555555555555555555555555");
    powers_mask[1] = BitString("33333333333333333333333333333333");
    powers_mask[2] = BitString("f0f0f0ff0f0f0ff0f0f0ff0f0f0f");
    powers_mask[3] = BitString("ff00ffff00ffff00ffff00ff");
    powers_mask[4] = BitString("ffffffffffffffff");
    powers_mask[5] = BitString("280fffffd7f280fffffd7f");
    powers_mask[6] = BitString("3d30f19cd100ffffc2cfe632eff");
    powers_mask[7] = BitString("42f108940989af5720224");

    while(true){
        _prime = mrand_bstr(128);
        _prime.setbit(0, 1);

        init_quick_mod(_prime);

        _primitive = mrand_bstr(128);
        init_exp(_primitive);

        if(_powers[128] != _powers[0])
            continue;

        for(int i = 0; i < 8; i++)
            if(exp(powers_mask[i]) == 1)
                continue;

        break;
    }

    set_clock(1);
}

/* set_closed_key(string primary_key)     ТРЕБУЕТСЯ РЕДАКТИРОВАНИЕ
 *      Функция, задающая закрытый ключ генератора.
 *      Закрытый ключ представляет собой набор из 32 бент-функций от 128 переменных, находящихся в массиве _bent[128].
 *      Для генерации каждой нужно задать случайную перестановку от 64 переменных и случайную булевую функцию от 64 переменных.
 *      Это позволяет гарантированно задать бент функцию (см. класс Мэйонара-МакФарланда).
 *
 *      Для генерации "случайных" чисел используется метод mrand(), primary_key нужен для его инициализации
*/
void PSRGen::set_closed_key(string primary_key)
{
    _primary_closed_key = primary_key;

    smrand(primary_key);

    BitString spare_k;
    BitString spare_p;

    for(int i = 0; i < 32; i++){
        spare_k = mrand_bstr(64);
        spare_k.setbit(0);

        spare_p = mrand_bstr(64);

        _bent[i].set(spare_k, spare_p, 128);
    }
}


/* add_bents(int p_ceiling)
 *      В случае, когда бент-функци недостаточно для генерации числа (в функции make_bent())
 *      добавляет нужное количество бент-функций в массив bent[128]
*/
void PSRGen::add_bents(int p_ceiling)
{
    if(_bent_init >= p_ceiling)
        return;

    smrand(_primary_closed_key);
    mrand_bstr(128 * _bent_init);

    BitString spare_k;
    BitString spare_p;

    for(int i = _bent_init; i < p_ceiling; i++){
        spare_k = mrand_bstr(64);
        spare_k.setbit(0);

        spare_p = mrand_bstr(64);
        _bent[i].set(spare_k, spare_p, 128);
    }
    _bent_init = p_ceiling;
}


void PSRGen::set_prime(BitString p_prime)
{
    _prime = p_prime;
    init_quick_mod(_prime);
}

void PSRGen::set_primitive(BitString p_primitive)
{
    BitString powers_mask [8];

    // Вспомогательные данные
    powers_mask[0] = BitString("55555555555555555555555555555555");
    powers_mask[1] = BitString("33333333333333333333333333333333");
    powers_mask[2] = BitString("f0f0f0ff0f0f0ff0f0f0ff0f0f0f");
    powers_mask[3] = BitString("ff00ffff00ffff00ffff00ff");
    powers_mask[4] = BitString("ffffffffffffffff");
    powers_mask[5] = BitString("280fffffd7f280fffffd7f");
    powers_mask[6] = BitString("3d30f19cd100ffffc2cfe632eff");
    powers_mask[7] = BitString("42f108940989af5720224");

    _primitive = p_primitive;
    init_exp(_primitive);

    for(int i = 0; i < 8; i++)
        if(exp(powers_mask[i]) == 1)
            cout << "equals to 1 in power " << powers_mask[i] << endl;

}
///SYNC FUNCTIONS====================================================================
///==================================================================================
/* reg_info()
 *      Возвращает значение некоторых бит из текущего состояния reg. Номера битов находятся в _reg_info_mask. Этот
 *      параметр должен быть одинаков у всех абонентов сети
 *
 *      Предлагается отправлять значение reg_info, представляющее из себя несколько бит информации, в каждом слоте радиопередачи.
 *      Это позволит использовать функцию пассивной синхронизации.
*/
int PSRGen::reg_info()
{
    int out = 0;
    for(int i = 0; i < int(_reg_info_mask.size()); i++){
        out ^= _reg.test(_reg_info_mask[i]) << i;
    }
    return out;
}

/* sync(uint info)
 *      Функция пассивной синхронизации.
 *      На вход поступают значения нескольких бит из текущего значения регистра генератора, к которому пытаемся подстроиться.
 *      Номера этих битов содержатся в _reg_info_mask.
 *      При каджом вызове матрица equations дополняется некоторым количеством линейных уравнений, коих требуется 128 линейнонезависимых штук.
 *      Ясно, что одного вызова данной функции для полной синхронизации недостаточно, поэтому функция возврашает булевое значение,
 *      показывающее, набралось ли нужное количество уравнений, или нет.
 *      Функцию sync нужно вызывать всякий раз, когда в полученном сообщении мы получили информацию о состоянии регистра
 *
 *      Синхронизироваться можно лишь при знании открытого ключа. Функция подстройки без знания открытого ключа не реализовывалась,
 *      хотя считается, что противник имеет данную возможность.
 *
 *      ВАЖНО!!!
 *          Функция sync, несмотря на свое название, не восстанавливает значение _clock! Восстанавливается только значение _reg.
 *          Для восстановления значения счетчика времени все равно придется делать системный запрос в сеть.
 *          Бонусом является то, что делая этот запрос, мы уже синхронизированны с остальной сетью, то есть переключаемся на нужные частоты
 *
 *          Поскольку значение p_clock никак не используется в расчете следуещего значения генератора ПСП, целесообразность
 *          использования счетчика времени впринципе является спорной
*/
bool PSRGen::sync(unsigned int info)
{
    if(!_is_out_of_sync){
        _is_out_of_sync = true;
        _equations = Matrix(128);
        _sync_start_clock = _clock;
    }

    BitString since = sub(_clock, _sync_start_clock);

    BitString c_p = exp(since);
    Matrix p_m(128);

    for(int i = 0; i < 128; i++){
        p_m.set_column(quick_mod(c_p<<i), i);
    }

    int k = _reg_info_mask.size();

    for(int i = 0; i < k; i++){
        int row = _reg_info_mask[i];

        p_m.block[row] = (info>>i)&1;
        if(!_equations.add_row(p_m[row], p_m.block[row])){
            _equations.diagonalize();

            BitString solution = quick_mod(_equations.compile_solution() * c_p);

            _reg = solution;
            cout << "...Synchronization complete" << endl;
            return true;
        }
    }

//    cout << float(_equations.h_fill) /1.28 << "%" << endl;
    return false;
}


