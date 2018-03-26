#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mmap_wrapper {
    void *addr;
    size_t length;
    int prot;
    int flags;
    int fd;
    off_t offset;
};

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int munmap(void *addr, size_t length);

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

#define MAP_SHARED 0x001
#define MAP_PRIVATE 0x002
#define MAP_FIXED 0x010
#define MAP_FILE 0x000
#define MAP_ANONYMOUS 0x020
#define MAP_ANON MAP_ANONYMOUS

#define MAP_FAILED (void *)-1

#ifdef __cplusplus
}
#endif
