#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

int main(int argc, char** argv)
{
    int fd = open("/sbin/init", O_RDONLY, 0);
    if (argc == 2 && !strcmp(argv[1], "test")) {
        syscall(42, 1, 2, 3, 4, 5);
    }
    if (fd == 0) {
        syscall(20, 0xDE, 0xAD, 0xBE, 0xEE, 0xEF);
    }
    printf("%s\n", "Hello world\n");
}
