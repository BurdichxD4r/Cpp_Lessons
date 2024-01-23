#include "t_bitstring.h"
#include <iomanip>

using namespace std;

uint16_t BitString::multtable[256][256];
bool BitString::multtable_init_completed;

BitString::BitString(string p_data)
{
    from_string(p_data);
}
BitString::BitString(uint64_t p_data)
{
    field.push_back(uint32_t(p_data));
    field.push_back(uint32_t(p_data >> 32));
}

void BitString::push_back(uint32_t p_data)
{
    field.push_back(p_data);
}
int BitString::size() const
{
    return int(field.size());
}
int BitString::bsize() const
{
    int byte = field.size() - 1;
    int bits_from_left = 0;

    while(byte != -1 && field[byte] == 0)
        byte--;
    if(byte == -1)
        return 0;

    uint32_t point = 0x80000000;
    while(!(point>>bits_from_left & field[byte])){
        bits_from_left++;
    }
    return (byte+1)*32 - bits_from_left;

}
void BitString::cleanZeros()
{
    while(field.back() == 0 && field.size() > 1)
        field.pop_back();
}
int BitString::test(int bit) const
{
    int byte = bit/32;
    if(byte >= this->size())
        return 0;
    return field[byte]>>(bit%32)&1;
}
void BitString::setbit(int bit, bool value)
{
    int byte = bit/32;
    if(value){
        while(byte >= this->size())
            field.push_back(0);
        field[byte] |= 1lu<<(bit%32);
    }else{
        if(byte >= this->size())
            return;
        field[byte] &= ~(1lu <<(bit%32));
    }
}
void BitString::clear()
{
    this->field.clear();
}
BitString BitString::transpos(vector<int> rule)
{
    BitString res = 0;

    for(int i = 0; i < int(rule.size()); i++){
        if(this->test(rule[i]))
            res.setbit(i);
    }
    return res;
}

void BitString::initMulttable()
{
    for(int i = 0; i <= 255; i++)
        for(int j = 0; j <= 255; j++){
            uint16_t word= 0;
            int z = 0;
            while(j >> z != 0){
                if((j>>z)&1)
                    word ^= i<<z;
                z++;
            }
            multtable[i][j] = word;
        }
    multtable_init_completed = true;
}
void BitString::increment()
{
    int byte = 0;
    while(true){
        if(this->size() <= byte)
            break;
        if(field[byte] == 0xFFFFFFFF){
            field[byte] = 0;
            byte++;
        }else{
            field[byte]++;
            return;
        }
    }
    field.push_back(1);
}
int BitString::count() const
{
    int n = 0;
    for(int i = 0; i < this->size(); i++){
        n += CountOnes32(field[i]);
    }
    return n;
}
//PRIVATE=============================================================================================================
//====================================================================================================================
inline int BitString::CountOnes32(uint32_t n) const //Взято с habr.com.  https://habr.com/ru/post/276957/
{
    n -= (n>>1) & 0x55555555;
    n = ((n>>2) & 0x33333333 ) + (n & 0x33333333);
    n = ((((n>>4) + n) & 0x0F0F0F0F) * 0x01010101) >> 24;
    return n;
}
void BitString::from_string(string p_data)
{
    field.clear();

    int point = p_data.length() - 8;
    stringstream stream;
    uint32_t word;

    while(point > 0){
        string bruh = p_data.substr(point, 8);
        stream << hex << bruh;
        stream >> word;
        stream.clear();
        this->push_back(word);
        point -= 8;
    }

    string chunk = p_data.substr(0, point + 8);
    stream << hex << chunk;
    stream >> word;
    this->push_back(word);
}

//OPERATORS===========================================================================================================
//====================================================================================================================
uint32_t &BitString::operator[](int num)
{
    return field[num];
}

BitString   operator >> (const BitString &p_left, int p_shift){
    if(p_shift < 0) return p_left<<(-p_shift);
    if(p_shift == 0) return p_left;

    BitString res;
    int L = p_left.bsize(); // старая степень
    int M = L - p_shift;     // новая степень
    if (M < 0){
        return 0;
    }

    int bytes = p_left.field.size();

    int n_word = M / 32 + 1;
    int bit_shift  = p_shift % 32;
    int word_shift = p_shift / 32;
    for (int i = 0; i < n_word; i++){
        uint32_t word = 0;
        if(bit_shift){
            if ((i + word_shift < bytes) && (i + word_shift >= 0)){
                word ^= p_left.field[i + word_shift] >> bit_shift;
            }
            if ((i + 1 + word_shift <bytes) && (i + 1 + word_shift >= 0)){
                word ^= p_left.field[i + 1 + word_shift] << (32 - bit_shift);
            }
        }else{
           if ((i + word_shift < bytes) && (i + word_shift >= 0))
                word ^= p_left.field[i + word_shift];
        }
        res.push_back(word);
    }
    return res;
}
BitString   operator << (const BitString &p_left, int p_shift){
    if(p_shift < 0) return p_left>>(-p_shift);
    if(p_shift == 0) return p_left;

    BitString res;
    int L = p_left.bsize(); // старая степень
    int M = L + p_shift;     // новая степень
    if (M <= 0){
        return 0;
    }

    int bytes = p_left.field.size();

    int n_word = M / 32 + 1;
    int bit_shift  = p_shift % 32;
    int word_shift = p_shift / 32;
    for (int i = 0; i < n_word; i++){
        uint32_t word = 0;
        if(bit_shift){
            if ((i - word_shift < bytes) && (i - word_shift >= 0)){
                word ^= p_left.field[i - word_shift] << bit_shift;
            }
            if ((i - 1 - word_shift < bytes) && (i - 1 - word_shift >= 0)){
                word ^= p_left.field[i - 1 - word_shift] >> (32 - bit_shift);
            }
        }else{
            if ((i - word_shift <bytes) && (i - word_shift >= 0))
                word ^= p_left.field[i - word_shift];
        }
        res.push_back(word);
    }
    return res;
}

void        operator>>=(BitString &p_left, int shift){
    p_left = p_left>>shift;
}
void        operator<<=(BitString &p_left, int shift){
    p_left = p_left<<shift;
}

stringstream &operator<< (stringstream &stream, const BitString p_pol){
    int bytes = p_pol.field.size();
    stream << "0x";
    for(int i = bytes-1; i >= 0; i--){
        if(i != bytes-1)
            stream << setfill('0') << setw(8);

        stream << hex << p_pol.field[i];
    }
    stream << dec;
    return stream;
}
basic_ostream<char> &operator<< (basic_ostream<char> &stream, const BitString p_pol){
    int bytes = p_pol.field.size();
    stream << "0x";
    for(int i = bytes-1; i >= 0; i--){
        if(i != bytes - 1)
            stream << setfill('0') << setw(8);

        stream << hex << p_pol.field[i];
    }
    stream << dec;
    return stream;
}

BitString   operator~   (const BitString &p_left)
{
    BitString res;
    for(int i = 0; i < p_left.size(); i++){
        res.push_back(~p_left.field[i]);
    }
    return res;
}
BitString   operator&   (const BitString &p_left, const BitString &p_right){
    BitString res;
    int L = p_left.size();
    int R = p_right.size();
    int M = (L > R) ? L : R;

    for (int i = 0; i < M; i++){
        uint32_t word = 0;
        word = (i < L) ? (p_left.field[i])  : 0;
        word &= (i < R) ? (p_right.field[i]) : 0;
        res.push_back(word);
    }
    return res;
}
BitString   operator|   (const BitString &p_left, const BitString &p_right){
    BitString res;
    int L = p_left.size();
    int R = p_right.size();
    int M = (L > R) ? L : R;

    for (int i = 0; i < M; i++){
        uint32_t word = 0;
        word = (i < L) ? (p_left.field[i])  : 0;
        word |= (i < R) ? (p_right.field[i]) : 0;
        res.push_back(word);
    }
    return res;
}
BitString   operator^   (const BitString &p_left, const BitString &p_right){
    BitString res;
    int L = p_left.size();
    int R = p_right.size();
    int M = (L > R) ? L : R;

    for (int i = 0; i < M; i++){
        uint32_t word = 0;
        word = (i < L) ? (p_left.field[i])  : 0;
        word ^= (i < R) ? (p_right.field[i]) : 0;
        res.push_back(word);
    }
    return res;
}

void        operator&=  (BitString &p_left, const BitString &p_right){
    p_left = p_left & p_right;
}
void        operator|=  (BitString &p_left, const BitString &p_right){
    p_left = p_left | p_right;
}
void        operator^=  (BitString &p_left, const BitString &p_right){
    p_left = p_left ^ p_right;
}
bool        operator==  (const BitString &p_left, const BitString &p_right){
    int L = p_left.size();
    int R = p_right.size();
    int M = (L > R) ? L : R;

    for(int i = 0; i < M; i++){
        uint32_t lword = (i < p_left.size()) ? p_left.field[i] : 0;
        uint32_t rword = (i < p_right.size()) ? p_right.field[i] : 0;
        if(lword != rword)
            return false;
    }
    return true;
}
bool        operator!= (const BitString &p_left, const BitString &p_right){
    return !(p_left == p_right);
}

BitString   operator* (const BitString &p_left, const BitString &p_right){

    assert(BitString::multtable_init_completed);

    BitString res;
    int res_len = (p_left.bsize() + p_right.bsize())/32 + 1;
    uint32_t chunk = 0;

    for(int int32 = 0; int32 < res_len; int32++){
        uint32_t word = 0;
        word ^= (chunk>>8);

        for(int bytenum = 0; bytenum < 4; bytenum++){
            chunk = 0;
            for(int i = bytenum + int32*4; i >= 0; i--){
                int j = bytenum + int32*4 - i;

                int bytel = (i/4 < p_left.size())? p_left.field[i/4]>>((i%4)*8) & 0xFF : 0;
                int byter = (j/4 < p_right.size())? p_right.field[j/4]>>((j%4)*8) & 0xFF : 0;

                chunk ^= BitString::multtable[bytel][byter];
            }
            word ^= chunk<<(bytenum*8);
        }
        res.push_back(word);
    }
    return res;
}
BitString   operator% (const BitString &p_left, const BitString &p_right){
    BitString res = p_left;
    int L = p_left.bsize();
    int R = p_right.bsize();

    if(L >= R)
        for(int i = L - R; i >= 0; i--)
            if(res.bsize() == R+i)
                res = res ^ p_right<<i;
    res.cleanZeros();
    return res;
}

bool      bigger    (const BitString &p_left, const BitString &p_right)
{
    int L = p_left.size();
    int R = p_right.size();
    int M = (L > R) ? L : R;

    for(int i = M - 1; i >= 0; i--){
        uint32_t wordl = (i < L) ? p_left.field[i] : 0;
        uint32_t wordr = (i < R) ? p_right.field[i] : 0;

        if(wordl > wordr)
            return true;
        if(wordl < wordr)
            return false;
    }
    return false;
}
bool      smaller   (const BitString &p_left, const BitString &p_right)
{
    return bigger(p_right, p_left);
}
bool      nsbigger  (const BitString &p_left, const BitString &p_right)
{
    return bigger(p_left, p_right) || p_left == p_right;
}
bool      nssmaller (const BitString &p_left, const BitString &p_right)
{
    return smaller(p_left, p_right) || p_left == p_right;
}

BitString sub (const BitString &p_left, const BitString &p_right)
{
    BitString res;
    int per = 0;

    for(int i = 0; i < p_left.size(); i++){
        uint32_t lword = (i < p_left.size())? p_left.field[i] : 0;
        uint32_t rword = (i < p_right.size())? p_right.field[i] : 0;
        res.push_back(lword -(rword + per));

        if(lword > rword){
            per = 0;
        }else if(lword < rword){
            per = 1;
        }
    }
    return res;
}
BitString add       (const BitString &p_left, const BitString &p_right)
{
    BitString res;
    int m = (p_left.size() > p_right.size())? p_left.size() : p_right.size();
    int i = 0;
    int per = 0;

    while(i < m || per){
        uint32_t lword = (p_left.size() > i) ? p_left.field[i] : 0;
        uint32_t rword = (p_right.size() > i) ? p_right.field[i] : 0;

        res.push_back(lword + rword + per);

        if(lword < 0xFFFFFFFF - rword)
            per = 0;
        else if(lword > 0xFFFFFFFF - rword)
            per = 1;

        i++;
    }

    return res;
}
BitString mult      (const BitString &p_left, const BitString &p_right)
{
    BitString res;
    int L = p_left.size();
    int R = p_right.size();

    res.field = vector<uint32_t>(L + R);

    for(int i = 0; i < L; i++){
        for(int j = 0; j < R; j++){
            uint64_t word = uint64_t(p_left.field[i]) * uint64_t(p_right.field[j]);
            BitString spare;
            spare.field = vector<uint32_t>(i+j + 2);
            spare.field[i+j] = word & 0xFFFFFFFF;
            spare.field[i+j+1] = (word>>32) & 0xFFFFFFFF;
            res = add(res, spare);
        }
    }
    res.cleanZeros();
    return res;
}
BitString div    (const BitString &p_left, const BitString &p_right)
{
    BitString res = 0;
    BitString divided = p_left;
    int L_deg = divided.bsize();
    int R_deg = p_right.bsize();

    for(int i = L_deg - R_deg; i >= 0; i--)
        if(nsbigger((divided>>i), p_right)){
            divided = sub(divided,(p_right<<i));
            res.setbit(i);
        }

    return res;
}
BitString mod       (const BitString &p_left, const BitString &p_right)
{
    BitString divided = p_left;
    int L_deg = divided.bsize();
    int R_deg = p_right.bsize();

    for(int i = L_deg - R_deg; i >= 0; i--)
        if(nsbigger((divided), p_right<<i))
            divided = sub(divided, (p_right<<i));

    divided.cleanZeros();

    return divided;
}
