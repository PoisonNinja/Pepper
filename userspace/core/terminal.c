#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define TIOCGPTN 0x30

const int VGA_HEIGHT = 25;
const int VGA_WIDTH  = 80;

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

int main(int argc, char* argv[])
{
    open("/dev/keyboard", O_RDONLY); // stdin
    open("/dev/fb", O_WRONLY);       // stdout
    open("/dev/fb", O_WRONLY);       // stderr

    int x = 0, y = 0;
    int color = 15;

    for (int i = 0; i < 80 * 25; i++) {
        internal_buffer[i] = 15 << 8;
    }

    int ptm    = open("/dev/ptmx", O_RDWR);
    int pts_no = -1;
    ioctl(ptm, TIOCGPTN, &pts_no);
    char pts_path[128];
    sprintf(pts_path, "/dev/pts/pts%d", pts_no);
    int pts = open(pts_path, O_RDWR);

    int ret = fork();
    if (ret) {
        char buffer[128];
        while (1) {
            ssize_t ret = read(ptm, buffer, 1);
            char val    = buffer[0];
            if (val == '\n') {
                x = 0;
                y++;
            } else if (val == '\e') {
                read(ptm, buffer, 3);
                val = buffer[2];
                if (buffer[1] == '3') {
                    switch (val) {
                        case '0':
                            color = 0; // Black
                            break;
                        case '1':
                            color = 4; // Red
                            break;
                        case '2':
                            color = 2; // Green
                            break;
                        case '3':
                            color = 14; // Yellow
                            break;
                        case '4':
                            color = 1; // Blue
                            break;
                        case '5':
                            color = 5; // Magenta
                            break;
                        case '6':
                            color = 3; // Cyan
                            break;
                        case '7':
                            color = 7; // Light gray
                            break;
                        default:
                            color = 15;
                            break;
                    }
                } else {
                    switch (val) {
                        case '0':
                            color = 8; // Dark gray
                            break;
                        case '1':
                            color = 12; // Light Red
                            break;
                        case '2':
                            color = 10; // Light Green
                            break;
                        case '3':
                            color = 14; // Yellow
                            break;
                        case '4':
                            color = 9; // Light blue
                            break;
                        case '5':
                            color = 13; // Light Magenta
                            break;
                        case '6':
                            color = 11; // Light Cyan
                            break;
                        case '7':
                            color = 15; // White
                            break;
                        default:
                            color = 15;
                            break;
                    }
                }
                read(ptm, buffer, 1);
            } else {
                size_t index           = y * VGA_WIDTH + x++;
                internal_buffer[index] = val | (color << 8);
            }
            if (x == VGA_WIDTH) {
                x = 0;
                y++;
            }
            if (y == VGA_HEIGHT) {
                for (int ny = 1; ny < VGA_HEIGHT; ny++) {
                    memcpy((void*)&internal_buffer[(ny - 1) * VGA_WIDTH],
                           (void*)&internal_buffer[ny * VGA_WIDTH],
                           2 * VGA_WIDTH);
                }
                memset((void*)&internal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH], 0,
                       2 * VGA_WIDTH);
                y = VGA_HEIGHT - 1;
            }
            lseek(1, 0, SEEK_SET);
            write(1, (uint8_t*)internal_buffer, sizeof(internal_buffer));
            update_cursor(x, y);
        }
    } else {
        dup2(0, pts);
        dup2(1, pts);
        dup2(2, pts);

        close(ptm);
        close(pts);

        char* const envp[] = {
            NULL,
        };

        execve(argv[1], argv, envp);
    }
}