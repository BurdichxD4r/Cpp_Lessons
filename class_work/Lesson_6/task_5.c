#include <stdio.h>

int main()
{
    int a = 5;
    int b = 8;
    int c = a > b ? a - b : a + b;
    printf("c = %d\n", c);
    return 0;
}
