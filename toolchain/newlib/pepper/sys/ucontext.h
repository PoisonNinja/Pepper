#ifndef _SYS_UCONTEXT_H_
#define _SYS_UCONTEXT_H_

#include <signal.h>
#include <sys/mcontext.h>

typedef struct ucontext {
    struct ucontext* uc_link;
    sigset_t uc_sigmask;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
} ucontext_t;

#endif