#pragma once

#include <stdint.h>

#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_stat 4
#define SYS_fstat 5
#define SYS_lseek 8
#define SYS_mmap 9
#define SYS_mprotect 10
#define SYS_munmap 11
#define SYS_fork 57
#define SYS_exit 60

extern uint64_t syscall(uint64_t num, uint64_t a, uint64_t b, uint64_t c,
                        uint64_t d, uint64_t e);
