#include <stdio.h>

int main()
{
    double num;
    printf("Enter any number\n");
    scanf("%lf", &num);

    if (num < 10)
        printf("The number less 10\n");
    else if (num == 10)
        printf("The number equal 10\n");
    else
        printf("The number greater 10\n");

    return 0;
}
