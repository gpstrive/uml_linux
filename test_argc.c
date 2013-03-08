#include <stdio.h>
int main(int argc ,char *argv[]){
    int i=0;
    printf("argc=%d\n",argc);
    char *my_argv[]={
        "hello","just","for","a","test"};
    printf("sizeof(my_argv)=%d,sizeof(my_argv[0])=%d\n",sizeof(my_argv),sizeof(my_argv[0]));
    char **pmy_argv=my_argv;
    pmy_argv++;
    printf("pmy_argv[0]=%s\n",*pmy_argv);
    for (i=0;i<argc;i++)
    {
        printf("%s\n",argv[i]);
    }
    return 0;
}

