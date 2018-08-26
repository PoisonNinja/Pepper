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

uint32_t get_lba28_capacity(uint16_t* identify)
{
    uint32_t lba_cap =
        identify[static_cast<int>(AHCIIdentify::ATA_LBA28_CAPACITY) + 1];
    return lba_cap << 16 |
           identify[static_cast<int>(AHCIIdentify::ATA_LBA28_CAPACITY)];
}

uint64_t get_lba48_capacity(uint16_t* identify)
{
    uint64_t lba48_cap =
        identify[static_cast<int>(AHCIIdentify::ATA_LBA48_CAPACITY) + 3];
    return ((lba48_cap << 16 |
             identify[static_cast<int>(AHCIIdentify::ATA_LBA48_CAPACITY) + 2])
                << 16 |
            identify[static_cast<int>(AHCIIdentify::ATA_LBA48_CAPACITY) + 1])
               << 16 |
           identify[static_cast<int>(AHCIIdentify::ATA_LBA48_CAPACITY)];
}

bool lba48_supported(uint16_t* identify)
{
    return identify[static_cast<int>(AHCIIdentify::ATA_COMMANDSET_2)] &
           (1 << 10);
}
} // namespace

AHCIPort::AHCIPort(AHCIController* c, volatile struct hba_port* port)
    : controller(c)
    , identify(nullptr)
    , port(port)
    , is_lba48(false)
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

    Memory::DMA::SGList* ident_region =
        Memory::DMA::build_sglist(max_prdt_slots, max_prdt_size, 512);

    this->send_command(ATA_CMD_IDENTIFY, 512, 0, 0, ident_region);

    Log::printk(Log::LogLevel::INFO, "ahci: Decoding IDENTIFY data\n");
    this->identify =
        reinterpret_cast<uint16_t*>(ident_region->list.front().virtual_base);
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
    this->is_lba48 = lba48_supported(this->identify);
    if (this->is_lba48) {
        Log::printk(Log::LogLevel::INFO, "ahci: LBA48 count: 0x%zX\n",
                    get_lba48_capacity(this->identify));
    } else {
        Log::printk(Log::LogLevel::INFO, "ahci: LBA28 count: 0x%zX\n",
                    get_lba28_capacity(this->identify));
    }
}

AHCIPort::~AHCIPort()
{
}

bool AHCIPort::request(Filesystem::BlockRequest* request)
{
    if (request->command == Filesystem::BlockRequestType::READ) {
        uint8_t command =
            (this->is_lba48) ? ATA_CMD_READ_DMA : ATA_CMD_READ_DMA_EXT;
        return this->send_command(command, request->num_sectors, 0,
                                  request->start, request->sglist);
    } else {
        return false;
    }
}

Filesystem::sector_t AHCIPort::sector_size()
{
    // TODO: Actually calculate this
    return 512;
}

size_t AHCIPort::sg_max_size()
{
    return max_prdt_size;
}

size_t AHCIPort::sg_max_count()
{
    return max_prdt_slots;
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
    for (size_t i = 0; i < this->controller->get_ncs(); i++) {
        if (slots & (1 << i)) {
            return i;
        }
    }
    return -1;
}

bool AHCIPort::send_command(uint8_t command, size_t num_blocks, uint8_t write,
                            uint64_t lba, Memory::DMA::SGList* sglist)
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
    command_header->write      = write ? 1 : 0;
    command_header->prdt_len   = sglist->num_regions;
    command_header->prdb_count = 0;

    volatile struct hba_command_table* command_table =
        reinterpret_cast<volatile struct hba_command_table*>(
            this->command_tables[slot].virtual_base);
    auto sg = sglist->list.begin();
    for (unsigned int index = 0;
         index < max_prdt_slots && sg != sglist->list.end(); index++, sg++) {
        String::memset((void*)((*sg).virtual_base), 0xFE, (*sg).size);
        Log::printk(Log::LogLevel::INFO, "ahci: %p %p 0x%zX 0x%zX\n",
                    (*sg).physical_base, (*sg).virtual_base, (*sg).size,
                    (*sg).real_size);
        command_table->prdt[index].data_base_low =
            (*sg).physical_base & 0xFFFFFFFF;
        // TODO: Not 64-bit correct
        command_table->prdt[index].data_base_high = 0;
        command_table->prdt[index].byte_count     = (*sg).size - 1;
    }

    volatile struct fis_h2d* fis =
        (volatile struct fis_h2d*)&command_table->command_fis;
    String::memset((void*)fis, 0, sizeof(*fis));
    fis->type    = FIS_TYPE_REG_H2D;
    fis->command = command;
    fis->c       = 1;
    if (this->is_lba48) {
        fis->count_low  = num_blocks & 0xFF;
        fis->count_high = (num_blocks >> 8) & 0xFF;
        fis->lba0       = lba & 0xFF;
        fis->lba1       = (lba >> 8) & 0xFF;
        fis->lba2       = (lba >> 16) & 0xFF;
        fis->lba3       = (lba >> 24) & 0xFF;
        fis->lba4       = (lba >> 32) & 0xFF;
        fis->lba5       = (lba >> 40) & 0xFF;
        fis->device     = (1 << 6);
    } else {
        fis->count_low = num_blocks & 0xFF;
        fis->lba0      = lba & 0xFF;
        fis->lba1      = (lba >> 8) & 0xFF;
        fis->lba2      = (lba >> 16) & 0xFF;
        fis->device    = (1 << 6) | ((lba >> 24) & 0xF);
    }

    while (this->port->task_file_data & (ATA_STATUS_BSY | ATA_STATUS_DRQ))
        ;

    this->port->command_issue |= (1 << slot);

    while (this->port->command_issue & (1 << slot))
        ;

    return true;
}