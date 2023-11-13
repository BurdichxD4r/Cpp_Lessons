#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define FILE_NAME "data_contacts.txt"

struct List_contacts{
    char name[10];
    char phone_number[12];
};

int create_data_file(){
    FILE * fp = fopen(FILE_NAME, "w");
    if (!fp){
        printf("[Error data file]\n");
        return 1;
    }
    fclose(fp);
    return 0;
}

void check_data_file(){
    // функция проверяет существует ли data_file,
    // если нет, то создаёт его
    int f = 1;
    while (f != 0){
        FILE * fp = fopen(FILE_NAME, "r");
        if (!fp){
            if (create_data_file()){
                printf("[Error create data file]\n");
                break;
            }
        }else{
            f = 0;
            fclose(fp);
        }
    }
}

int read_data_file(struct List_contacts * list, int * len_struct){
    // функция считывает данные из data_file и
    // записывает их в структуру, так же сохраняет
    // длину структуры
    check_data_file();
    FILE * fp = fopen(FILE_NAME, "r");
    if (!fp){
        printf("[Error data file]\n");
        return 1;
    }
    for (int i = 0; !feof(fp); i++){
        fscanf(fp, "%s %s", list[i].name, list[i].phone_number);
        if (list[i].name[0] != '\0') *len_struct += 1;
    }
    fclose(fp);
    return 0;
}

void print_data(struct List_contacts * list, int * len_struct){
    for (int i = 0; i < *len_struct; i++){
        printf("%s\t\t%s\n", list[i].name, list[i].phone_number);
    }
}

void add_contact(struct List_contacts * list, int * len_struct){
    printf("Введите имя\n => ");
    scanf("%s", list[*len_struct].name);
    printf("Введите номер\n => ");
    scanf("%s", list[*len_struct].phone_number);
    FILE * fp = fopen(FILE_NAME, "a");
    if (!fp) printf("[Error data file]\n");
    fprintf(fp, "%s\t\t%s\n", list[*len_struct].name, list[*len_struct].phone_number);
    *len_struct += 1;
}
