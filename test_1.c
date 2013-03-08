#include <stdio.h>
int main()
{
    int i;
    i=({2;3;});
    //i=(2;4); // no 
    //i=(2,3); // yes
    //i={2;3;}; // no
    typeof(i) j=3;
    printf("j=%x\n",j);
    3;
    printf("i=%d\n",i);
    return 0;
}
