/* TODO: Вывод на экран консоли значения X:
	x = 2 % 2 + 2 * 2 - 2 / 2;
	x = (3 * 9 * (3 + (9 * 3 / 3)));
*/
#include <stdio.h>

int main()
{
	int x = 2 % 2 + 2 * 2 - 2 / 2;
	printf("First value x = %d\n", x);
	x = (3 * 9 * (3 + (9 * 3 / 3)));
	printf("Second value x = %d\n", x);
	return 0;
}
