#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    // Create the terminal device
    // Eventually this will be handled by our own udev implementation, but
    // for now init will be responsible for initializing it
    mknod("/dev/tty", 0644 | S_IFCHR, makedev(0, 0));
    mknod("/dev/keyboard", 0644 | S_IFCHR, makedev(1, 0));
    open("/dev/keyboard", O_RDONLY);  // stdin
    open("/dev/tty", O_WRONLY);       // stdout
    open("/dev/tty", O_WRONLY);       // stderr
    printf("[init] Hello from userspace!\n");
}
