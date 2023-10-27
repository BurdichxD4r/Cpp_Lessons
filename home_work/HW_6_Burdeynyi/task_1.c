/* TODO: Выполнить сравнение 2-х строк, введенных с клавиатуры с
	игнорированием пробелов.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char * word1 = (char *)malloc(128);
    char * word2 = (char *)malloc(128);
    printf("Введите первую строку:\n => ");
    scanf("%127s", word1);
    printf("Введите вторую строку:\n => ");
    scanf("%127s", word2);
    if (strcmp(word1, word2))
        printf("Строки разные\n");
    else
        printf("Строки одинаковые\n");
    return 0;
}
