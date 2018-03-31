#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    open("/dev/tty", O_RDONLY);  // stdin
    open("/dev/tty", O_WRONLY);  // stdout
    open("/dev/tty", O_WRONLY);  // stderr
    printf("%d arguments passed in\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("#%d: %s\n", i, argv[i]);
    }
}
