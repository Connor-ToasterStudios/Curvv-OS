#include <stdint.h>
#include <stdio.h>

#define NE2000_IO_BASE 0x280 // Base I/O address for NE2000 in QEMU
#define NE2000_INTERRUPT 9   // IRQ for NE2000 in QEMU

// NE2000 Registers
#define NE2000_REG_RESET       0x00
#define NE2000_REG_COMMAND     0x01
#define NE2000_REG_TX_START    0x04
#define NE2000_REG_RX_START    0x08

// Functions to write to NE2000 registers
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Initialize the NE2000 network card
void ne2000_init() {
    // Reset the card
    outb(NE2000_IO_BASE + NE2000_REG_RESET, 0x01);
    // Wait for reset to complete
    for (volatile int i = 0; i < 100000; i++);
    
    // Set the command register to 'receive' mode
    outb(NE2000_IO_BASE + NE2000_REG_COMMAND, 0x01);
    
    printf("NE2000 network card initialized.\n");
}

// Send a packet through the NE2000 interface
void ne2000_send(uint8_t* data, uint16_t length) {
    // For simplicity, we are just sending the data directly without packetization.
    // Normally, we would need to handle the packet format, CRC, etc.
    
    // Write the data to the transmit buffer
    for (int i = 0; i < length; i++) {
        outb(NE2000_IO_BASE + NE2000_REG_TX_START + i, data[i]);
    }
    
    // Start transmission by writing to the command register
    outb(NE2000_IO_BASE + NE2000_REG_COMMAND, 0x03); // TX start
    printf("Packet sent.\n");
}

// Receive a packet from the NE2000 interface
void ne2000_receive(uint8_t* buffer, uint16_t length) {
    // For simplicity, just read the data from the RX buffer
    for (int i = 0; i < length; i++) {
        buffer[i] = inb(NE2000_IO_BASE + NE2000_REG_RX_START + i);
    }
    
    printf("Packet received.\n");
}
