#include <stdio.h>
int a[][3]={{1,2,3},{4,5,6}};
int hell()
{
    int i=0;
    int j=0;
    printf("hello\n");
    printf("a=%d\n",&a);
    for( i=0;i<2;i++)
      for(j=0;j<3;j++)
        printf("a[%d][%d]=%d\n",i,j,a[i][j]);
    return 0;
}
