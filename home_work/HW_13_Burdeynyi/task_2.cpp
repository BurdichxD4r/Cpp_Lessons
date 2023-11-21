/* TODO: Перегрузка оператора сравнения (>)
*/

#include <iostream>

class Shop{
private:
    int m_revenue;
public:
    Shop(int revenue = 0.0):m_revenue(revenue){}
    friend bool operator>(const Shop & shop1, const Shop & shop2);
};

bool operator>(const Shop & shop1, const Shop & shop2){
    return shop1.m_revenue > shop2.m_revenue;
}

int main(){
    Shop shop1(1800);
    Shop shop2(2000);
    if (shop1 > shop2){
        std::cout << "Shop 1 top\n";
    }else{
        std::cout << "Shop 2 top\n";
    }
    return 0;
}
