#include <stdlib.h>

extern int main(int argc, char** argv, char** envp);
extern char** environ;

void _start(int argc, char** argv, char** envp)
{
    environ = envp;
    int ex = main(argc, argv, envp);
    exit(ex);
}
