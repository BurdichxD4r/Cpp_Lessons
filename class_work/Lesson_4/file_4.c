// Примеры написания кода с бинарными операторами

#include <stdio.h>

int main(){
	unsigned int a = 60; // 60 = 00111100
	unsigned int b = 13; // 13 = 00001101
	int c = 0;
	c = a & b; // 12 = 00001100
	printf("Line 1 - Value of c is %d\n", c);
	c = a | b; // 61 = 00111101
	printf("Line 2 - Value of c is %d\n", c);
	c = a ^ b; // 49 = 00110001
	printf("Line 3 - Value of c is %d\n", c);
	c = ~a; // -61 = 11000011
	printf("Line 4 - Value of c is %d\n", c);
	c = a << 2; // 240 = 11110000
	printf("Line 5 - Value of c is %d\n", c);
	c = a >> 2; // 15 = 00001111
	printf("Line 6 - Value of c is %d\n", c);
	return 0;
}
