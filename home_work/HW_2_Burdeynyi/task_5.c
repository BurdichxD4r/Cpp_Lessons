/* TODO: Поменять местами значения переменных a и b
	без использования временной переменной c
*/

#include <stdio.h>

int main()
{
	int a = 5;
	int b = 4;
	printf("First value a = %d\nFirst value b = %d\n", a, b);
	a += b;
	b = a - b;
	a -= b;
	printf("\nSecond value a = %d\nSecond value b = %d\n", a, b);
	return 0;
}
