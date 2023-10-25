/* TODO: Задача посложней. Нарисовать равнобедренный треугольник из
	символов ^. Высоту выбирает пользователь. Например: высота = 5, на
	         ^
                ^^^
        экране ^^^^^
              ^^^^^^^
             ^^^^^^^^^
*/

#include <stdio.h>

int main()
{
    int height;
    char block = '^';

    printf("Введите высоту треугольника:\n => ");
    scanf("%d", &height);

    int width = height - 1;

    for (int i = 1; i <= height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            printf(" ");
        }
        for (int j = 0; j < 2 * (height - width) - 1; j++)
        {
            printf("%c", block);
        }
        printf("\n");
        width--;
    }
    return 0;
}
