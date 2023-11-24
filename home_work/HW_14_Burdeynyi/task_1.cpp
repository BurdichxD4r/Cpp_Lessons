/* TODO: Спроектировать структуру классов в соответствии с
    индивидуальным заданием. Наследование осуществляется по типу
    public. Создать несколько объектов производных классов, задавая
    случайным образом их свойства. Определить Вычисляемый
    показатель.

    Транспортное средство. Легковой автомобиль, грузовой автомобиль.
    [Максимальный пробег на полном бензобаке]
*/

#include <iostream>

class Vehicle{
protected:
    double m_fuelConsumption;
    double m_mileage;
    int m_fuelTank;
public:
    Vehicle(double fuelConsumption, int fuelTank, double mileage = 0)
        :m_fuelConsumption(fuelConsumption), m_mileage(fuelTank / fuelConsumption * 100), m_fuelTank(fuelTank){}

    friend bool operator>(const Vehicle & vehicle1, const Vehicle & vehicle2);
};

bool operator>(const Vehicle & vehicle1, const Vehicle & vehicle2){
    return vehicle1.m_mileage > vehicle2.m_mileage;
}

class Car: public Vehicle{
protected:
    std::string m_model;
public:
    Car(std::string model, double fuelConsumption, int fuelTank)
        :m_model(model), Vehicle(fuelConsumption, fuelTank){}

    friend std::ostream & operator<<(std::ostream & out, const Car & car);
};

std::ostream & operator<<(std::ostream & out, const Car & car){
    out << car.m_model << " " << car.m_mileage << " км\n";
    return out;
}

class Truck: public Vehicle{
protected:
    std::string m_model;
public:
    Truck(std::string model, double fuelConsumption, int fuelTank)
        :m_model(model), Vehicle(fuelConsumption, fuelTank){}

    friend std::ostream & operator<<(std::ostream & out, const Truck & truck);
};

std::ostream & operator<<(std::ostream & out, const Truck & truck){
    out << truck.m_model << " " << truck.m_mileage << " км\n";
    return out;
}

int main(){
    Car car("RIO", 9.7, 67);
    Truck truck("MAN TGX", 28.0, 1400);

    (car > truck)? std::cout << car: std::cout << truck;
    return 0;
}
