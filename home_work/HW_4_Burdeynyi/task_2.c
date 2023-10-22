/* TODO: С клавиатуры вводится номер месяца. Необходимо проверить валидное
	ли введенное значение или нет, если нет выдать оператору сообщение
	об ошибке. Если значение валидное, вывести на экран название сезона
	к которому относится месяц.
*/

#include <stdio.h>

int main()
{
    unsigned char f;
    unsigned short month_number;

    do
    {
        printf("Введите номер месяца:\n => ");
        scanf("%hd", &month_number);
        f = 0;
        switch (month_number)
        {
            case 1:
                printf("Зима\n");
                break;
            case 2:
                printf("Зима\n");
                break;
            case 3:
                printf("Весна\n");
                break;
            case 4:
                printf("Весна\n");
                break;
            case 5:
                printf("Весна\n");
                break;
            case 6:
                printf("Лето\n");
                break;
            case 7:
                printf("Лето\n");
                break;
            case 8:
                printf("Лето\n");
                break;
            case 9:
                printf("Осень\n");
                break;
            case 10:
                printf("Осень\n");
                break;
            case 11:
                printf("Осень\n");
                break;
            case 12:
                printf("Зима\n");
                break;
            default:
                printf("[Error] Вы ввели некоректное значение.\n"
                       "Попробуйте снова\n");
                f = 1;
                break;
        }
    }
    while (f == 1);
    return 0;
}
