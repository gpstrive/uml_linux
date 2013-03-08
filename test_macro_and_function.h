#include <stdio.h>
void fun();

#ifdef WIN32
#define fun() do{ } while(0) 
#endif

