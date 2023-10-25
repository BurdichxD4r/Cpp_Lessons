/* TODO: Используя директиву #define, напишите именованную константу,
	возвращающую число секунд в году. Високосным годом – пренебречь.
*/

#include <stdio.h>
#define SECONDS_PER_YEAR 31536000

int main()
{
    printf("Секунд в году %d\n", SECONDS_PER_YEAR);
    return 0;
}
