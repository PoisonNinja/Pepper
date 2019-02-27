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
    int ret = fork();
    if (!ret) {
        char* const startup_argv[] = {
            "/sbin/terminal",
            "/usr/bin/hello",
        };
        char* const envp[] = {};
        execve("/sbin/terminal", startup_argv, envp);
    }
    return 0;
}
