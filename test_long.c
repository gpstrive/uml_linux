#include <stdio.h>
#include <stdlib.h>

#define BASE  0xa800000100000000
#define END  0xa800000013FFFFFFE

#define BASE3 0xa80000013FFFFFF0
#define END3 0xa800000013FFFFFFE

int main()
{
      unsigned long long addr=BASE3;
      printf("addr=%llx  end=%llx\n",addr,END3);

      addr += 1000000;
      printf("addr=%llx  end=%llx\n",addr,END3);
      if(addr > END3)
        printf("xxxxxxxxxxxxxxxxxxxxxxxxxxx");
      if(addr > BASE3)
        printf("yyyyyyyyyyyyyyyyyyyyyyyyyyyy");
}

