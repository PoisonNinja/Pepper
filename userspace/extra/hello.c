#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char** argv, char** envp)
{
    printf("[hello] Hello world!\n");
    printf("[hello] %d arguments passed in\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("[hello] #%d: %s\n", i, argv[i]);
    }
    printf("[hello] Printing out environment:\n");
    for (int i = 0; envp[i]; i++) {
        printf("[hello] %s\n", envp[i]);
    }
    printf("[hello] We also have color support :)\n");
    for (char a = '1'; a < '8'; a++) {
        printf("\e[3%cmTest \e[9%cmTest\n", a, a);
    }

    uint8_t* buffer = malloc(1024);
    int sda         = open("/dev/sda", O_RDWR);
    lseek(sda, 1024, SEEK_SET);
    read(sda, buffer, 1024);
    uint32_t* blocks = (uint32_t*)buffer;
    printf("%08X %08X %08X %08X\n", blocks[0], blocks[1], blocks[2], blocks[3]);
    free(buffer);
    close(sda);

    return 123;
}
