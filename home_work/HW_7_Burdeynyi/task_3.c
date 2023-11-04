/* TODO: Изучить вопрос создания многомерного динамического массива
*/

#include <stdio.h>
#include <stdlib.h>

int main()
{
    unsigned int length, width;

    printf("Введите длинну и ширину:\n длинна => ");
    scanf("%ud", &length);
    printf(" ширина => ");
    scanf("%ud", &width);

    int ** arr = (int **)malloc(length * sizeof(int *) + length * width * sizeof(int));
    int * start = (int *)((char *)arr + length * sizeof(int *));
    for (int i = 0; i < length; i++)
    {
        arr[i] = start + i * width * sizeof(int);
    }

    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < width; j++)
        {
            arr[i][j] = 45;
        }
    }

    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < width; j++)
        {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }

    free(arr);

    return 0;
}
