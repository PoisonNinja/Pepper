#include <stdarg.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

void _exit(int rc)
{
    syscall(SYS_exit, rc, 0, 0, 0, 0);
}
int close(int file);
char **environ; /* pointer to array of char * strings that define the current
                   environment variables */
int execve(char *name, char **argv, char **env);
int fork();
int fstat(int file, struct stat *st);
int getpid();
int isatty(int file);
int kill(int pid, int sig);
int link(char *old, char *new);
int lseek(int file, int ptr, int dir);
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
int read(int file, char *ptr, int len);
caddr_t sbrk(int incr);
int stat(const char *file, struct stat *st);
clock_t times(struct tms *buf);
int unlink(char *name);
int wait(int *status);
int write(int file, char *ptr, int len);
int gettimeofday(struct timeval *__restrict__ p, void *__restrict__ tz);
