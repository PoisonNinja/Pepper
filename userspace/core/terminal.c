#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../lib/kb.h"

#define TIOCGPTN 0x30

const int VGA_HEIGHT = 25;
const int VGA_WIDTH  = 80;

extern int ioctl(int fd, unsigned long request, void* argp);

uint16_t internal_buffer[80 * 25];

static inline void outb(uint16_t port, uint8_t v)
{
    asm volatile("outb %0,%1" : : "a"(v), "dN"(port));
}
static inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile("inb %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

void update_cursor(int col, int row)
{
    uint16_t position = (row * VGA_WIDTH) + col;

    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void enable_cursor()
{
    outb(0x3D4, 0x09); // set maximum scan line register to 15
    outb(0x3D5, 0x0F);

    outb(0x3D4, 0x0B); // set the cursor end line to 15
    outb(0x3D5, 0x0F);

    outb(0x3D4,
         0x0A); // set the cursor start line to 14 and enable cursor visibility
    outb(0x3D5, 0x0E);
}

int main(int argc, char* argv[])
{
    int kb = open("/dev/keyboard", O_RDONLY); // stdin
    open("/dev/fb", O_WRONLY);                // stdout
    open("/dev/fb", O_WRONLY);                // stderr

    int x = 0, y = 0;
    int bg = 0;
    int fg = 15;

    for (int i = 0; i < 80 * 25; i++) {
        internal_buffer[i] = 15 << 8;
    }

    int ptm    = open("/dev/ptmx", O_RDWR);
    int pts_no = -1;
    ioctl(ptm, TIOCGPTN, &pts_no);
    char pts_path[128];
    sprintf(pts_path, "/dev/pts/%d", pts_no);
    int pts = open(pts_path, O_RDWR);

    int ret = fork();
    if (ret) {
        enable_cursor();
        char buffer[128];
        struct pollfd pfds[2];
        pfds[0].fd     = kb;
        pfds[0].events = POLLIN;
        pfds[1].fd     = ptm;
        pfds[1].events = POLLIN;
        struct kb_state kb_state;
        while (1) {
            int i = poll(pfds, 2, -1);
            if (i < 0) {
                perror("Exception occurred when polling\n");
                exit(EXIT_FAILURE);
            }
            if (pfds[0].revents & POLLIN) {
                // Keyboard event
                read(kb, buffer, 1);
                struct kb_result kb_result;
                int ret = kb_parse(&kb_state, &kb_result, buffer[0]);
                if (ret) {
                    buffer[0] = kb_result.result;
                    if (buffer[0] == '\b') {
                        // Translate backspace into kill character
                        buffer[0] = 0x7F;
                    }
                    write(ptm, buffer, 1);
                }
            }
            if (pfds[0].revents & POLLHUP) {
                /*
                 * Wow, how does a keyboard hang up, but oh well got to handle
                 * it anyways
                 */
                perror("Keyboard hung up somehow\n");
                exit(EXIT_FAILURE);
            }
            if (pfds[1].revents & POLLIN) {
                read(ptm, buffer, 1);
                char val = buffer[0];
                if (val == '\n') {
                    x = 0;
                    y++;
                } else if (val == '\r') {
                    x = 0;
                } else if (val == '\b') {
                    if (x)
                        x--;
                } else if (val == '\e') {
                    read(ptm, buffer, 3);
                    val = buffer[2];
                    if (buffer[1] == '3') {
                        switch (val) {
                            case '0':
                                fg = 0; // Black
                                break;
                            case '1':
                                fg = 4; // Red
                                break;
                            case '2':
                                fg = 2; // Green
                                break;
                            case '3':
                                fg = 14; // Yellow
                                break;
                            case '4':
                                fg = 1; // Blue
                                break;
                            case '5':
                                fg = 5; // Magenta
                                break;
                            case '6':
                                fg = 3; // Cyan
                                break;
                            case '7':
                                fg = 7; // Light gray
                                break;
                        }
                    } else if (buffer[1] == '9') {
                        switch (val) {
                            case '0':
                                fg = 8; // Dark gray
                                break;
                            case '1':
                                fg = 12; // Light Red
                                break;
                            case '2':
                                fg = 10; // Light Green
                                break;
                            case '3':
                                fg = 14; // Yellow
                                break;
                            case '4':
                                fg = 9; // Light blue
                                break;
                            case '5':
                                fg = 13; // Light Magenta
                                break;
                            case '6':
                                fg = 11; // Light Cyan
                                break;
                            case '7':
                                fg = 15; // White
                                break;
                        }
                    } else if (buffer[1] == '4') {
                        switch (val) {
                            case '0':
                                bg = 0; // Black
                                break;
                            case '1':
                                bg = 4; // Red
                                break;
                            case '2':
                                bg = 2; // Green
                                break;
                            case '3':
                                bg = 14; // Yellow
                                break;
                            case '4':
                                bg = 1; // Blue
                                break;
                            case '5':
                                bg = 5; // Magenta
                                break;
                            case '6':
                                bg = 3; // Cyan
                                break;
                            case '7':
                                bg = 7; // Light gray
                                break;
                        }
                    } else if (buffer[1] == '7') {
                        // read(ptm, buffer, 1);
                        // val = buffer[0];
                        switch (val) {
                            case '0':
                                bg = 8; // Dark gray
                                break;
                            case '1':
                                bg = 12; // Light Red
                                break;
                            case '2':
                                bg = 10; // Light Green
                                break;
                            case '3':
                                bg = 14; // Yellow
                                break;
                            case '4':
                                bg = 9; // Light blue
                                break;
                            case '5':
                                bg = 13; // Light Magenta
                                break;
                            case '6':
                                bg = 11; // Light Cyan
                                break;
                            case '7':
                                bg = 15; // White
                                break;
                        }
                    }
                    read(ptm, buffer, 1);
                } else {
                    size_t index           = y * VGA_WIDTH + x++;
                    uint8_t color          = ((0xF & bg) << 4) | (0xF & fg);
                    internal_buffer[index] = val | (color << 8);
                }
                if (x == VGA_WIDTH) {
                    x = 0;
                    y++;
                }
                if (y == VGA_HEIGHT) {
                    for (int ny = 1; ny < VGA_HEIGHT; ny++) {
                        memcpy((void*)&(internal_buffer[(ny - 1) * VGA_WIDTH]),
                               (void*)&(internal_buffer[ny * VGA_WIDTH]),
                               2 * VGA_WIDTH);
                    }
                    memset(
                        (void*)&(internal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH]),
                        0, 2 * VGA_WIDTH);
                    y = VGA_HEIGHT - 1;
                }
                lseek(1, 0, SEEK_SET);
                write(1, (uint8_t*)internal_buffer, sizeof(internal_buffer));
                update_cursor(x, y);
            }
            if (pfds[1].revents & POLLHUP) {
                perror("Program exited\n");
                // TODO: Grab exit code from child
                exit(EXIT_FAILURE);
            }
        }
    } else {
        dup2(0, pts);
        dup2(1, pts);
        dup2(2, pts);

        close(ptm);
        close(pts);

        char* const program = (argc > 1) ? argv[1] : "/usr/bin/sh";

        char* const new_argv[] = {
            program,
            NULL,
        };

        char* const new_envp[] = {
            "hello=world",
            NULL,
        };

        execve(argv[1], new_argv, new_envp);
    }
}
