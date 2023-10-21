#include <stdio.h>

int main()
{
    int outer = 1;
    while (outer <= 5)
    {
        int inner = 1;
        while (inner <= outer)
        {
            printf("%d\n", inner++);
        }
        printf("\n");
        ++outer;
    }
    return 0;
}
