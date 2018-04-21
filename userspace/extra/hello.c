#include <stdio.h>

int main(int argc, char** argv)
{
    printf("Hello world!\n");
    printf("My argv and envp:\n");
    printf("%d arguments passed in\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("#%d: %s\n", i, argv[i]);
    }
    return 123;
}
