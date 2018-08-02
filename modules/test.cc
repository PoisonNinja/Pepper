#include <kernel.h>
#include <kernel/init.h>
#include <lib/printf.h>
#include <lib/string.h>

extern void h(int a);

constexpr size_t panic_max = 1024;

static char panic_buffer[panic_max];

extern "C" {
int init()
{
    // h(5);
    // String::memset(panic_buffer, 0, panic_max);
    // snprintf(panic_buffer, 1024, "Hello");
    Log::printk(Log::LogLevel::INFO, "Hello from a module!\n");
    return 0;
}
}
