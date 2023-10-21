#include <stdio.h>

int main()
{
    int x = 1;
    switch (x)
    {
        case 1:
            {
                int z = 5;
            printf("z = %d\n", z);
            break;
            }
        default:
            printf("default case");
            break;
    }
    return 0;
}
