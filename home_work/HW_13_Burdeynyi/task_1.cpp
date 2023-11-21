/* TODO: Перегрузка оператора ввода (>>)
*/

#include <iostream>

class Input{
private:
    std::string m_name;
public:
    Input(std::string name = "Human"):m_name(name){}
    void getName(){std::cout << m_name << std::endl;}
    friend std::istream & operator>>(std::istream & in, Input & input);
};

std::istream & operator>>(std::istream & in, Input & input){
    in >> input.m_name;
    return in;
}

int main(){
    Input i;
    i.getName();
    std::cin >> i;
    i.getName();
    return 0;
}
