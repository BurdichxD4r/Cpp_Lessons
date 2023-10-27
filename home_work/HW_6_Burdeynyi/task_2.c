/* TODO: *Для 2-х строк введенных с клавиатуры определить все
	возможные варианты вхождения второй строки в первую.

	str1 = "qwertyqwerty";				str1 = "qwertyqwert";
	str2 = "qwty";					str2 = "qwty";
	варианты вхождения:				варианты вхождения:
	"qw..ty......"					"qw..ty....."
	"qw..t......y"					"qw...y....t"
	"qw........ty"					"q....y.w..t"
	"q......w..ty"					".....yqw..t"
	"......qw..ty"    =>    всего 16 вариантов	"....tyqw..."    =>    всего 8 вариантов
	".....yqw..t."					".w..tyq...."
	"....tyqw...."					"q...ty.w..."
	".w..tyq....."...				".w...yq...t"
			формула count1 * count2 * ... * countn = count_var;
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int count_var = 1;

    char * word1 = (char *)malloc(128);
    char * word2 = (char *)malloc(128);

    printf("Введите первую стороку:\n => ");
    scanf("%127s", word1);
    printf("Введите вторую стороку:\n => ");
    scanf("%127s", word2);

    for (int i = 0; i < strlen(word2); i++)
    {
        int count = 0;
        for (int j = 0; j < strlen(word1); j++)
        {
            if (*(word2 + i) == *(word1 + j))
                count++;
        }
        count_var *= count;
    }

    printf("Количество вариантов вхождения второй строки в первую = %d\n", count_var);

    return 0;
}
