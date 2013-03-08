extern void a(char *);
int main(int ac,char **argv)
{
    static char string[]="hello,world\n";
    a(string);
}
