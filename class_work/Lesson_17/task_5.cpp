#include <iostream>

class Accumulator{
private:
    int m_counter = 0;
public:
    Accumulator(){}
    int operator()(int i){return (m_counter += i);}
};

int main(){
    Accumulator accum;
    std::cout << accum(30) << std::endl;
    std::cout << accum(40) << std::endl;
    return 0;
}
