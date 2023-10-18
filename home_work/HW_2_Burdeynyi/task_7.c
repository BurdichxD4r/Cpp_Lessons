/* TODO: Осуществить перевод из 2 -> 10 систему счисления. Число в 2-ой
	системе счисления выбрать самостоятельно.
*/

#include <stdio.h>

int main()
{
	int number10;
	int number2 = 1010;
	number10 = (number2 / 1000 == 1) ? 8: 0;
	number10 += (number2 / 100 % 10 == 1) ? 4: 0;
	number10 += (number2 / 10 % 10 == 1) ? 2: 0;
	number10 += (number2 % 10 == 1) ? 1: 0;
	printf("Number2 = %d\nNumber10 = %d\n", number2, number10);
	return 0;
}
