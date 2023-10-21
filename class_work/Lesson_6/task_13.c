#include <stdio.h>

int main()
{
    for (char c = 'a'; c <= 'e'; ++c)
    {
        printf("%c", c);
        for (int i = 0; i < 3; ++i)
            printf("%d", i);
        printf("\n");
    }
    return 0;
}
