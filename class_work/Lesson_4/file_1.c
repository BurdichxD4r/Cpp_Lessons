// Примеры написания кода с логическими операторами:

#include <stdio.h>

int main(){
	int a = 5;
	int b = 20;
	int c;
	if (a && b){
		printf("Line 1 - Condition is true\n");
	}
	if (a || b){
		printf("Line 2 - Condition is true\n");
	}
	/* изменяем значения a и b */
	a = 0;
	b = 10;
	if (a && b){
		printf("Line 3 - Condition is true\n");
	} else {
		printf("Line 3 - Condition is not true\n");
	}
	if (!(a && b)){
		printf("Line 4 - Condition is true\n");
	}
	return 0;
}
