/* TODO: Напишите стандартный макрос MIN – поиск минимального значения из 2-х
	чисел. Для определенности пусть числа типа int.
*/

#include <stdio.h>
#define MIN(a, b) (a < b) ? a: b

int main()
{
    int a = 5;
    int b = 4;
    printf("Min = %d\n", MIN(a, b));
    return 0;
}
