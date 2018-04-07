#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    open("/dev/tty", O_RDONLY);  // stdin
    open("/dev/tty", O_WRONLY);  // stdout
    open("/dev/tty", O_WRONLY);  // stderr
    printf("Hello from userspace!\n");
    printf("%d arguments passed in\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("#%d: %s\n", i, argv[i]);
    }
    pid_t pid = fork();
    printf("My PID is %llX\n", pid);
}
