#include "ahci.h"
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/virtual.h>

AHCIPort::AHCIPort(AHCIController* c, volatile struct hba_port* port)
    : controller(c)
    , port(port)
{
    size_t clb_size = 0x1000; // sizeof(struct hba_command_header) *
                              // this->controller->get_ncs();
    if (!Memory::DMA::allocate(clb_size, this->clb)) {
        Log::printk(Log::LogLevel::ERROR,
                    "ahci: Failed to allocate CLB memory, aborting\n");
        return;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: CLB at %p\n",
                this->clb.physical_base);
    String::memset(reinterpret_cast<void*>(this->clb.virtual_base), 0,
                   clb_size);
    this->port->command_list_base_low = this->clb.physical_base;
    // TODO: Not 64-bit ready
    this->port->command_list_base_high = 0;

    size_t fb_size = sizeof(struct hba_received_fis);
    if (!Memory::DMA::allocate(fb_size, this->fb)) {
        Log::printk(Log::LogLevel::ERROR,
                    "ahci: Failed to allocate FIS memory, aborting\n");
        return;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: FIS at %p\n",
                this->fb.physical_base);
    String::memset(reinterpret_cast<void*>(this->fb.virtual_base), 0, fb_size);
    this->port->fis_base_low = this->fb.physical_base;
    // TODO: Not 64-bit ready
    this->port->fis_base_high = 0;

    Log::printk(Log::LogLevel::INFO, "ahci: Tables rebased\n", port);

    this->port->command |= PXCMD_FRE;
    this->port->command |= PXCMD_ST;
    this->port->interrupt_enable |= (PORT_INTR_ERROR | PXIE_DHRE | PXIE_PSE |
                                     PXIE_DSE | PXIE_SDBE | PXIE_DPE);
    Log::printk(Log::LogLevel::INFO, "ahci: Interrupt config set to 0x%X\n",
                this->port->interrupt_enable);
}

AHCIPort::~AHCIPort()
{
}

ssize_t AHCIPort::read(uint8_t* buffer, size_t count, off_t offset)
{
    Memory::DMA::Region dma;
    Memory::DMA::allocate(Memory::Virtual::align_up(count), dma);
    int slot = this->get_free_slot();
    Log::printk(Log::LogLevel::INFO, "ahci: Got free slot %d\n", slot);

    volatile struct hba_command_header* command_header =
        reinterpret_cast<volatile struct hba_command_header*>(
            this->clb.virtual_base) +
        slot;
    Memory::DMA::Region r;
    // TODO: Evaluate if 32 is the correct #
    size_t command_table_size =
        Memory::Virtual::align_up(sizeof(struct hba_command_table) +
                                  (sizeof(struct hba_prdt_entry) * 32));
    Memory::DMA::allocate(command_table_size, r);
    Log::printk(Log::LogLevel::INFO, "ahci: CMDTBL at %p\n", r.physical_base);
    command_header->command_table_base_low = r.physical_base & 0xFFFFFFFF;
    // TODO: Not 64-bit correct!
    command_header->command_table_base_high = 0;

    command_header->fis_length =
        sizeof(struct fis_h2d) / sizeof(uint32_t); // dwords
    command_header->write      = 0;
    command_header->prdt_len   = 1;
    command_header->prdb_count = 0;

    String::memset((void*)r.virtual_base, 0,
                   sizeof(struct hba_command_table) +
                       (sizeof(struct hba_prdt_entry) * 32));
    volatile struct hba_command_table* command_table =
        reinterpret_cast<volatile struct hba_command_table*>(r.virtual_base);

    command_table->command_fis.type    = FIS_TYPE_REG_H2D;
    command_table->command_fis.pmport  = 0;
    command_table->command_fis.c       = 1;
    command_table->command_fis.command = ATA_CMD_IDENTIFY;
    command_table->command_fis.lba0    = 0;
    command_table->command_fis.lba1    = 0;
    command_table->command_fis.lba2    = 0;
    command_table->command_fis.lba3    = 0;
    command_table->command_fis.lba4    = 0;
    command_table->command_fis.lba5    = 0;
    command_table->command_fis.device  = 1 << 6;

    command_table->command_fis.count_low  = 1;
    command_table->command_fis.count_high = 0;

    Log::printk(Log::LogLevel::INFO, "ahci: DMA target %p, virt %p\n",
                dma.physical_base);
    command_table->prdt[0].data_base_low = dma.physical_base & 0xFFFFFFFF;
    // TODO: This is not correct
    command_table->prdt[0].data_base_high = 0;
    command_table->prdt[0].byte_count     = 511; // Arbitrarily chosen

    this->port->command_issue = (1 << slot);

    while (1) {
        if (this->port->command_issue & (1 << slot) == 0) {
            break;
        }
        if (this->port->sata_error) {
            break;
        }
    }
    String::memcpy(buffer, reinterpret_cast<void*>(dma.virtual_base), count);
    return count;
}

ssize_t AHCIPort::write(uint8_t* buffer, size_t count, off_t offset)
{
}

int AHCIPort::get_free_slot()
{
    uint32_t slots = this->port->sata_active | this->port->command_issue;
    volatile struct hba_command_header* base =
        reinterpret_cast<struct hba_command_header*>(this->clb.virtual_base);
    for (int i = 0; i < this->controller->get_ncs(); i++) {
        if (slots & (1 << i)) {
            return i;
        }
    }
    return -1;
}