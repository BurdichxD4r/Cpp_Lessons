/* TODO: Придумать эллипсис со своим механизмом контроля вводимых параметров
*/

#include <stdio.h>
#include <stdarg.h>

int multiplication(int n, ...){
    int result = 1;
    va_list factor;
    va_start(factor, n);
    for (int i = 0; i < n; i++){
        result *= va_arg(factor, int);
    }
    va_end(factor);
    return result;
}

int main(){
    printf("%d\n", multiplication(4, 3, 5, 10, 2));
    return 0;
}
