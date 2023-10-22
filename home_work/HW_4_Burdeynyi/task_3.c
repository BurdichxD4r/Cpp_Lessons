/* TODO: Напечатать таблицу умножения на 7. В виде : 1 х 7 = 7
						     2 х 7 = 14
							и т.д
*/

#include <stdio.h>

int main()
{
    for (int i = 1; i <= 10; i++)
    {
        printf("%d x 7 = %d\n", i, i * 7);
    }
    return 0;
}
