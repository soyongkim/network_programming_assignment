#include <stdio.h>

int main() {
    int a;
    char b;
    short c;

    a = 2;

    b = 255;

    c = 32768;

    // for(int i=0; i<4; i++) {
    //     printf("%d\n", a << i);
    // }

    // printf("%d\n", 256 << 8);
    // printf("%d\n", 256 << 8);



    printf("%.8x %d %c\n", b, b, b);
    printf("%.8x %d\n", c, c);
    printf("%.16x %d\n", c, c);
    // printf("%.8x %d\n", b << 8, b << 8);
    // printf("%.8x %d\n", b << 16, b << 16);
    // printf("%.8x %d\n", b << 24, b << 24);

    return 0;
}