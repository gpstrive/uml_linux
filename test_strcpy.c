#include <stdio.h>
char *str="hello,world\n";
int main(int argc,int *argv[]){
    char cpystr[20];
    strcpy(cpystr,str);
    printf("%s",cpystr);
    return 0;
}

