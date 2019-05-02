#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    size_t buffer_size = 1024;
    char* buffer       = malloc(buffer_size);
    while (1) {
        printf("$ ");
        size_t ret = getline(&buffer, &buffer_size, stdin);
        if (ret == 1 && buffer[0] == '\n')
            continue;
        buffer[ret - 1] = '\0';
        int fd          = open(buffer, O_RDONLY);
        if (fd < 0) {
            printf("File not found\n");
        } else {
            close(fd);
            int pid;
            if ((pid = fork())) {
                waitpid(pid, NULL, 0);
            } else {
                char* const child_argv[] = {
                    buffer,
                    0,
                };
                execv(buffer, child_argv);
            }
        }
    }
    free(buffer);
}
