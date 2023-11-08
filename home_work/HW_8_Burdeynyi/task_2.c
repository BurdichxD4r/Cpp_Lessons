/* TODO: Массив:
		считывается из файла;
		передается в функцию где он сортируется по убыванию ;
		отсортированный массива возвращается из функции ;
		сохраняется в файл ;
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define N 256
#define M 10000

void read_file(char * filename, int * arr){
    FILE * fp = fopen(filename, "r");

    if (fp){
        for (int i = 0; !feof(fp); i++){
            fscanf(fp, "%2d", &arr[i]);
        }
        fclose(fp);
    }
}

void sorted(int * arr){
    int lenarr = 0;
    while (arr[lenarr] != '\0') lenarr++;

    for (int i = 0; i < lenarr; i++){
        for (int j = lenarr - 1; j > i; j--){
            if (*(arr + i) < *(arr + j)){
                *(arr + i) += *(arr + j);
                *(arr + j) = *(arr + i) - *(arr + j);
                *(arr + i) -= *(arr + j);
            }
        }
    }
}

void write_file(char * filename, int * arr){
    int lenarr = 0;
    while (arr[lenarr] != '\0') lenarr++;

    FILE * fp = fopen(filename, "w");

    if (fp){
        for (int i = 0; i < lenarr; i++){
            fprintf(fp, "%2d", arr[i]);
        }
        fclose(fp);
    }
}

int main(){
    char * filename = (char *)malloc(N * sizeof(char));
    if (NULL == filename){
        printf("No memory!\n");
        return 1;
    }
    int * arr = (int *)malloc(M * sizeof(int));
    if (NULL == arr){
        printf("No memory!\n");
        return 1;
    }

    printf("Введите название файла из которого будем считывать\n => ");
    scanf("%s", filename);

    read_file(filename, arr);
    sorted(arr);
    write_file(filename, arr);

    printf("Файл успешно преобразован...\n");
    return 0;
}
