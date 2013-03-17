#include <stdio.h>
int main(int argc, char *argv[]){
    int a,b;
    int num;
    num=sscanf(argv[1],"%02x:%02x",&a,&b);
    printf("%02x:%02x,num=%d\n",a,b,num);
    return 0;
}

