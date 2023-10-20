/* TODO: Написать программу с переполнением по типу double.
*/

#include <stdio.h>

int main()
{
	double a = 1.7976931348623158E308;
	printf("Double a is     %e\n", a);
	a += 0.0000000000000001E308;
	printf("New double a is %e\n", a);
	return 0;
}
