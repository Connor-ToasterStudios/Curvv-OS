#include <stdint.h>
#include "std.h"
#define SECTOR_SIZE 512
#define FAT12_BOOT_SECTOR 0x0
#define FAT12_FAT1_SECTOR 0x1
#define FAT12_ROOT_DIR_SECTOR 0x2

// Disk access (for demonstration, we assume BIOS/interrupts can be used to read the disk)
extern void read_disk(uint32_t sector, uint8_t* buffer);

// Structure of the Boot Sector (simplified)
typedef struct {
    uint8_t  jump[3];               // Jump instruction
    uint8_t  oem[8];                // OEM identifier
    uint16_t bytes_per_sector;      // Bytes per sector (usually 512)
    uint8_t  sectors_per_cluster;   // Sectors per cluster
    uint16_t reserved_sectors;      // Reserved sectors (usually 32)
    uint8_t  num_fats;              // Number of FATs (usually 2)
    uint16_t max_root_dir_entries;  // Max root directory entries
    uint16_t total_sectors;         // Total sectors (if FAT16, else use 32-bit field)
    uint8_t  media_descriptor;      // Media descriptor
    uint16_t sectors_per_fat;      // Sectors per FAT table
} __attribute__((packed)) FAT12_BootSector;

// Read the boot sector to understand the disk layout
void read_boot_sector(uint8_t* buffer) {
    read_disk(FAT12_BOOT_SECTOR, buffer);
}

void print_boot_sector_info(uint8_t* buffer) {
    FAT12_BootSector* boot_sector = (FAT12_BootSector*)buffer;
    printf("Bytes per sector: %u\n", boot_sector->bytes_per_sector);
    printf("Sectors per cluster: %u\n", boot_sector->sectors_per_cluster);
    printf("Reserved sectors: %u\n", boot_sector->reserved_sectors);
    printf("Number of FATs: %u\n", boot_sector->num_fats);
}

// Read the FAT Table from the disk
void read_fat_table(uint8_t* buffer) {
    read_disk(FAT12_FAT1_SECTOR, buffer);
}

// Load a file starting from a given cluster
void load_file(uint16_t start_cluster, uint8_t* buffer, uint32_t max_size) {
    uint32_t current_cluster = start_cluster;
    uint32_t offset = 0;

    while (current_cluster < 0xFFF) {
        // Read the data sector for this cluster
        uint32_t data_sector = current_cluster * SECTOR_SIZE;
        read_disk(data_sector, buffer + offset);
        offset += SECTOR_SIZE;
        
        // Get the next cluster in the FAT chain
        // Here, we just simulate reading the next cluster
        // (In actual FAT12, we'd need to read the FAT to find the next cluster)
        current_cluster++; // For simplicity, incrementing the cluster here
    }
}

// Basic driver to display the first few bytes of the file
void load_and_print_file(uint16_t start_cluster) {
    uint8_t buffer[SECTOR_SIZE * 2]; // Buffer to hold a couple of sectors
    load_file(start_cluster, buffer, sizeof(buffer));

    // Print the first 16 bytes of the loaded file
    for (int i = 0; i < 16; i++) {
        printf("%c", buffer[i]);
    }
}

// Example of usage
void kernel_main() {
    uint8_t boot_sector_buffer[SECTOR_SIZE];
    read_boot_sector(boot_sector_buffer);
    print_boot_sector_info(boot_sector_buffer);

    // Let's assume the file starts at cluster 2
    load_and_print_file(2);  // Load and print file starting from cluster 2
}
