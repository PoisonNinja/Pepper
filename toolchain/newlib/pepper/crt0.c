#include <stdlib.h>

extern int main(int argc, char** argv);
extern char** environ;

void _start(int argc, char** argv, int envc, char** envp)
{
    environ = envp;
    int ex = main(argc, argv);
    exit(ex);
}
