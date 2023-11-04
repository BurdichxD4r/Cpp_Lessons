/* TODO: Сортировка методом пузырька при помощи указателей.
*/

#include <stdio.h>
#define N 7

int main()
{
    int arr[N] = {6, 4, 5, 3, 1, 2, 0};

    for (int i = 0; i < N; i++)
    {
        printf("%d ", *(arr + i));
    }
    printf("\n");

    for (int i = 0; i < N; i++)
    {
        for (int j = N - 1; j > i; j--)
        {
            if (*(arr + i) > *(arr + j))
            {
                *(arr + i) += *(arr + j);
                *(arr + j) = *(arr + i) - *(arr + j);
                *(arr + i) -= *(arr + j);
            }
        }
    }

    for (int i = 0; i < N; i++)
    {
        printf("%d ", *(arr + i));
    }
    printf("\n");

    return 0;
}
