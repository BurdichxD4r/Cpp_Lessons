#include <iostream>

class N{
private:
    int m_n;
public:
    N(int n):m_n(n){}
    N & operator--();
    N operator++(int);
    void getN(){
        std::cout << m_n << std::endl;
    }
};

N & N::operator--(){
    if (m_n == 0){
        m_n = 8;
    }else{
        --m_n;
    }
    return * this;
}

N N::operator++(int){
    N tmp(m_n);
    ++(*this).m_n;
    return tmp;
}

int main(){
    N n1(3);
    n1.getN();
    --n1;
    n1.getN();
    n1++;
    n1.getN();
    return 0;
}
