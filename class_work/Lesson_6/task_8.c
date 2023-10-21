#include <stdio.h>

int main()
{
    int count = 1;
    while (count <= 50)
    {
        if (count < 10)
        {
            printf("0%d\n", count);
        }
        else
        {
            printf("%d\n", count);
        }
        if (count % 10 == 0)
            printf("\n");
        count++;
    }
    return 0;
}
