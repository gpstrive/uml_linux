#include <stdio.h>
struct test{
    int a;
    int b;
    int c;
};
int main()
{
    struct test test[20];
    printf("sizeof(test)=%d\n",sizeof(test));
    return 0;
}
