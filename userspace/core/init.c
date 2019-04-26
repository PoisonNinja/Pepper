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
    // Mount ptsfs
    mkdir("/dev/pts", 0755);
    mount("ptsfs", "/dev/pts", "ptsfs", 0, NULL);

    /*
     * Create the terminal device
     *
     * Eventually this will be handled by our own udev implementation, but now
     * init will be responsible for initializing it instead.
     */
    mknod("/dev/fb", 0644 | S_IFCHR, makedev(1, 0));
    mknod("/dev/keyboard", 0644 | S_IFCHR, makedev(2, 0));
    mknod("/dev/ptmx", 0644 | S_IFCHR, makedev(5, 0));

    int ret = fork();
    if (!ret) {
        char* const startup_argv[] = {
            "/sbin/terminal",
            "/usr/bin/sh",
            0,
        };
        char* const envp[] = {
            0,
        };
        execve("/sbin/terminal", startup_argv, envp);
    }
    for (;;) {
        // TODO: Are we supposed to call waitpid() here to reap?
    }
}
