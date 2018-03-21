#include <stdarg.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
}

int munmap(void *, int)
{
}

void _exit(int rc)
{
    syscall(SYS_exit, rc, 0, 0, 0, 0);
}
int close(int file)
{
    return syscall(SYS_close, file, 0, 0, 0, 0);
}
char **environ; /* pointer to array of char * strings that define the current
                   environment variables */
int execve(char *name, char **argv, char **env);
int fork();
int fstat(int file, struct stat *st)
{
    return syscall(SYS_fstat, file, st, 0, 0, 0);
}
int getpid();
int isatty(int file)
{
    // Always return as a TTY
    return 1;
}
int kill(int pid, int sig);
int link(char *old, char *new);
int lseek(int file, int offset, int whence)
{
    return syscall(SYS_lseek, file, offset, whence, 0, 0);
}
int open(const char *name, int flags, ...)
{
    va_list argp;
    mode_t mode = 0;
    va_start(argp, flags);
    if (flags & O_CREAT)
        mode = va_arg(argp, mode_t);
    va_end(argp);

    int result = syscall(SYS_open, name, flags, mode, 0, 0);
    return result;
}
ssize_t read(int file, char *ptr, int len)
{
    return syscall(SYS_read, file, ptr, len, 0, 0);
}
caddr_t sbrk(int incr);
int stat(const char *file, struct stat *st)
{
    return syscall(SYS_stat, file, st, 0, 0, 0);
}
clock_t times(struct tms *buf);
int unlink(char *name);
int wait(int *status);
ssize_t write(int file, char *ptr, int len)
{
    return syscall(SYS_write, file, ptr, len, 0, 0);
}
int gettimeofday(struct timeval *__restrict__ p, void *__restrict__ tz);
