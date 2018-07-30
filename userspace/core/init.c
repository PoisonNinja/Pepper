#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/ucontext.h>
#include <unistd.h>

static int got_alarm = 0;
__thread int a = 0;
__thread int b = 3;

void handler(int signum, siginfo_t* siginfo, void* ucontext)
{
    (void)signum;
    (void)siginfo;
    (void)ucontext;
    got_alarm = 1;
}

int main(int argc, char** argv)
{
    // Create the terminal device
    // Eventually this will be handled by our own udev implementation, but
    // for now init will be responsible for initializing it
    mknod("/dev/tty", 0644 | S_IFCHR, makedev(0, 0));
    mknod("/dev/keyboard", 0644 | S_IFCHR, makedev(1, 0));
    open("/dev/keyboard", O_RDONLY);  // stdin
    open("/dev/tty", O_WRONLY);       // stdout
    open("/dev/tty", O_WRONLY);       // stderr
    printf("[init] Hello from userspace!\n");
    printf("[init] %d arguments passed in\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("[init] #%d: %s\n", i, argv[i]);
    }
    printf("Thread local variables: %d %d\n", a, b);
    pid_t pid = fork();
    if (pid) {
        printf("[init] parent: My child's PID is %d\n", pid);
        printf("[init] Starting up signal test binary: \n");
        pid = fork();
        if (!pid) {
            printf("[init] Second child, preparing to launch...\n");
            char* argv[] = {
                "/sbin/signal",
                (char*)0,
            };
            char* envp[] = {
                "hello=world",
                (char*)0,
            };
            execve("/sbin/signal", argv, envp);
        } else {
            char* stack = malloc(4096);
            stack_t ss = {.ss_size = 4096, .ss_sp = stack, .ss_flags = 0};
            printf("[init] Registering signal handler...\n");
            struct sigaction sa = {
                .sa_sigaction = &handler,
                .sa_mask = 0,
                .sa_flags = SA_ONSTACK | SA_SIGINFO,
            };
            sigaltstack(&ss, NULL);
            sigaction(SIGALRM, &sa, NULL);
            printf("[init] Registered signal handler\n");
            printf("[init] Awaiting SIGALRM...\n");
            while (!got_alarm)
                ;
            printf("[init] Alarm recieved! Exiting!\n");
            return 0;
        }
    } else {
        printf("[init] child: I am the child! My PID is %d\n", getpid());
        printf("[init] child: Preparing to exec...\n");
        char* argv[] = {
            "/sbin/hello",
            (char*)0,
        };
        char* envp[] = {
            "hello=world",
            (char*)0,
        };
        execve("/sbin/hello", argv, envp);
    }
}
