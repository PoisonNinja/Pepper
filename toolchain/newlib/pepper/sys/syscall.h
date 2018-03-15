#define SYS_exit 60

static long syscall(int num, long a, long b, long c, long d, long e)
{
    long x;
    __asm__ __volatile__("int $0x80"
                         : "=a"(x)
                         : "0"(num), "r"(a), "c"(b), "d"(c), "S"(d), "D"(e));
    return x;
}
