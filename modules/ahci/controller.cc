#include "ahci.h"
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <lib/functional.h>

using namespace libcxx::placeholders;

ahci_controller::ahci_controller(pci::device* d, dev_t major)
    : major{major}
    , ports{}
    , hba{nullptr}
    , device{d}
    , handler_data{libcxx::bind(&ahci_controller::handler, this, _1, _2, _3),
                   "ahci", this}
{
}

void ahci_controller::init()
{
    log::printk(log::log_level::INFO, "ahci: Initializing AHCI controller\n");

    // BAR5 contains ABAR
    pci::pcibar raw_abar = this->device->get_pcibar(5);
    log::printk(log::log_level::INFO, "ahci: Raw ABAR at %p with size 0x%zX\n",
                raw_abar.addr, raw_abar.size);
    auto mapped = pci::map(raw_abar.addr, raw_abar.size);
    if (!mapped) {
        log::printk(log::log_level::ERROR, "ahci: Failed to map ABAR\n");
        return;
    }
    log::printk(log::log_level::INFO, "ahci: Mapped ABAR to %p\n", *mapped);
    this->hba = reinterpret_cast<struct hba_memory*>(*mapped);

    // TODO: Support MSI
    uint8_t irq = this->device->read_config_8(pci::pci_interrupt_line);
    log::printk(log::log_level::INFO, "ahci: IRQ #%d\n", irq);

    interrupt::register_handler(interrupt::irq_to_interrupt(irq),
                                this->handler_data);

    // We are AHCI aware
    this->hba->global_host_control |= GHC_AE;

    log::printk(log::log_level::INFO, "ahci: Ports implemented: %X\n",
                this->hba->port_implemented);
    for (unsigned int i = 0; i < 32; i++) {
        if (this->hba->port_implemented & (1 << i)) {
            if (this->hba->ports[i].signature != AHCI_TYPE_SATA) {
                continue;
            }
            if (this->hba->ports[i].command &
                (PXCMD_ST | PXCMD_CR | PXCMD_FRE | PXCMD_FR)) {
                log::printk(log::log_level::INFO,
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
                log::printk(log::log_level::INFO,
                            "ahci: Port %d already idle\n", i);
            }
            ports[i] = new ahci_port(this, &this->hba->ports[i]);
            filesystem::register_blockdev(this->major, ports[i]);
        }
    }

    /*
     * Ports have set their interrupts that they can handle, let's enable them
     * globally so we can start getting them
     */
    this->hba->global_host_control |= GHC_IE;
}

size_t ahci_controller::get_ncs()
{
    return capability_ncs(this->hba->capability);
}

bool ahci_controller::is_64bit()
{
    return this->hba->capability & CAP_S64A;
}

void ahci_controller::handler(int, void*, struct interrupt_context* /* ctx */)
{
    uint32_t is = this->hba->interrupt_status;
    for (int i = 0; i < 32; i++) {
        if ((is & (1 << i)) && this->ports[i]) {
            this->ports[i]->handle();
        }
    }
    this->hba->interrupt_status = is;
}
