#ifndef T_BITSTRING_H
#define T_BITSTRING_H
#include <iostream>
#include <vector>
#include <bitset>
#include <cassert>
#include <sstream>

class BitString
{
public: // constructor
    BitString(){}
    BitString(std::string p_data);
    BitString(uint64_t p_data);

public: // data
    std::vector<uint32_t> field;
public: //static data
    static uint16_t multtable[256][256];
    static bool multtable_init_completed;

public: // functions
        void push_back(uint32_t p_data);    // Pushes back the value
        void pop_back(){field.pop_back();};
        int  size() const;                  // Amount of bytes
        int  bsize() const;                 // Amount of bits
        int  count() const;                 // Counts amount of nonzero bits
        int  test(int bit) const;           // Returns value of the bit in asked position
        void setbit(int bit, bool value=true); // Sets value of the bit in asked position to be 1 or 0 (true/false)
        void clear();                       // Clears the field;
        void cleanZeros();                  // Deletes front bytes equal 0 (except the last one)
   BitString transpos(std::vector<int> rule);    // Permutates the ByteString

static  void initMulttable();               // Initiate multtable
        void increment();                   // Adds 1 to the BitString as to a number

private: // functions
       int   CountOnes32(uint32_t n) const;       //Counts the amount of nonzero bits in uint32_t
       void  from_string(std::string p_data);


public: // operators
    uint32_t & operator[] (int num);
};

//Binary shifts
BitString  operator>> (const BitString &p_left, int shift);
BitString  operator<< (const BitString &p_left, int shift);

void       operator>>=(BitString &p_left, int shift);
void       operator<<=(BitString &p_left, int shift);

//Streams operators
std::stringstream &operator<< (std::stringstream &stream, const BitString p_pol);
std::basic_ostream<char> &operator<< (std::basic_ostream<char> &stream, const BitString p_pol);

//Logical operations
BitString  operator~  (const BitString &p_left);
BitString  operator&  (const BitString &p_left, const BitString &p_right);
BitString  operator|  (const BitString &p_left, const BitString &p_right);
BitString  operator^  (const BitString &p_left, const BitString &p_right);
void       operator&= (BitString &p_left,const BitString &p_right);
void       operator|= (BitString &p_left,const BitString &p_right);
void       operator^= (BitString &p_left,const BitString &p_right);
bool       operator== (const BitString &p_left, const BitString &p_right);
bool       operator!= (const BitString &p_left, const BitString &p_right);

//Operations on polynoms
BitString   operator* (const BitString &p_left, const BitString &p_right);
BitString   operator% (const BitString &p_left, const BitString &p_right);

//Operations on numbers
bool     bigger       (const BitString &p_left, const BitString &p_right);    //p_left > p_right;
bool     smaller      (const BitString &p_left, const BitString &p_right);    //p_left < p_right;
bool     nsbigger     (const BitString &p_left, const BitString &p_right);    //p_left >= p_right;
bool     nssmaller    (const BitString &p_left, const BitString &p_right);    //p_left <= p_right;

BitString   sub       (const BitString &p_left, const BitString &p_right);    //p_left - p_right;
BitString   add       (const BitString &p_left, const BitString &p_right);    //p_left + p_right;
BitString   mult      (const BitString &p_left, const BitString &p_right);    //p_left * p_right;
BitString   div       (const BitString &p_left, const BitString &p_right);    //p_left / p_right;
BitString   mod       (const BitString &p_left, const BitString &p_right);    //p_left % p_right;

#endif // T_BITSTRING_H
