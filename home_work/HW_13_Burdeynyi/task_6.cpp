/*TODO: Перегрузка оператора присваивания (=)
*/

#include <iostream>

class Shaverma{
private:
    bool m_poison;
public:
    Shaverma(bool poison = 0):m_poison(poison){}
    void getShava(){std::cout << m_poison << std::endl;}
    Shaverma & operator=(const Shaverma & shava){
        m_poison = shava.m_poison;
        return * this;
    }
};

int main(){
    Shaverma shava1(1);
    Shaverma shava2;
    shava2.getShava();
    shava2 = shava1;
    shava2.getShava(); // теперь вторая тоже отравлена
    return 0;
}
