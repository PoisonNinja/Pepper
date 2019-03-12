#include <stdio.h>

int main(int argc, char* argv[])
{
    while (1) {
        printf("sh> ");
        while (1) {
            char c = fgetc(stdin);
            if (c == '\n') {

            } else {
                printf("%c", c);
            }
        }
    }
}