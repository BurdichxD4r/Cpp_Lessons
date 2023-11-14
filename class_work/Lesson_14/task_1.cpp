#include <iostream>
struct DataClass{
    int day;
    int month;
    int year;
};

void print(DataClass &data){
    std::cout << data.day << "/" << data.month << "/" << data.year << std::endl;
}

int main(){
    DataClass today = {12, 11, 2018}; // используем uniform-инициализацию
    today.day = 18; // используем оператор выбора члена для выбора члена структуры
    print(today);
    return 0;
}
