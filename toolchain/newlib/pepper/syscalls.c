#include <stdarg.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
    struct mmap_wrapper wrapper = {
        .addr = addr,
        .length = len,
        .prot = prot,
        .flags = flags,
        .fd = fildes,
        .offset = off,
    };
    return syscall(SYS_mmap, &wrapper, 0, 0, 0, 0);
}

int munmap(void *addr, size_t length)
{
    return syscall(123, 0, 0, 0, 0, 0);
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

int execve(char *name, char **argv, char **env)
{
    return syscall(SYS_execve, name, argv, env, 0, 0);
}

pid_t fork()
{
    return syscall(SYS_fork, 0, 0, 0, 0, 0);
}

int fstat(int file, struct stat *st)
{
    return syscall(SYS_fstat, file, st, 0, 0, 0);
}

int getpid()
{
    return syscall(SYS_getpid, 0, 0, 0, 0, 0);
}

int isatty(int file)
{
    // Always return as a TTY
    return 1;
}
int kill(pid_t pid, int sig)
{
    return syscall(SYS_kill, pid, sig, 0, 0, 0);
}

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

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    return syscall(SYS_sigaction, signum, act, oldact, 0, 0);
}

int sigpending(sigset_t *set)
{
    return syscall(SYS_sigpending, set, 0, 0, 0, 0);
}

int sigaltstack(const stack_t *ss, stack_t *oldss)
{
    return syscall(SYS_sigaltstack, ss, oldss, 0, 0, 0);
}
