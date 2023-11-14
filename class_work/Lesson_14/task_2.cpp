#include <iostream>

class Values{
private:
    int m_number;
public:
    void setNumber(int number){
        m_number = number;
    }

    int getNumber(){return m_number;}
};

int main(){
    Values value;
    value.setNumber(7);
    std::cout << value.getNumber() << std::endl;
    return 0;
}
