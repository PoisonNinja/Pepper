#include "ahci.h"
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/virtual.h>

AHCIPort::AHCIPort(AHCIController* c, volatile struct hba_port* port)
    : controller(c)
    , port(port)
{
    size_t clb_size = 0x1000; /*sizeof(struct hba_command_header) *
                                 this->controller->get_ncs(); */
    if (!Memory::DMA::allocate(clb_size, this->clb)) {
        Log::printk(Log::LogLevel::ERROR,
                    "ahci: Failed to allocate CLB memory, aborting\n");
        return;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: CLB at %p, size 0x%zX\n",
                this->clb.physical_base, this->clb.size);
    String::memset(reinterpret_cast<void*>(this->clb.virtual_base), 0,
                   clb_size);
    this->port->command_list_base_low = this->clb.physical_base;
    // TODO: Not 64-bit ready
    this->port->command_list_base_high = 0;

    size_t fb_size = 0x1000;
    if (!Memory::DMA::allocate(fb_size, this->fb)) {
        Log::printk(Log::LogLevel::ERROR,
                    "ahci: Failed to allocate FIS memory, aborting\n");
        return;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: FIS at %p, size 0x%zX\n",
                this->fb.physical_base, this->fb.size);
    String::memset(reinterpret_cast<void*>(this->fb.virtual_base), 0, fb_size);
    this->port->fis_base_low = this->fb.physical_base;
    // TODO: Not 64-bit ready
    this->port->fis_base_high = 0;

    volatile struct hba_command_header* header =
        reinterpret_cast<volatile struct hba_command_header*>(clb.virtual_base);
    for (size_t i = 0; i < this->controller->get_ncs(); i++, header++) {
        Memory::DMA::allocate(0x1000, this->command_tables[i]);
        String::memset((void*)this->command_tables[i].virtual_base, 0, 0x1000);
        header->command_table_base_low =
            this->command_tables[i].physical_base & 0xFFFFFFFF;
        header->command_table_base_high = 0;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: Tables rebased\n", port);

    this->port->command |= PXCMD_FRE;
    this->port->command |= PXCMD_ST;
    this->port->interrupt_enable =
        PXIE_DHRE | PXIE_PSE | PXIE_DSE | PXIE_SDBE | PXIE_DPE;
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
    if (slot == -1) {
        Log::printk(Log::LogLevel::ERROR,
                    "ahci: Failed to get free command slot\n");
        return false;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: Got free slot %d\n", slot);

    volatile struct hba_command_header* command_header =
        reinterpret_cast<volatile struct hba_command_header*>(
            this->clb.virtual_base);
    command_header += slot;
    command_header->fis_length =
        sizeof(struct fis_h2d) / sizeof(uint32_t); // dwords
    command_header->write    = 0;
    command_header->prdt_len = 1;

    volatile struct hba_command_table* command_table =
        reinterpret_cast<volatile struct hba_command_table*>(
            this->command_tables[slot].virtual_base);

    command_table->prdt[0].data_base_low = dma.physical_base & 0xFFFFFFFF;
    // TODO: This is not correct
    command_table->prdt[0].data_base_high = 0;
    command_table->prdt[0].byte_count     = 511;

    volatile struct fis_h2d* fis =
        (volatile struct fis_h2d*)&command_table->command_fis;
    String::memset((void*)fis, 0, sizeof(*fis));
    fis->type    = FIS_TYPE_REG_H2D;
    fis->command = ATA_CMD_IDENTIFY;
    fis->c       = 1;
    // size_t num_blocks = (size + AHCI_BLOCK_SIZE - 1) / AHCI_BLOCK_SIZE;
    fis->count_low  = 1;
    fis->count_high = 0;
    fis->lba0       = 0;
    fis->lba1       = 0;
    fis->lba2       = 0;
    fis->lba3       = 0;
    fis->lba4       = 0;
    fis->lba5       = 0;
    fis->device     = (1 << 6);

    Log::printk(Log::LogLevel::INFO, "ahci: DMA target %p, virt %p\n",
                dma.physical_base, dma.virtual_base);

    this->port->command_issue |= (1 << slot);

    while (1) {
        if (!(this->port->command_issue & (1 << slot))) {
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