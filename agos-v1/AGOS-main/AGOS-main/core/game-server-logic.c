#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define GAME_SERVER_PORT 12345 // Port the server will listen on
#define MAX_PACKET_SIZE 1500  // Maximum size of the packet

// Define a simple game packet structure (you can expand this as needed)
typedef struct {
    uint32_t player_id;    // Player's unique ID
    uint8_t message[1024]; // Message from player
} GamePacket;

// Function to handle incoming game packets
void handle_game_packet(uint8_t* packet, uint16_t length) {
    // Assume the packet is a GamePacket
    GamePacket* game_packet = (GamePacket*) packet;

    // For simplicity, just print the incoming message and player ID
    printf("Received packet from player ID: %u\n", game_packet->player_id);
    printf("Message: %s\n", game_packet->message);

    // Echo back the packet (respond to the client)
    // We'll send it back to the same player, with the same port and address.
    // In a real server, you would need to keep track of client IPs/ports, but for simplicity:
    ne2000_send(packet, length);
    printf("Sent packet back to player ID: %u\n", game_packet->player_id);
}
