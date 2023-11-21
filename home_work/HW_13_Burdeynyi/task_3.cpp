/*TODO: Перегрузка инкремента/декремента(++х/х--)
*/

#include <iostream>

class N{
private:
    int m_n;
public:
    N(int n = 0):m_n(n){}
    void getN(){std::cout << m_n << std::endl;}
    N & operator++();
    N operator--(int);
};

N & N::operator++(){
    ++m_n;
    return * this;
}

N N::operator--(int){
    N tmp(m_n);
    --(* this).m_n;
    return tmp;
}

int main(){
    N n(5);
    n.getN();
    ++n;
    n.getN();
    n--;
    n.getN();
    return 0;
}
