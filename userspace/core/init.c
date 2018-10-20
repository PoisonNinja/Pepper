#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <termios.h>
#include <unistd.h>

extern int init_module(void* module_image, unsigned long len,
                       const char* param_values);
extern int delete_module(const char* name, int flags);

int main(int argc, char** argv)
{
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

    mknod("/dev/hda", 0644 | S_IFBLK, makedev(0, 0));
    buffer  = malloc(1024);
    int hda = open("/dev/hda", O_RDWR);
    lseek(hda, 1030, SEEK_SET);
    read(hda, buffer, 1024);
    uint32_t* blocks = (uint32_t*)buffer;
    printf("%08X %08X %08X %08X\n", blocks[0], blocks[1], blocks[2], blocks[3]);
    free(buffer);
    close(hda);

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
