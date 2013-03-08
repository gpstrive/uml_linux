#include <stdio.h>
struct anony{
    int comm;
    int value;
};
int main()
{
    struct anony anon = {
        .comm=1,
        .value=1,
    };
    return 0;
}
