#include <stdio.h>
#include <pthread.h>
struct gp{
    int ID;
    int name;
};
int i=0;
int timer()
{
    while(1)
    {
        sleep(1);
        i++;
    }
}
int main(void)
{

    struct gp l;
    pthread_t thread;
    int value=i;
    pthread_create(&thread,NULL,timer,NULL);
    printf("hello world !\n");
    while(1)
    {
        while(i < value);
        value=i+8;
        printf("i=%d\n",i);
    }


        

    return 0;
}
