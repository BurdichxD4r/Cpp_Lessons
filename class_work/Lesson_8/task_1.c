#include <stdio.h>

int main()
{
    int n;
    int * pn1 = &n;
    char z;
    char * pz = &z;
    float x;
    float * px1 = &x;
    int * pn2 = &n;
    float * px2 = &x;
    float * px3 = &x;
    printf("Целочисленный = %d - %pd\nСимвольный = %c - %pc\nВешественный"
	" = %0.2f - %pf\n", n, pn1, z, pz, x, px1);
    printf("Размеры указателей:\npn1 = %ld\npz = %ld\npx1 = %ld",
	sizeof(pn1), sizeof(pz), sizeof(px1));
    typedef char * PCHAR;
    PCHAR pc1, pc2;
    printf("%pc\n%pc\n", pc1, pc2);
    return 0;
}
