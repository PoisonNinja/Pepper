#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#undef NSIGS
#define NSIGS 32
//\We should probably typedef something like _sigset_t (__sigset_t is used
//already) then #define sigset_t to _sigset_t
#undef sigset_t
#define sigset_t uint32_t

// Same as Linux
#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPWR 30
#define SIGSYS 31

#define SIGEV_NONE 1
#define SIGEV_SIGNAL 2
#define SIGEV_THREAD 3

#define SI_USER 1
#define SI_QUEUE 2
#define SI_TIMER 3
#define SI_ASYNCIO 4
#define SI_MESGQ 5

#define SA_NOCLDSTOP 1
#define SA_ONSTACK 8
#define SA_RESETHAND 16
#define SA_SIGINFO 128

#define SS_ONSTACK 1
#define SS_DISABLE 2

#define SIG_SETMASK 0
#define SIG_BLOCK 1
#define SIG_UNBLOCK 2

typedef void (*_sig_func_ptr)(int);

union sigval {       /* Data passed with notification */
    int sival_int;   /* Integer value */
    void *sival_ptr; /* Pointer value */
};

typedef struct {
    union sigval si_value;
    void *si_addr;
    pid_t si_pid;
    uid_t si_uid;
    int si_signo;
    int si_code;
    int si_errno;
    int si_status;
} siginfo_t;

struct sigaction {
    union {
        void (*sa_handler)(int);
        void (*sa_sigaction)(int, siginfo_t *, void *);
    };
    sigset_t sa_mask;
    int sa_flags;
};

typedef struct {
    void *ss_sp;    /* Base address of stack */
    int ss_flags;   /* Flags */
    size_t ss_size; /* Number of bytes in stack */
} stack_t;

#define sigaddset(what, sig) (*(what) |= (1 << (sig)), 0)
#define sigdelset(what, sig) (*(what) &= ~(1 << (sig)), 0)
#define sigemptyset(what) (*(what) = 0, 0)
#define sigfillset(what) (*(what) = ~(0), 0)
#define sigismember(what, sig) (((*(what)) & (1 << (sig))) != 0)

int kill(pid_t, int);
int sigaction(int, const struct sigaction *, struct sigaction *);
int sigaltstack(const stack_t *, stack_t *);

#ifdef __cplusplus
}
#endif

#ifndef _SIGNAL_H_
#include <signal.h>
#endif
#endif /* _SYS_SIGNAL_H */