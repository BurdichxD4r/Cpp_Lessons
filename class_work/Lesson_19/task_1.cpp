#include <stdio.h>

class A{};
class B: public A{};
class C: public B{};

void method (A a){
    printf("a\n");}

void method (B b){
    printf("b\n");}

int main(){
    C c;
    method(c);
    return 0;
}
