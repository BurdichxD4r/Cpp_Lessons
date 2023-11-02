/* TODO: Исправить код
    Дана строка символов. Необходимо убрать из нее все пробелы за 1 проход.
*/

#include <stdio.h>

int main()
{
    char str[] = "asdfn ddf ddd ffff";
    printf("%s\n", str);
    char * pStr = str;
    int count = 0;
    while (*pStr != '\0'){
        if (*pStr != ' '){
            *(str + count++) = *pStr;
        }
        pStr++;
    }
    *(str + count) = '\0';
    printf("%s\n", str);
    return 0;
}
