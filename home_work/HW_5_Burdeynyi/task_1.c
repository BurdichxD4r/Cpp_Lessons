/* TODO: Напечатать минимальное число, большее 200, которое нацело
	делится на 17.
*/

#include <stdio.h>

int main()
{
    int min_number = 200;
    int divider = 17;
    int remains = min_number % divider;
    printf("Минимальное число, больше %d, которое нацело делится на %d:\n"
        "%d\n", min_number, divider, min_number + (divider - remains));
    return 0;
}
