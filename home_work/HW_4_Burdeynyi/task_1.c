/* TODO: Сделайте так, чтобы цифры выводились следующим образом :
	        1
	      2 1
	    3 2 1
	  4 3 2 1
	5 4 3 2 1
*/

#include <stdio.h>

int main()
{
    int height;
    printf("Какой высоты пирамиду Вы хотите?\n => ");
    scanf("%d", &height);
    for (int first_number = 1; first_number <= height; first_number++)
    {
        for (int space = 0; space < height - first_number; space++)
        {
            printf("  ");
        }
        for (int out_number = first_number; out_number >= 1; out_number--)
        {
            printf("%d ", out_number);
        }
        printf("\n");
    }
    return 0;
}
