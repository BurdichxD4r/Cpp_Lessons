/*TODO: Перегрузка оператора[] при работе с указателем
*/

#include <iostream>

class intArray{
private:
    int m_array[10];
public:
    int & operator[](const int index){
        return m_array[index];
    }
};

int main(){
    intArray * array = new intArray;
    (*array)[4] = 5;
    std::cout << (*array)[4] << std::endl;
    delete array;
    return 0;
}
