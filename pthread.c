#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
int hello()
{
    while(1)
    {
        sleep(10);
        printf("hello\n");
    }
    return 0;
}
int main()
{
    pthread_t thread;
    pthread_create(&thread,NULL,hello,NULL);
    while(1)
    {
        sleep(1);
    }
    return 0;
}
