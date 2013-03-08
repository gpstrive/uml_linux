#include <stdio.h>
int main()
{
    int ip=0x10101010;
    int *ipp=&ip;
    printf("%pI4\n",ipp);
    return 0;
}
