#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

//hard coding read after the module installed
#define KERNEL_PHY_ADDR  0x23bb4000

int main()
{

    char *buf;
    int fd;
    unsigned long phy_addr;
    int  pagesize = getpagesize();

    phy_addr=KERNEL_PHY_ADDR;

    fd=open("/dev/mem",O_RDWR);

    if(fd == -1)
      perror("open");

    buf=mmap(0, pagesize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr);

    if(buf == MAP_FAILED)
    {
        perror("mmap");
    }

    printf("buf : %s\n",buf);

    // test the write 
    buf[0] = 'X';

    munmap(buf,pagesize);
    close(fd);
    return 0;

}
