/* TODO:Дано действительно число a. Пользуясь только умножением получить
	a^4 и a^10 за четыре операции.
*/

#include <stdio.h>

int main()
{
	float a = 7, a2, a4;
	a2 = a * a;
	a4 = a2 * a2;
	printf("a^4 = %f\na^10 = %f\n", a4, a4 * a4 *a2);
	return 0;
}
