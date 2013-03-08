#include <stdio.h>
int main()
{
    int a=0x12345678;
    char *ch=(char *)&a;
    int i=0;
    printf("&a=%x\n",&a);
    for(i=0;i<4;i++)
      printf("&ch[%d]=%x,ch[%d]=%x\n",i,&ch[i],i,ch[i]);
    return 0;
}
