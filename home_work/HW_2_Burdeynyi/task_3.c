/* TODO: Даны натуральные числа - n, m.
	Разделите n на m и выведите на экран
	консоли младшую цифру целой части
*/

#include <stdio.h>

int main()
{
	int n = 97;
	int m = 3;
	int res = n / m % 10;
	printf("Value res = %d\n", res);
	return 0;
}

