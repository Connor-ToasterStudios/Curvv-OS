// UDP Header
typedef struct {
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) UDPHeader;

// A simple UDP echo server
void udp_echo_server(uint8_t* packet, uint16_t length) {
    UDPHeader* udp_header = (UDPHeader*) packet;
    uint16_t dest_port = udp_header->destination_port;
    
    printf("Received UDP packet on port %u\n", dest_port);
    
    // Echo back the packet (simplified)
    ne2000_send(packet, length);
}
