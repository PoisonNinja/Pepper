#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    size_t buffer_size = 1024;
    char* buffer       = malloc(buffer_size);
    while (1) {
        printf("sh> ");
        getline(&buffer, &buffer_size, stdin);
        printf("%s\n", buffer);
    }
    free(buffer);
}
