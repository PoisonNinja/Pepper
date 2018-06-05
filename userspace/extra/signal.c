#include <signal.h>
#include <stdio.h>

int main(int argc, char** argv, char** envp)
{
    printf("[signal] Dispatching SIGALRM to init...\n");
    kill(1, SIGALRM);
    return 0;
}
