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

extern int init_module(void* module_image, unsigned long len,
                       const char* param_values);
extern int delete_module(const char* name, int flags);

int main(int argc, char** argv)
{
    // Mount a new instance of tmpfs on /dev so it'll be easier to move later
    mount("tmpfs", "/dev", "tmpfs", 0, NULL);

    // Create the terminal device
    // Eventually this will be handled by our own udev implementation, but
    // for now init will be responsible for initializing it
    mknod("/dev/fb", 0644 | S_IFCHR, makedev(1, 0));
    mknod("/dev/keyboard", 0644 | S_IFCHR, makedev(2, 0));
    mknod("/dev/ptmx", 0644 | S_IFCHR, makedev(5, 0));

    int test = open("/lib/modules/test.ko", O_RDONLY);
    struct stat st;
    fstat(test, &st);
    char* buffer = malloc(st.st_size);
    read(test, buffer, st.st_size);
    init_module(buffer, st.st_size, "");
    delete_module("test", 0);
    free(buffer);
    close(test);

    int ahci = open("/lib/modules/ahci.ko", O_RDONLY);
    fstat(ahci, &st);
    buffer = malloc(st.st_size);
    memset(buffer, 0, st.st_size);
    read(ahci, buffer, st.st_size);
    init_module(buffer, st.st_size, "");
    free(buffer);
    close(ahci);
    mknod("/dev/sda", 0644 | S_IFBLK, makedev(0, 0));

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
