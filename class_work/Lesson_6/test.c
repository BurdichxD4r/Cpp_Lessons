#include <stdio.h>

int main()
{
	char x = 6;
	x = x << 5;
	printf("%d\n", (int)x);
	x = x >> 2;
	printf("%d\n", (int)x);
}
