#include <iostream>
#include <cstdlib>

class Students{
private:
    std::string name;
    char group[10];
    short grade[5];
public:
    void setValue(std::string value){
        name = value;
    }
    void setValue(char value[10]){
        for (int i = 0; i < 10; i++) {
            group[i] = value[i];
        }
    }
    void setValue(short value[5]){
        for (int i = 0; i < 5; i++) {
            grade[i] = value[i];
        }
    }
    void printValues(Students &data){
        std::cout << data.name << '\n' << data.group << '\n' <<
            (data.grade[0] + data.grade[1] + data.grade[2] + data.grade[3] +
                data.grade[4]) / 5.0 << std::endl;
    }
};

int main(){
    Students student;
    std::string name;
    char group[10];
    short grade[5];

    std::cout << "Введите Фамилию и инициалы\n => ";
    std::getline(std::cin, name);
    student.setValue(name);
    std::cout << "Введите номер группы\n => ";
    std::cin >> group;
    student.setValue(group);
    std::cout << "Введите успеваймость\n";
    for (int i = 0; i < 5; ++i) {
        std::cout << " => ";
        std::cin >> grade[i];
    }
    student.setValue(grade);

    student.printValues(student);
    return 0;
}
