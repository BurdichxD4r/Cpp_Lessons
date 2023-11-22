/* TODO: Определить массив указателей на функции. Вводить цифру, определяющую, какую функцию надо выполнить:
	0 - найти min число;
	1 - найти max число;
	2 - вычислить сумму;
	3 - вычислить разность;
	4 - произведение чисел;
	5 - найти частное;
	6 - завершить работу.

    Выполнить соответствующую функцию , используя указатель на неё.
*/

#include <stdio.h>
#include <stdlib.h>

int min(int x, int y){
    if (x < y) return x;
    else return y;
}

int max(int x, int y){
    if (x > y) return x;
    else return y;
}

int sum(int x, int y){
    return x + y;
}

int sub(int x, int y){
    return x - y;
}

int mul(int x, int y){
    return x * y;
}

int divid(int x, int y){
    return x / y;
}

int exit_(int x, int y){
    exit(0);
    return 0;
}

int main(){
    int x = 10;
    int y = 5;
    int (* operation[7])(int, int) = {min, max, sum, sub, mul, divid, exit_};

    printf("%d\n", operation[0](x, y));
    printf("%d\n", operation[1](x, y));
    printf("%d\n", operation[2](x, y));
    printf("%d\n", operation[3](x, y));
    printf("%d\n", operation[4](x, y));
    printf("%d\n", operation[5](x, y));

    operation[6](x, y);

    printf("%d\n", operation[0](x, y));

    return 0;
}
