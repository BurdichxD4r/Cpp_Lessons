/* TODO: Требуется создать список из элементов последовательности
	целых чисел, вводимых пользователем. Также требуется написать
	функцию удаления всех элементов списка, имеющих некоторое
	значение x.
*/

#include <stdio.h>
#include <stdlib.h>
#define True 1

typedef struct Node{
    int value;
    struct Node * next;
} List;

List * create(int data){
    List * tmp = (List *)malloc(sizeof(List));
    tmp -> value = data;
    tmp -> next = NULL;
    return tmp;
}

void add_element_end(int data, List * head){
    List * tmp = (List *)malloc(sizeof(List));
    tmp -> value = data;
    tmp -> next = NULL;
    List * p = head;
    while (p -> next != NULL){
        p = p -> next;
    }
    p -> next = tmp;
}

List * add_element_n_position(int data, int n, List * head){
    List * p = head;
    int count = 1;
    while (count < n - 1 && p -> next != NULL){
        p = p -> next;
        count++;
    }
    List * tmp = (List *)malloc(sizeof(List));
    tmp -> value = data;
    tmp -> next = p -> next;
    p -> next = tmp;
    return head;
}

void print(List * tmp){
    while (tmp != NULL){
        printf("%d ", tmp -> value);
        tmp = tmp -> next;
    }
    printf("\n");
}

List * remove_element(int x, List * head){
    List * tmp = head, * p = NULL;
    if (head == NULL) return NULL;
    while (tmp && tmp -> value != x){
        p = tmp;
        tmp = tmp -> next;
    }
    if (tmp == head){
        head = tmp -> next;
        free(tmp);
        return head;
    }
    if (!tmp) return head;
    p -> next = tmp -> next;
    free(tmp);
    return head;
}

List * remove_all(List * head){
    while (head != NULL){
        List * p = head;
        head = head -> next;
        free(p);
    }
    return NULL;
}

int main(){
    List * head;
    int a;
    printf("a = ");
    scanf("%d", &a);
    head = create(a);
    while (True){
        printf("a = ");
        scanf("%d", &a);
        if (a != 0)
            add_element_end(a, head);
        else
            break;
    }
    print(head);
    printf("Delete x = ");
    scanf("%d", &a);
    head = remove_element(a, head);
    print(head);
    head = remove_all(head);
    return 0;
}
