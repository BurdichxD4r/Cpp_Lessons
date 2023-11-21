#include <iostream>

class Car{
private:
    std::string m_comp;
    std::string m_model;
public:
    Car(std::string comp, std::string model):m_comp(comp), m_model(model){}
    friend bool operator==(const Car & car1, const Car & car2);
};

bool operator==(const Car & car1, const Car & car2){
    return (car1.m_comp == car2.m_comp && car1.m_model == car2.m_model);
}

int main(){
    Car car1("FORD", "MUSTANG");
    Car car2 ("KIA", "RIO");
    if (car1 == car2){
        std::cout << "True\n";
    }else{
        std::cout << "False\n";
    }
    return 0;
}
