/* TODO: Известен факториал числа. Найти это число (факториал числа n равен 1
	·2 · ... ·n).
*/

#include <stdio.h>

int main()
{
    int n_factorial, n;
    int factorial = 1;
    printf("Введите факториал n:\n => ");
    scanf("%d", &n_factorial);
    for (int i = 1; factorial < n_factorial; i++)
    {
        factorial *= i;
        n = i;
    }
    if (n % 2 == 0)
        printf("Число n = +-%d\n", n);
    else
        printf("Число n = %d\n", n);
    return 0;
}
