/* TODO: Составить описание класса для представления времени.
Предусмотреть возможности установки времени и изменения его
отдельных полей (час, минута, секунда) с проверкой допустимости
вводимых значений. В случае недопустимых значений полей
выводятся сообщения об ошибке. Создать методы изменения
времени на заданное количество часов, минут и секунд.
*/

#include <iostream>

class Time{
    int m_second;
    int oneMinute = 60;
    int oneHour = oneMinute * 60;
    int oneDay = oneHour * 24;
public:
    Time(int second = 0){m_second = second;}

    void getTime(){
        int tmp = m_second;
        tmp %= oneDay;

        if (tmp / oneHour <= 10){
            std::cout << "0" << tmp / oneHour << ":";
        }else{
            std::cout << tmp / oneHour << ":";
        }

        tmp %= oneHour;
        if (tmp / oneMinute <= 10){
            std::cout << "0" << tmp / oneMinute << ":";
        }else{
            std::cout << tmp / oneMinute << ":";
        }

        tmp %= oneMinute;
        if (tmp <= 10){
            std::cout << "0" << tmp << std::endl;
        }else{
            std::cout << tmp << std::endl;
        }
    }

    void setHour(int hour){
        m_second = m_second % oneHour + hour * oneHour;
    }
    void setMinute(int minute){
        m_second = m_second / oneHour * oneHour + m_second % oneMinute + minute * oneMinute;
    }
    void setSecond(int second){
        m_second = m_second / oneMinute * oneMinute + second;
    }
};

int main(){
    Time a(32949);
    a.getTime();
    a.setHour(23);
    a.getTime();
    a.setMinute(59);
    a.getTime();
    a.setSecond(59);
    a.getTime();
    return 0;
}
