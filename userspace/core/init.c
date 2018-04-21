#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
