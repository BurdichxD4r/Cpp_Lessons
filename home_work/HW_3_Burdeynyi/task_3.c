/* TODO: Даны два числа x, y. Вывести первое число, если оно
	больше второго, и оба, если это не так.
*/

#include <stdio.h>

int main()
{
	int x = 4;
	int y = 5;
	(x > y) ? printf("x = %d\n", x): printf("x = %d\ny = %d\n", x, y);
	return 0;
}
