/* TODO: Найти произведение всех чисел от 8 до n. Число n – вводится с
	клавиатуры оператором. Вывести результат на экран.
*/

#include <stdio.h>

int main()
{
    short n;
    short i = 8;
    long double output_number = 8.0;
    printf("Введите n:\n => ");
    scanf("%hd", &n);
    do
    {
        (n > 8) ? i++: i--;
        output_number *= i;
        if (output_number == 0)
            break;
    }
    while (i != n);
    printf("Произведение всех чисел от 8 до %hd = %0.0Lf\n", n, output_number);
    return 0;
}
