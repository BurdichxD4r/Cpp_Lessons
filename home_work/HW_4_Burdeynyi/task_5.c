/* TODO: *Дано натуральное число n (вводим с клавиатуры).
	Найти сумму : (n)^2 + (n+1)^2 + ... + (2n)^2.
	Результат вывести на экран.
*/

#include <stdio.h>

int main()
{
    int n, i = 0;
    int output_number = 0;
    printf("Введите n:\n => ");
    scanf("%d", &n);
    while (i != n)
    {
        output_number += (n + i) * (n + i);
        (n < 0) ? i--: i++;
    }
    output_number += 4 * n * n;
    printf("Summ = %d\n", output_number);
    return 0;
}
