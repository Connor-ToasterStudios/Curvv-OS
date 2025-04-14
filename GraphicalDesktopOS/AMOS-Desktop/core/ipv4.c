// IPv4 header structure
typedef struct {
    uint8_t  version_and_header_length;
    uint8_t  type_of_service;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_and_fragment_offset;
    uint8_t  time_to_live;
    uint8_t  protocol;
    uint16_t header_checksum;
    uint32_t source_ip;
    uint32_t destination_ip;
} __attribute__((packed)) IPv4Header;

// Simple IP packet processing
void process_ip_packet(uint8_t* packet, uint16_t length) {
    IPv4Header* ip_header = (IPv4Header*) packet;
    
    // Check the version and header length
    if ((ip_header->version_and_header_length >> 4) != 4) {
        printf("Invalid IP version\n");
        return;
    }

    // Just printing out the source and destination IP addresses for now
    uint32_t src_ip = ip_header->source_ip;
    uint32_t dst_ip = ip_header->destination_ip;

    printf("Source IP: %u.%u.%u.%u\n", (src_ip >> 24) & 0xFF, (src_ip >> 16) & 0xFF, 
                                       (src_ip >> 8) & 0xFF, src_ip & 0xFF);
    printf("Destination IP: %u.%u.%u.%u\n", (dst_ip >> 24) & 0xFF, (dst_ip >> 16) & 0xFF, 
                                           (dst_ip >> 8) & 0xFF, dst_ip & 0xFF);
}
