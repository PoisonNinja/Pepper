#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
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
    mknod("/dev/ptmx", 0644 | S_IFCHR, makedev(5, 0));
    mknod("/dev/tty1", 0644 | S_IFCHR, makedev(4, 1));
    mknod("/dev/tty2", 0644 | S_IFCHR, makedev(4, 2));

    open("/dev/tty2", O_RDONLY); // stdin
    open("/dev/tty2", O_WRONLY); // stdout
    open("/dev/tty2", O_WRONLY); // stderr

    int ret = fork();
    if (!ret) {
        char* const startup_argv[] = {
            "/usr/bin/sh",
            0,
        };
        char* const envp[] = {
            0,
        };
        execve("/usr/bin/sh", startup_argv, envp);
    }
    int status = 0;
    for (;;) {
        wait(&status);
    }
}
