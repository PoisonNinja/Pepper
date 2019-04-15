#include <stdio.h>

int main(int argc, char* argv[])
{
    while (1) {
        printf("sh> ");
        char buffer[1024];
        while (1) {
            fgets(buffer, 1024, stdin);
        }
    }
}
