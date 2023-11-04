/* TODO: Создать массив [5][7]. Инициализируем числами от 0 до 35:
	a) при помощи списка инициализаторов;
	b) в цикле ;
	c) с использованием вложенного цикла.
*/

#include <stdio.h>
#define N 5
#define M 7

int main()
{
    int arr1[N][M] = {{0, 1, 2, 3, 4, 5, 6}, {7, 8, 9, 10, 11, 12, 13},
		{14, 15, 16, 17, 18, 19, 20}, {21, 22, 23, 24, 25, 26, 27},
		{28, 29, 30, 31, 32, 33, 34}};
    int arr2[N][M];
    int arr3[N][M];

    int count = 0;
    for (int i = 0; i < N; i++)
    {
        arr2[i][0] = count;
        arr2[i][1] = count + 1;
        arr2[i][2] = count + 2;
        arr2[i][3] = count + 3;
        arr2[i][4] = count + 4;
        arr2[i][5] = count + 5;
        arr2[i][6] = count + 6;
        count += 7;
    }

    count = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            arr3[i][j] = count;
            count++;
        }
    }
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            printf("%d ", arr1[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            printf("%d ", arr2[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            printf("%d ", arr3[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    return 0;
}
