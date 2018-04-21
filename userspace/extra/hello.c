#include <stdio.h>

int main(int argc, char** argv, char** envp)
{
    printf("[hello] Hello world!\n");
    printf("[hello] %d arguments passed in\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("[hello] #%d: %s\n", i, argv[i]);
    }
    printf("[hello] Printing out environment:\n");
    for (int i = 0; envp[i]; i++) {
        printf("[hello] %s\n", envp[i]);
    }
    return 123;
}
