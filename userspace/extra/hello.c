#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WHITE "\e[77m "
#define GRAY "\e[70m "
#define RED "\e[41m "
#define CLEAR "\e[40m"

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

    // uint8_t* buffer = malloc(1024);
    // int hda         = open("/dev/hda", O_RDWR);
    // lseek(hda, 1030, SEEK_SET);
    // read(hda, buffer, 1024);
    // uint32_t* blocks = (uint32_t*)buffer;
    // printf("%08X %08X %08X %08X\n", blocks[0], blocks[1], blocks[2],
    // blocks[3]); free(buffer); close(hda);

    printf("\nHello MIT! This is all rendered using the userspace terminal!\n");

    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", WHITE,
           WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE, WHITE);
    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", WHITE,
           WHITE, RED, RED, WHITE, WHITE, RED, RED, WHITE, WHITE, RED, RED,
           WHITE, WHITE, RED, RED, WHITE, WHITE, RED, RED, RED, RED, RED, RED,
           WHITE, WHITE);
    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", WHITE,
           WHITE, RED, RED, WHITE, WHITE, RED, RED, WHITE, WHITE, RED, RED,
           WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE);
    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", WHITE,
           WHITE, RED, RED, WHITE, WHITE, RED, RED, WHITE, WHITE, RED, RED,
           WHITE, WHITE, GRAY, GRAY, WHITE, WHITE, RED, RED, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE);
    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", WHITE,
           WHITE, RED, RED, WHITE, WHITE, RED, RED, WHITE, WHITE, RED, RED,
           WHITE, WHITE, GRAY, GRAY, WHITE, WHITE, RED, RED, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE);
    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", WHITE,
           WHITE, RED, RED, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, RED, RED,
           WHITE, WHITE, GRAY, GRAY, WHITE, WHITE, RED, RED, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE);
    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", WHITE,
           WHITE, RED, RED, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, RED, RED,
           WHITE, WHITE, GRAY, GRAY, WHITE, WHITE, RED, RED, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE);
    printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", WHITE,
           WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
           WHITE, WHITE, WHITE, WHITE, WHITE);
    return 123;
}
