#include <stdio.h>
#include "myheader.h"

#define PI 3.14
#define SQUARE(x) ((x) * (x))
#define ADD(a, b) ((a) + (b))

/* program to test
   mypreprocessor */
int main() {
    int r = 5; // radius
    float area = PI * SQUARE(r);

    printf("PI = %f\n", PI);
    printf("Area = %f\n", area);
    printf("Sum = %d\n", ADD(10, 20));
    printf("Max = %d\n", MAX); // MAX is in myheader.h
    return 0;
}
