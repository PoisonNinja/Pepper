#include "ahci.h"
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <lib/functional.h>

using namespace libcxx::placeholders;

AHCIController::AHCIController(PCI::Device* d, dev_t major)
    : major{major}
    , ports{}
    , hba{nullptr}
    , device{d}
    , handler_data{libcxx::bind(&AHCIController::handler, this, _1, _2, _3),
                   "ahci", this}
{
}

void AHCIController::init()
{
    Log::printk(Log::LogLevel::INFO, "ahci: Initializing AHCI controller\n");

    // BAR5 contains ABAR
    PCI::PCIBAR raw_abar = this->device->get_pcibar(5);
    Log::printk(Log::LogLevel::INFO, "ahci: Raw ABAR at %p with size 0x%zX\n",
                raw_abar.addr, raw_abar.size);
    auto mapped = PCI::map(raw_abar.addr, raw_abar.size);
    if (!mapped.first) {
        Log::printk(Log::LogLevel::ERROR, "ahci: Failed to map ABAR\n");
        return;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: Mapped ABAR to %p\n",
                mapped.second);
    this->hba = reinterpret_cast<struct hba_memory*>(mapped.second);

    // TODO: Support MSI
    uint8_t irq = this->device->read_config_8(PCI::pci_interrupt_line);
    Log::printk(Log::LogLevel::INFO, "ahci: IRQ #%d\n", irq);

    Interrupt::register_handler(Interrupt::irq_to_interrupt(irq),
                                this->handler_data);

    // We are AHCI aware
    this->hba->global_host_control |= GHC_AE;

    Log::printk(Log::LogLevel::INFO, "ahci: Ports implemented: %X\n",
                this->hba->port_implemented);
    for (unsigned int i = 0; i < 32; i++) {
        if (this->hba->port_implemented & (1 << i)) {
            if (this->hba->ports[i].signature != AHCI_TYPE_SATA) {
                continue;
            }
            if (this->hba->ports[i].command &
                (PXCMD_ST | PXCMD_CR | PXCMD_FRE | PXCMD_FR)) {
                Log::printk(Log::LogLevel::INFO,
                            "ahci: Port %d not idle, setting to idle\n", i);
                this->hba->ports[i].command &= ~PXCMD_ST;
                while (this->hba->ports[i].command & PXCMD_CR)
                    ;
                if (this->hba->ports[i].command & PXCMD_FRE) {
                    this->hba->ports[i].command &= ~PXCMD_FRE;
                    while (this->hba->ports[i].command & PXCMD_FR)
                        ;
                }
            } else {
                Log::printk(Log::LogLevel::INFO, "ahci: Port %d already idle\n",
                            i);
            }
            ports[i] = new AHCIPort(this, &this->hba->ports[i]);
            Filesystem::register_blockdev(this->major, ports[i]);
        }
    }

    /*
     * Ports have set their interrupts that they can handle, let's enable them
     * globally so we can start getting them
     */
    this->hba->global_host_control |= GHC_IE;
}

size_t AHCIController::get_ncs()
{
    return capability_ncs(this->hba->capability);
}

bool AHCIController::is_64bit()
{
    return this->hba->capability & CAP_S64A;
}

void AHCIController::handler(int, void* data,
                             struct InterruptContext* /* ctx */)
{
    uint32_t is = this->hba->interrupt_status;
    for (int i = 0; i < 32; i++) {
        if ((is & (1 << i)) && this->ports[i]) {
            this->ports[i]->handle();
        }
    }
    this->hba->interrupt_status = is;
}
