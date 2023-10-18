// Примеры написания кода с операторами sizeof(), &, *, ?

#include <stdio.h>

int main(){
	int a = 4;
	short b;
	double c;
	int* ptr;
	/* примеры операторов sizeof */
	printf("Line 1 - Size of variable a = %ld\n", sizeof(a));
	printf("Line 2 - Size of variable b = %ld\n", sizeof(b));
	printf("Line 3 - Size of variable c = %ld\n", sizeof(c));
	/* примеры операторов & и * */
	ptr = &a; // 'ptr' now contains the address of 'a'
	printf("value of a is %d\n", a);
	printf("*ptr is %d\n", *ptr);
	/* пример тернарного оператора */
	a = 10;
	b = (a == 1) ? 20: 30;
	printf("Value of b is %d\n", b);
	b = (a == 10) ? 20: 30;
	printf("Value of b is %d\n", b);
	return 0;
}
