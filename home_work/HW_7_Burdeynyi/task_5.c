/* TODO: Вычисление значения арифметического выражения, введенного с
	клавиатуры. Использовать операции: +, -
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N 255

int main()
{
    char plus = '+', minus = '-';
    char numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    int answer = 0;
    int tmp = 0;
    int f = 0;
    int f_sign = 0;
    int indent = 4;

    char * expression = (char *)malloc(N * sizeof(char));
    if (NULL == expression)
    {
        printf("No memory\n");
        return 1;
    }

    printf("Введите математическое выражение:\n => ");
    scanf("%s", expression);

    while (*expression != '\0')
    {
        if (*expression != plus && *expression != minus)
        {
            tmp *= 10;
            if (f_sign == 0)
            {
                for (int i = 0; i < 10; i++)
                {
                    if (*expression == numbers[i]) tmp += i;
                }
            }
            else
            {
                for (int i = 0; i < 10; i++)
                {
                    if (*expression == numbers[i]) tmp -= i;
                }
            }
            f = 1;
        }
        else if (*expression == plus)
        {
            f_sign = 0;
            f = 0;
        }
        else
        {
            f_sign = 1;
            f = 0;
        }

        if (f == 0)
        {
            answer += tmp;
            tmp = 0;
        }

        indent++;
        expression++;
    }
    answer += tmp;

    for (int i = 0; i < indent; i++) printf(" ");
    printf("=%d\n", answer);
    return 0;
}
