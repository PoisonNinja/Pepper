#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int test = open("/lib/modules/test.ko", O_RDONLY);
    struct stat st;
    fstat(test, &st);
    char* buffer = malloc(st.st_size);
    read(test, buffer, st.st_size);
    init_module(buffer, st.st_size, "");
    delete_module("test", 0);
    free(buffer);
    close(test);

    int ret = fork();
    if (!ret) {
        char* const startup_argv[] = {
            "/root/sbin/terminal",
            "/root/usr/bin/hello",
        };
        char* const envp[] = {};
        execve("/root/sbin/terminal", startup_argv, envp);
    }

    return 0;
}
