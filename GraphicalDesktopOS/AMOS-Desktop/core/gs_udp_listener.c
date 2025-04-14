void game_server_main() {
    uint8_t packet[MAX_PACKET_SIZE];  // Buffer for receiving packets

    while (1) {
        // Wait for a packet from the network (the packet will be filled with data)
        ne2000_receive(packet, MAX_PACKET_SIZE);
        
        // Process the received packet as a game packet
        handle_game_packet(packet, MAX_PACKET_SIZE);
    }
}
