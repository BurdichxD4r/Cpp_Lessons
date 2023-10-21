#include <stdio.h>

int main()
{
    unsigned int count = 10;
    while (count >= 0)
    {
        if (count == 0)
        {
            printf("Wow!\n");
            break;
        }
        else
        {
            printf("%u\n", count);
        }
        --count;
    }
    return 0;
}
