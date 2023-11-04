/* TODO: Задан массив из 30 вещественных чисел. Найти элемент (индекс),
	имеющий максимальное значение в массиве
*/

#include <stdio.h>
#define N 30
int main()
{
    float arr[N];

    float x = 12.234; // случайное float число
    for (int i = 0; i < N; i++)
    {
        x = (x * 3 < 70) ? x + 2.78: -98.321; // псевдорандомная комбинация
        x += 1.9832;
        arr[i] = x;
        printf("%f ", arr[i]);
    }
    printf("\n");

    float max_number = arr[0];

    for (int i = 0; i < N; i++)
    {
        if (max_number < arr[i]) max_number = arr[i];
    }
    printf("Max number is %f\n", max_number);

    return 0;
}
