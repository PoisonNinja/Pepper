#include "ahci.h"
#include <drivers/pci/pci.h>
#include <kernel.h>

AHCIController::AHCIController(PCI::Device* d)
{
    this->device = d;
}

void AHCIController::init()
{
    Log::printk(Log::LogLevel::INFO, "ahci: Initializing AHCI controller\n");
    // BAR5 contains ABAR
    PCI::PCIBAR raw_abar = this->device->get_pcibar(5);
    Log::printk(Log::LogLevel::INFO, "ahci: Raw ABAR at %p with size %zX\n",
                raw_abar.addr, raw_abar.size);
    auto mapped = PCI::map(raw_abar.addr, raw_abar.size);
    if (!mapped.first) {
        Log::printk(Log::LogLevel::ERROR, "ahci: Failed to map ABAR\n");
        return;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: Mapped ABAR to %p\n",
                mapped.second);
}