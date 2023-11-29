/* TODO: Приводится пример 3-х классов CalcLength, CalcArea, CalcVolume, в которых в
    виртуальной функции Calc() возвращается соответственно длина окружности, площадь
    круга и объем шара.
    Для демонстрации создается некоторая функция ShowResult(), в которую передается
    указатель на базовый класс. Функция вызывает виртуальную функцию Calc() по
    указателю. В теле функции ShowResult() неизвестно, экземпляр какого класса будет ей
    передан. Экземпляр будет сформирован во время выполнения.
*/

#include <iostream>

class CalcLength{
private:
    std::string m_name;
protected:
    double m_r;
    double m_Pi;
public:
    CalcLength(double r, double Pi = 3.1415926535897932384, std::string name = "Длина окружности ")
        :m_r(r), m_Pi(Pi), m_name(name){}

    virtual std::string getName() const {return m_name;}

    virtual double Calc() const {
        return 2 * m_Pi * m_r;
    }

    ~CalcLength(){}
};

class CalcArea: public CalcLength{
private:
    std::string m_name;
public:
    CalcArea(double r, double Pi = 3.1415926535897932384, std::string name = "Площадь круга ")
        :CalcLength(r, Pi), m_name(name){}

    virtual std::string getName() const {return m_name;}

    virtual double Calc() const {
        return m_Pi * m_r * m_r;
    }

    ~CalcArea(){}
};

class CalcVolume: public CalcArea{
private:
    std::string m_name;
public:
    CalcVolume(double r, double Pi = 3.1415926535897932384, std::string name = "Объём шара ")
        :CalcArea(r, Pi), m_name(name){}

    virtual std::string getName() const {return m_name;}

    virtual double Calc() const {
        return 4.0 / 3 * m_Pi * m_r * m_r * m_r;
    }

    ~CalcVolume(){}
};

void ShowResult(const CalcLength * figure){
    std::cout << figure->getName() << "равна " << figure->Calc() << std::endl;
}

int main(){
    CalcLength length(10);
    CalcArea area(5);
    CalcVolume volume(1);

    ShowResult(&volume);
    ShowResult(&area);
    ShowResult(&length);
    return 0;
}
