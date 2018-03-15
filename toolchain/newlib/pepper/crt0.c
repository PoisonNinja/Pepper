#include <stdlib.h>

extern int main(int argc, char** argv);

void _start(int argc, char** argv)
{
    int ex = main(argc, argv);
    exit(ex);
}
