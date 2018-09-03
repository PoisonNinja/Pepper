#include "ahci.h"
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/module.h>
#include <lib/printf.h>
#include <lib/string.h>

namespace
{
// TODO: Support generic AHCI devices
PCI::Filter ahci_filter[] = {
    // Intel
    {PCI_VDEV(0x8086, 0x2829)},    // ICH8M (VirtualBox)
    {PCI_VDEV(0x8086, 0x2922)},    // ICH9 (QEMU),
    {PCI_CLASS_IF(0x1, 0x6, 0x1)}, // Generic AHCI device
    {},                            // Null terminator
};
} // namespace

class AHCIDriver : public PCI::Driver
{
public:
    AHCIDriver();
    ~AHCIDriver() override;
    bool probe(PCI::Device* dev) override;
    const char* name() override;
    const PCI::Filter* filter() override;

private:
    dev_t major;
};

AHCIDriver::AHCIDriver()
{
    this->major = Filesystem::locate_class(Filesystem::BLK);
    Filesystem::register_class(Filesystem::BLK, this->major);
}

AHCIDriver::~AHCIDriver()
{
}

bool AHCIDriver::probe(PCI::Device* dev)
{
    dev->claim();
    AHCIController* ahci = new AHCIController(dev, this->major);
    ahci->init();
    return true;
}

const char* AHCIDriver::name()
{
    return "ahci";
}

const PCI::Filter* AHCIDriver::filter()
{
    return ahci_filter;
}

namespace
{
AHCIDriver ahci;
}

extern "C" {
int init()
{
    Log::printk(Log::LogLevel::INFO, "ahci: Registering driver...\n");
    PCI::register_driver(ahci);
    return 0;
}

int fini()
{
    Log::printk(Log::LogLevel::INFO, "Good bye from a module\n");
    return 0;
}
}

MODULE_NAME("ahci");
MODULE_AUTHOR("Jason Lu");
MODULE_DESCRIPTION("Driver for AHCI controllers");
MODULE_VERSION("0.0.1");
