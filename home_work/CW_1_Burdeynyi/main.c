/* TODO: Сделать программу «Телефонный справочник». Программа должна
    хранить связку «имя - телефон» с помощью структуры. Программа
    должна работать в режимах:

	1. Добавление новых записей с консоли. Хранение записей в
    файле.
	2. Выводить все записи справочника

	3.* Режим поиска существующей записи
*/

#include "manege_file.h"

int main(){
    struct List_contacts * list = malloc(256 * sizeof(struct List_contacts));
    int len_struct = 0;
    int menu_value = 1;

    if (read_data_file(list, &len_struct)) return 1;

    printf("1. Показать список контактов\n"
	"2. Добавить контакт\n"
	"3. Показать меню\n"
	"4. Выйти из программы\n");

    while (menu_value != 0){
        printf(" => ");
        scanf("%d", &menu_value);

        switch (menu_value){
            case 1:
                printf("Список всех контактов\n");
                print_data(list, &len_struct);
                break;
            case 2:
                printf("Добавление нового контакта\n");
                add_contact(list, &len_struct);
                break;
            case 3:
                printf("1. Показать список контактов\n"
			"2. Добавить контакт\n"
			"3. Показать меню\n"
			"4. Выйти из программы\n");
                break;
            case 4:
                menu_value = 0;
                break;
            default:
                printf("Некорректное значение!\n");
                break;
        }
    }
    return 0;
}