/* TODO: Объявляется 3 класса с именами A, B, C. Класс A является базовым для класса B.
    Класс B является базовым для класса C. Классы содержат только один метод,
    выводящий название класса. С помощью механизма полиморфизма в
    функцию DemoPolymorphism() передается ссылка на один из экземпляров. В
    соответствии с переданным экземпляром вызывается требуемый метод.
*/
#include <iostream>

class A{
public:
    A(){}

    virtual void printNameClass() const {
        std::cout << "class A" << std::endl;
    }

    ~A(){}
};

class B: public A{
public:
    B(){}

    virtual void printNameClass() const {
        std::cout << "class B" << std::endl;
    }

    ~B(){}
};

class C: public B{
public:
    C(){}

    virtual void printNameClass() const {
        std::cout << "class C" << std::endl;
    }

    ~C(){}
};

void DemoPolymorphism(const A & a){
    a.printNameClass();
}

int main(){
    A a;
    B b;
    C c;

    DemoPolymorphism(c);
    DemoPolymorphism(b);
    DemoPolymorphism(a);
    return 0;
}
