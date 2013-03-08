#include <stdio.h>
#include <stdlib.h>
struct fib_test{
    int id;
    void *point;
//    char point[0];
};
int main()
{
    struct fib_test *ft;
    ft=malloc(sizeof(struct fib_test)+10);
    printf("sizeof(struct fib_test)=%d\n",sizeof(struct fib_test));
    memset(ft->point,0,10);
    printf("memset\n");
    sprintf(ft->point,"for test\n");
    printf("%s\n",ft->point);
    return 0;
}

