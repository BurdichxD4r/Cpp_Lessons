#include <iostream>

class A{
private:
    int m_a;
public:
    A(int a = 0):m_a(a){}
    int getA() const {return m_a;}
    A operator-()const;
};

A A::operator-() const{
    return A(-m_a);
}

int main(){
    A a1(5);
    std::cout << (-a1).getA() << std::endl;
    return 0;
}
