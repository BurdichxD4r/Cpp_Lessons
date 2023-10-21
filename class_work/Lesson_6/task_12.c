#include <stdio.h>

int main()
{
    int choice;
    do
    {
        printf("Please make a selection:\n");
        printf("1) Addition\n");
        printf("2) Subtraction\n");
        printf("3) Multiplication\n");
        printf("4) Division\n");

        scanf("%d", &choice);
    }
    while (choice != 1 && choice != 2 && choice != 3 && choice != 4);
        printf("You selected option #%d\n", choice);
    return 0;
}
