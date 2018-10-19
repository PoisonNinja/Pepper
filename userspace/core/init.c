#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <termios.h>
#include <unistd.h>

#define TIOCGPTN 0x30

extern int init_module(void* module_image, unsigned long len,
                       const char* param_values);
extern int delete_module(const char* name, int flags);

int main(int argc, char** argv)
{
    // Create the terminal device
    // Eventually this will be handled by our own udev implementation, but
    // for now init will be responsible for initializing it
    mknod("/dev/tty", 0644 | S_IFCHR, makedev(0, 0));
    mknod("/dev/keyboard", 0644 | S_IFCHR, makedev(1, 0));
    mknod("/dev/ptmx", 0644 | S_IFCHR, makedev(5, 0));
    open("/dev/keyboard", O_RDONLY); // stdin
    open("/dev/tty", O_WRONLY);      // stdout
    open("/dev/tty", O_WRONLY);      // stderr
    printf("[init] Hello from userspace!\n");
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

    int ptm    = open("/dev/ptmx", O_RDWR);
    int pts_no = -1;
    ioctl(ptm, TIOCGPTN, &pts_no);
    printf("%d\n", pts_no);
    char pts_path[128];
    sprintf(pts_path, "/dev/pts/pts%d", pts_no);
    int pts = open(pts_path, O_RDWR);
    int a   = fork();
    if (a) {
        write(ptm, "Hello!", 7);
    } else {
        dup2(0, pts);
        char input[128];
        scanf("%s", input);
        printf("Got %s from stdin\n", input);
    }

    return 0;
}
