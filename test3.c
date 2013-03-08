#include <stdio.h>
extern int **a;
extern int hell();
int main()
{
    int i;
    int j;
    hell();
    printf("main\n");
    printf("&a=%d\n",&a);
    printf("a=%d\n",a);

    for(i=0;i<2;i++)
      for(j=0;j<3;j++)
        printf("a[%d][%d]=%d\n",i,j,a[i][j]);
    return 0;
}
