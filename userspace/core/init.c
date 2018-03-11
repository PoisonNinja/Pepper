#include <string.h>


#define DEFN_SYSCALL0(fn, num) \
int syscall_##fn() \
{ \
 int a; \
 asm volatile("int $0x80" : "=a" (a) : "0" (num)); \
 return a; \
}

DEFN_SYSCALL0(test, 0);

int main(int argc, char** argv)
{
    syscall_test();
    for(;;);
}
