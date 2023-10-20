/* TODO: Написать программу демонстрирующую переполнение по типу char и вывести на экран
	и число и символ.
*/

#include <stdio.h>

int main()
{
	unsigned char a = 255;
	printf("Char a is \"%c\", int version - \"%d\"\n", a, (int)a);
	a += 1;
	printf("New char a is \"%c\", int version - \"%d\"\n", a, (int)a);
	return 0;
}
