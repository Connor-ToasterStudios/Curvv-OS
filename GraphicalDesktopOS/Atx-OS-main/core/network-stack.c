void kernel_main() {
    // Initialize NE2000 Network Interface
    ne2000_init();

    uint8_t packet[1500];  // Buffer to store incoming packets
    
    while (1) {
        // Receive packet from network
        ne2000_receive(packet, sizeof(packet));
        
        // Process the IP packet
        process_ip_packet(packet, sizeof(packet));
        
        // If UDP packet, echo back
        udp_echo_server(packet, sizeof(packet));
    }
}
