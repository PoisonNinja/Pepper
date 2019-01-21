#include <kernel.h>
#include <kernel/init.h>
#include <kernel/module.h>
#include <lib/printf.h>
#include <lib/string.h>

class Test
{
public:
    Test()
    {
        log::printk(log::log_level::INFO, "Test instance created\n");
    }
};

Test test;

extern "C" {
int init()
{
    log::printk(log::log_level::INFO, "Hello from a module\n");
    return 0;
}

int fini()
{
    log::printk(log::log_level::INFO, "Good bye from a module\n");
    return 0;
}
}

MODULE_NAME("test");
MODULE_AUTHOR("Jason Lu");
MODULE_DESCRIPTION(
    "A simple module to demonstrate capabilities and act as a test bed");
MODULE_VERSION("0.0.1");
