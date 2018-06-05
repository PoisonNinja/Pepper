#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int got_alarm = 0;

void handler(int signum)
{
    got_alarm = 1;
}

int main(int argc, char** argv)
{
    open("/dev/tty", O_RDONLY);  // stdin
    open("/dev/tty", O_WRONLY);  // stdout
    open("/dev/tty", O_WRONLY);  // stderr
    printf("[init] Hello from userspace!\n");
    printf("[init] %d arguments passed in\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("[init] #%d: %s\n", i, argv[i]);
    }
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
            printf("[init] Registering signal handler...\n");
            struct sigaction sa = {
                .sa_handler = &handler, .sa_mask = 0, .sa_flags = 0};
            struct sigaction oldsa;
            sigaction(SIGALRM, &sa, &oldsa);
            raise(SIGALRM);
            printf("[init] Registered signal handler\n");
            printf("[init] Awaiting SIGALRM...\n");
            while (!got_alarm)
                printf("[init] No alarm yet\n");
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
