#include <stdio.h>
char *str="hello,world\n";
int main(int argc,int *argv[]){
    char cpystr[20];
    memcpy(cpystr,str,strlen(str)+1);
    printf("%s",cpystr);
    return 0;
}

