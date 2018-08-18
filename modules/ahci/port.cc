#include "ahci.h"
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/virtual.h>

namespace
{
size_t max_prdt_slots = 32;              // 32 PRDT slots per command table
size_t max_prdt_size  = 4 * 1024 * 1024; // 4 MiB

void string_copy(char* target, char* source, size_t length)
{
    for (size_t i = 0; i < length; i += 2) {
        target[i]     = source[i + 1];
        target[i + 1] = source[i];
    }
    target[length] = '\0';
}
} // namespace

AHCIPort::AHCIPort(AHCIController* c, volatile struct hba_port* port)
    : controller(c)
    , identify(nullptr)
    , port(port)
{
    // We only need to allocate space for however many command slots there are
    size_t clb_size =
        sizeof(struct hba_command_header) * this->controller->get_ncs();
    if (!Memory::DMA::allocate(clb_size, this->clb)) {
        Log::printk(Log::LogLevel::ERROR,
                    "ahci: Failed to allocate CLB memory, aborting\n");
        return;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: CLB at %p, size 0x%zX\n",
                this->clb.physical_base, this->clb.size);
    String::memset(reinterpret_cast<void*>(this->clb.virtual_base), 0,
                   clb_size);
    this->port->command_list_base_low = this->clb.physical_base & 0xFFFFFFFF;
    // TODO: Not 64-bit ready
    this->port->command_list_base_high = 0;

    size_t fb_size = sizeof(struct hba_received_fis);
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
    size_t command_table_size =
        sizeof(struct hba_command_table) +
        (sizeof(struct hba_prdt_entry) * (max_prdt_slots - 1));
    Log::printk(Log::LogLevel::INFO, "ahci: Command table size: 0x%zX\n",
                command_table_size);
    for (size_t i = 0; i < this->controller->get_ncs(); i++, header++) {
        Memory::DMA::allocate(command_table_size, this->command_tables[i]);
        String::memset((void*)this->command_tables[i].virtual_base, 0,
                       command_table_size);
        header->command_table_base_low =
            this->command_tables[i].physical_base & 0xFFFFFFFF;
        header->command_table_base_high = 0;
    }
    Log::printk(Log::LogLevel::INFO, "ahci: Tables rebased\n", port);

    this->port->command |= PXCMD_FRE;
    this->port->command |= PXCMD_ST;
    this->port->interrupt_enable = PORT_INTR_ERROR | PXIE_DHRE | PXIE_PSE |
                                   PXIE_DSE | PXIE_SDBE | PXIE_DPE;
    Log::printk(Log::LogLevel::INFO, "ahci: Interrupt config set to 0x%X\n",
                this->port->interrupt_enable);

    Memory::DMA::Region ident_region;
    if (!Memory::DMA::allocate(sizeof(uint16_t) * 256, ident_region)) {
        Log::printk(Log::LogLevel::ERROR,
                    "ahci: Failed to allocate ident region buffer\n");
        return;
    }

    this->send_command(ATA_CMD_IDENTIFY, 512, 0, 0, ident_region.physical_base);

    Log::printk(Log::LogLevel::INFO, "ahci: Decoding IDENTIFY data\n");
    this->identify = reinterpret_cast<uint16_t*>(ident_region.virtual_base);
    char model[41];
    char serial[21];
    string_copy(serial,
                (char*)(&this->identify[static_cast<int>(
                    AHCIIdentify::ATA_SERIAL_NUMBER)]),
                20);
    string_copy(model,
                (char*)(&this->identify[static_cast<int>(
                    AHCIIdentify::ATA_MODEL_NUMBER)]),
                40);
    Log::printk(Log::LogLevel::INFO, "ahci: Serial: %s\n", serial);
    Log::printk(Log::LogLevel::INFO, "ahci: Model: %s\n", model);
}

AHCIPort::~AHCIPort()
{
}

ssize_t AHCIPort::read(uint8_t* buffer, size_t count, off_t offset)
{
    Memory::DMA::Region dma;
    Memory::DMA::allocate(Memory::Virtual::align_up(count), dma);

    Log::printk(Log::LogLevel::INFO, "ahci: DMA target %p, virt %p\n",
                dma.physical_base, dma.virtual_base);

    String::memcpy(buffer, reinterpret_cast<void*>(dma.virtual_base), count);
    return count;
}

ssize_t AHCIPort::write(uint8_t* buffer, size_t count, off_t offset)
{
}

void AHCIPort::handle()
{
    uint32_t is = this->port->interrupt_status;
    if (!is)
        return;

    // Clear the pending interrupts.
    this->port->interrupt_status = is;
}

int AHCIPort::get_free_slot()
{
    uint32_t slots = this->port->sata_active | this->port->command_issue;
    for (int i = 0; i < this->controller->get_ncs(); i++) {
        if (slots & (1 << i)) {
            return i;
        }
    }
    return -1;
}

bool AHCIPort::send_command(uint8_t command, size_t size, uint8_t write,
                            uint64_t lba, addr_t phys_buffer)
{
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
    command_header->write    = write ? 1 : 0;
    command_header->prdt_len = ((size - 1) / max_prdt_size) + 1;

    volatile struct hba_command_table* command_table =
        reinterpret_cast<volatile struct hba_command_table*>(
            this->command_tables[slot].virtual_base);
    size_t left = size;
    for (unsigned int index = 0; index < 65536 && left; index++) {
        command_table->prdt[index].data_base_low = phys_buffer & 0xFFFFFFFF;
        // TODO: Not 64-bit correct
        command_table->prdt[index].data_base_high = 0;
        command_table->prdt[index].byte_count =
            (left <= max_prdt_size) ? (left - 1) : (max_prdt_size - 1);
        left -= command_table->prdt[index].byte_count + 1;
        phys_buffer += command_table->prdt[index].byte_count + 1;
    }

    volatile struct fis_h2d* fis =
        (volatile struct fis_h2d*)&command_table->command_fis;
    String::memset((void*)fis, 0, sizeof(*fis));
    fis->type         = FIS_TYPE_REG_H2D;
    fis->command      = ATA_CMD_IDENTIFY;
    fis->c            = 1;
    size_t num_blocks = (size + AHCI_BLOCK_SIZE - 1) / AHCI_BLOCK_SIZE;
    fis->count_low    = 1;
    fis->count_high   = 0;
    fis->lba0         = 0;
    fis->lba1         = 0;
    fis->lba2         = 0;
    fis->lba3         = 0;
    fis->lba4         = 0;
    fis->lba5         = 0;
    fis->device       = (1 << 6);

    this->port->command_issue |= (1 << slot);

    while (1) {
        if (!(this->port->command_issue & (1 << slot))) {
            break;
        }
        if (this->port->sata_error) {
            break;
        }
    }
    return true;
}