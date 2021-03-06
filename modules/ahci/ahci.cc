#include "ahci.h"
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/module.h>
#include <lib/printf.h>
#include <lib/string.h>

namespace
{
pci::filter ahci_filter[] = {
    // Intel
    {PCI_VDEV(0x8086, 0x2829)}, // ICH8M (VirtualBox)
    {PCI_VDEV(0x8086, 0x2922)}, // ICH9 (QEMU),

    // Others
    {PCI_VDEV(0x15AD, 0x07E0)},    // VMWare
    {PCI_CLASS_IF(0x1, 0x6, 0x1)}, // Generic AHCI device
    {},                            // Null terminator
};
} // namespace

class ahci_driver : public pci::driver
{
public:
    ahci_driver();
    ~ahci_driver() override;
    bool probe(pci::device* dev) override;
    const char* name() override;
    const pci::filter* filt() override;

private:
    dev_t major;
};

ahci_driver::ahci_driver()
{
    this->major = filesystem::locate_class(filesystem::BLK);
    filesystem::register_class(filesystem::BLK, this->major);
}

ahci_driver::~ahci_driver()
{
}

bool ahci_driver::probe(pci::device* dev)
{
    dev->claim();
    ahci_controller* ahci = new ahci_controller(dev, this->major);
    ahci->init();
    return true;
}

const char* ahci_driver::name()
{
    return "ahci";
}

const pci::filter* ahci_driver::filt()
{
    return ahci_filter;
}

namespace
{
ahci_driver ahci;
}

extern "C" {
int init()
{
    log::printk(log::log_level::INFO, "ahci: Registering driver...\n");
    pci::register_driver(ahci);
    return 0;
}

int fini()
{
    log::printk(log::log_level::INFO, "Good bye from a module\n");
    return 0;
}
}

MODULE_NAME("ahci");
MODULE_AUTHOR("Jason Lu");
MODULE_DESCRIPTION("Driver for AHCI controllers");
MODULE_VERSION("0.0.1");
