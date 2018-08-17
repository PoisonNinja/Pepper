#pragma once

#include <fs/block.h>
#include <mm/dma.h>
#include <types.h>

typedef enum {
    FIS_TYPE_REG_H2D   = 0x27, // Register FIS - host to device
    FIS_TYPE_REG_D2H   = 0x34, // Register FIS - device to host
    FIS_TYPE_DMA_ACT   = 0x39, // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
    FIS_TYPE_DATA      = 0x46, // Data FIS - bidirectional
    FIS_TYPE_BIST      = 0x58, // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS  = 0xA1, // Set device bits FIS - device to host
} FIS_TYPE;

#define ATA_STATUS_ERR (1 << 0)  /* Error. */
#define ATA_STATUS_DRQ (1 << 3)  /* Data Request. */
#define ATA_STATUS_DF (1 << 5)   /* Device Fault. */
#define ATA_STATUS_DRDY (1 << 6) /* Device Ready. */
#define ATA_STATUS_BSY (1 << 7)  /* Busy. */

/** ATA Commands. */
#define ATA_CMD_READ_DMA 0xC8          /**< READ DMA. */
#define ATA_CMD_READ_DMA_EXT 0x25      /**< READ DMA EXT. */
#define ATA_CMD_READ_SECTORS 0x20      /**< READ SECTORS. */
#define ATA_CMD_READ_SECTORS_EXT 0x24  /**< READ SECTORS EXT. */
#define ATA_CMD_WRITE_DMA 0xCA         /**< WRITE DMA. */
#define ATA_CMD_WRITE_DMA_EXT 0x35     /**< WRITE DMA EXT. */
#define ATA_CMD_WRITE_SECTORS 0x30     /**< WRITE SECTORS. */
#define ATA_CMD_WRITE_SECTORS_EXT 0x34 /**< WRITE SECTORS EXT. */
#define ATA_CMD_PACKET 0xA0            /**< PACKET. */
#define ATA_CMD_IDENTIFY_PACKET 0xA1   /**< IDENTIFY PACKET DEVICE. */
#define ATA_CMD_FLUSH_CACHE 0xE7       /**< FLUSH CACHE. */
#define ATA_CMD_FLUSH_CACHE_EXT 0xEA   /**< FLUSH CACHE EXT. */
#define ATA_CMD_IDENTIFY 0xEC          /**< IDENTIFY DEVICE. */

/** Bits in the Port x Interrupt Enable register. */
#define PXIE_DHRE (1 << 0)  /**< Device to Host Register Enable. */
#define PXIE_PSE (1 << 1)   /**< PIO Setup FIS Enable. */
#define PXIE_DSE (1 << 2)   /**< DMA Setup FIS Enable. */
#define PXIE_SDBE (1 << 3)  /**< Set Device Bits Enable. */
#define PXIE_UFE (1 << 4)   /**< Unknown FIS Enable. */
#define PXIE_DPE (1 << 5)   /**< Descriptor Processed Enable. */
#define PXIE_PCE (1 << 6)   /**< Port Connect Change Enable. */
#define PXIE_DMPE (1 << 7)  /**< Device Mechanical Presence Enable. */
#define PXIE_PRCE (1 << 22) /**< PhyRdy Change Enable. */
#define PXIE_IPME (1 << 23) /**< Incorrect Port Multiplier Enable. */
#define PXIE_OFE (1 << 24)  /**< Overflow Enable. */
#define PXIE_INFE (1 << 26) /**< Interface Non-Fatal Error Enable. */
#define PXIE_IFE (1 << 27)  /**< Interface Fatal Error Enable. */
#define PXIE_HBDE (1 << 28) /**< Host Bus Data Error Enable. */
#define PXIE_HBFE (1 << 29) /**< Host Bus Fatal Error Enable. */
#define PXIE_TFEE (1 << 30) /**< Task File Error Enable. */
#define PXIE_CPDE (1 << 31) /**< Cold Port Detect Enable. */

#define PORT_INTR_ERROR                                                        \
    (PXIE_UFE | PXIE_PCE | PXIE_PRCE | PXIE_IPME | PXIE_OFE | PXIE_INFE |      \
     PXIE_IFE | PXIE_HBDE | PXIE_HBFE | PXIE_TFEE)

constexpr uint32_t PXCMD_ST  = 1 << 0; /* 0x00000001 */
constexpr uint32_t PXCMD_SUD = 1 << 1; /* 0x00000002 */
constexpr uint32_t PXCMD_POD = 1 << 2; /* 0x00000004 */
constexpr uint32_t PXCMD_CLO = 1 << 3; /* 0x00000008 */
constexpr uint32_t PXCMD_FRE = 1 << 4; /* 0x00000010 */
constexpr uint32_t PXCMD_CSS(uint32_t val)
{
    return (val >> 8) % 32;
}
constexpr uint32_t PXCMD_MPSS  = 1 << 13; /* 0x00002000 */
constexpr uint32_t PXCMD_FR    = 1 << 14; /* 0x00004000 */
constexpr uint32_t PXCMD_CR    = 1 << 15; /* 0x00008000 */
constexpr uint32_t PXCMD_CPS   = 1 << 16; /* 0x00010000 */
constexpr uint32_t PXCMD_PMA   = 1 << 17; /* 0x00020000 */
constexpr uint32_t PXCMD_HPCP  = 1 << 18; /* 0x00040000 */
constexpr uint32_t PXCMD_MPSP  = 1 << 19; /* 0x00080000 */
constexpr uint32_t PXCMD_CPD   = 1 << 20; /* 0x00100000 */
constexpr uint32_t PXCMD_ESP   = 1 << 21; /* 0x00200000 */
constexpr uint32_t PXCMD_FBSCP = 1 << 22; /* 0x00400000 */
constexpr uint32_t PXCMD_APSTE = 1 << 23; /* 0x00800000 */
constexpr uint32_t PXCMD_ATAPI = 1 << 24; /* 0x01000000 */
constexpr uint32_t PXCMD_DLAE  = 1 << 25; /* 0x02000000 */
constexpr uint32_t PXCMD_ALPE  = 1 << 26; /* 0x04000000 */
constexpr uint32_t PXCMD_ASP   = 1 << 27; /* 0x08000000 */
constexpr uint32_t PXCMD_ICC(uint32_t val)
{
    return (val >> 28) % 16;
}

#define AHCI_TYPE_NULL 0x0
#define AHCI_TYPE_SATA 0x00000101
#define AHCI_TYPE_ATAPI 0xEB140101
#define AHCI_TYPE_SEMB 0xC33C0101
#define AHCI_TYPE_PM 0x96690101

#define AHCI_PRDT_MAX_MEMORY 0x1000
#define AHCI_BLOCK_SIZE 512

struct hba_port {
    uint32_t command_list_base_low;
    uint32_t command_list_base_high;
    uint32_t fis_base_low;
    uint32_t fis_base_high;
    uint32_t interrupt_status;
    uint32_t interrupt_enable;
    uint32_t command;
    uint32_t reserved_0;
    uint32_t task_file_data;
    uint32_t signature;
    uint32_t sata_status;
    uint32_t sata_control;
    uint32_t sata_error;
    uint32_t sata_active;
    uint32_t command_issue;
    uint32_t sata_notification;
    uint32_t fis_based_switch_control;
    uint32_t reserved_1[11];
    uint32_t vendor[4];
} __attribute__((packed));

constexpr uint32_t GHC_AE   = 1U << 31;
constexpr uint32_t GHC_MRSM = 1U << 2;
constexpr uint32_t GHC_IE   = 1U << 1;
constexpr uint32_t GHC_HR   = 1U << 0;

constexpr uint32_t CAP_S64A = 1U << 31;
constexpr uint32_t capability_ncs(uint32_t capability)
{
    return (capability & 0x1F00) >> 8;
}

struct hba_memory {
    uint32_t capability;
    uint32_t global_host_control;
    uint32_t interrupt_status;
    uint32_t port_implemented;
    uint32_t version;
    uint32_t ccc_control;
    uint32_t ccc_ports;
    uint32_t em_location;
    uint32_t em_control;
    uint32_t ext_capabilities;
    uint32_t bohc;
    uint8_t reserved[0xA0 - 0x2C];
    uint8_t vendor[0x100 - 0xA0];
    struct hba_port ports[32];
} __attribute__((packed));

struct hba_received_fis {
    uint8_t fis_ds[0x1C];
    uint8_t pad_0[0x4];
    uint8_t fis_ps[0x14];
    uint8_t pad_1[0xC];
    uint8_t fis_r[0x14];
    uint8_t pad_2[0x4];
    uint8_t fis_sdb[0x8];
    uint8_t ufis[0x40];
    uint8_t reserved[0x60];
} __attribute__((packed));

struct hba_command_header {
    uint8_t fis_length : 5;
    uint8_t atapi : 1;
    uint8_t write : 1;
    uint8_t prefetchable : 1;
    uint8_t reset : 1;
    uint8_t bist : 1;
    uint8_t clear_busy_upon_r_ok : 1;
    uint8_t reserved_0 : 1;
    uint8_t pmport : 4;
    uint16_t prdt_len;
    uint32_t prdb_count;
    uint32_t command_table_base_low;
    uint32_t command_table_base_high;
    uint32_t reserved_1[4];
} __attribute__((packed));

struct hba_prdt_entry {
    uint32_t data_base_low;
    uint32_t data_base_high;
    uint32_t reserved_0;
    uint32_t byte_count : 22;
    uint32_t reserved_1 : 9;
    uint32_t interrupt_on_complete : 1;
} __attribute__((packed));

struct fis_h2d {
    uint8_t type;
    uint8_t pmport : 4;
    uint8_t reserved_0 : 3;
    uint8_t c : 1;
    uint8_t command;
    uint8_t feature_low;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t feature_high;

    uint8_t count_low;
    uint8_t count_high;
    uint8_t icc;
    uint8_t control;
    uint32_t reserved_1;
    uint8_t padding[0x2C];
} __attribute__((packed));

struct hba_command_table {
    struct fis_h2d command_fis;
    uint8_t acmd[0x10];
    uint8_t reserved[0x30];
    struct hba_prdt_entry prdt[1];
} __attribute__((packed));

/*
 * ATA identify response data, per the ATA spec at
 * http://www.t13.org/Documents/UploadedDocuments/docs2009/d2015r1a-ATAATAPI_Command_Set_-_2_ACS-2.pdf
 *
 * TODO: Move this to generic ATA header
 */
enum class AHCI_IDENTIFY_COMMANDS {
    ATA_GENERAL_CONFIGURATION  = 0,
    ATA_SPECIFIC_CONFIGURATION = 2,
    ATA_SERIAL_NUMBER          = 10,
    ATA_FIRMWARE_REVISION      = 23,
    ATA_MODEL_NUMBER           = 27,
    ATA_TRUSTED_COMPUTING      = 48,
    ATA_CAPABILITY             = 49,
    ATA_FIELD_VALID            = 53,
    ATA_MULTIPLE_SECTOR        = 59,
    ATA_LBA28_CAPACITY         = 60,
    ATA_MULTIWORD_MODES        = 63,
    ATA_PIO_MODES              = 64,
    ATA_MAJOR_VERSION          = 80,
    ATA_MINOR_VERSION          = 81,
    ATA_COMMANDSET_1           = 82,
    ATA_COMMANDSET_2           = 83,
    ATA_COMMANDSET_EXTENDED    = 84,
    ATA_CFS_ENABLE_1           = 85,
    ATA_CFS_ENABLE_2           = 86,
    ATA_CFS_DEFAULT            = 87,
    ATA_UDMA_MODES             = 88,
    ATA_HW_RESET               = 93,
    ATA_ACOUSTIC               = 94,
    ATA_LBA48_CAPACITY         = 100,
    ATA_REMOVABLE              = 127,
    ATA_SECURITY_STATUS        = 128,
    ATA_CFA_POWER_MODE         = 160,
    ATA_MEDIA_SERIAL_NUMBER    = 176,
    ATA_INTEGRITY              = 255,
};

namespace PCI
{
class Device;
}

class AHCIController;

class AHCIPort : public Filesystem::BlockDevice
{
public:
    AHCIPort(AHCIController* c, volatile struct hba_port* port);
    virtual ~AHCIPort() override;
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset) override;

private:
    int get_free_slot();

    AHCIController* controller;
    volatile struct hba_port* port;
    Memory::DMA::Region fb;
    Memory::DMA::Region clb;
};

class AHCIController
{
public:
    AHCIController(PCI::Device* d, dev_t major);
    void init();

    size_t get_ncs();

private:
    dev_t major;
    AHCIPort* ports[32];
    volatile struct hba_memory* hba;
    PCI::Device* device;
};