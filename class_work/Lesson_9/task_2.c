// TODO: Многомерные массивы

#include <stdio.h>

int main()
{
    int arr1[6] = {0, 1, 2, 3, 4, 5};
    int value5 = *(arr1 + 5);
    printf("%d\n", value5);

    int arr2[5][8] = {{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 1, 2, 3, 4, 5, 6, 7}};
    int value47 = *(*(arr2 + 4) + 7);
    printf("%d\n", value47);

    int arr3[3][4][6];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int g = 0; g < 6; g++)
            {
                arr3[i][j][g] = g;
            }
        }
    }
    int value235 = *(*(*(arr3 + 2) + 3) + 5);
    printf("%d\n", value235);
    return 0;
}
