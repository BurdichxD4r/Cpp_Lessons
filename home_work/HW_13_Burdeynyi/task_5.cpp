/*TODO: Перегрузка операций преобразования типов данных
*/

#include <iostream>

class Ruble{
private:
    int m_ruble;
public:
    Ruble(int ruble):m_ruble(ruble){}
    operator float(){return m_ruble;}
    double getRuble(){std::cout << (float)m_ruble << std::endl; return (float)m_ruble;}
};

int main(){
    Ruble ruble(13);
    ruble.getRuble();
    return 0;
}
