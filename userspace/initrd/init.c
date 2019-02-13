#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

extern int init_module(void* module_image, unsigned long len,
                       const char* param_values);
extern int delete_module(const char* name, int flags);

int main(int argc, char** argv, char** envp)
{
    // Mount a new instance of tmpfs on /dev so it'll be easier to move later
    mount("tmpfs", "/dev", "tmpfs", 0, NULL);

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

    struct stat st;
    int ahci = open("/lib/modules/ahci.ko", O_RDONLY);
    fstat(ahci, &st);
    char* buffer = malloc(st.st_size);
    memset(buffer, 0, st.st_size);
    read(ahci, buffer, st.st_size);
    init_module(buffer, st.st_size, "");
    free(buffer);
    close(ahci);
    mknod("/dev/sda", 0644 | S_IFBLK, makedev(0, 0));

    // Mount the root filesystem
    mkdir("/root", 0755);

    printf("Mounting root filesystem...\n");
    mount("/dev/sda", "/root", "ext2", 0, NULL);

    printf("Moving /dev to new root filesystem...\n");
    // mount("/dev", "/root/dev", "", MS_MOVE, NULL);

    // Execute the real init
    execve("/root/sbin/init", argv, envp);
}