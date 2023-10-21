#include <stdio.h>

int main()
{
    char user_char;
    printf("Yes or no (y/n)?\n");
    scanf("%c", &user_char);
    switch (user_char)
    {
        case 'y': case 'Y': {printf("Yes\n"); break;}
        case 'n': case 'N': {printf("No\n"); break;}
        default: printf("Error answer\n");
    }
    return 0;
}
