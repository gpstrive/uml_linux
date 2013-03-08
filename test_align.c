#include <stdio.h>
struct p
{
    int a;
    char b;
    char c;
}__attribute__((aligned(2)))pp;
int main()
{
    printf("sizeof(int)=%d,sizeof(char)=%d\n",sizeof(int),sizeof(char));
    printf("sizeof(pp)=%d\n",sizeof(pp));
    return 0;
}
