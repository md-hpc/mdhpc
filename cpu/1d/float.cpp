#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    float a,b;

    if (argc != 2) {
        b = 1e-15;
    } else {
        b = atof(argv[1]);
    }

    a = 1;
    b = a + b;
    printf("%e\n",b);
    printf("%d\n", a==b);
    return 0;
}
