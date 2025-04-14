void kernel_main() {
    // Initialize the NE2000 network interface
    ne2000_init();

    // Simulate a game client sending messages
    game_client_send(1, "Hello from Player 1!");
    game_client_send(2, "Hello from Player 2!");
    
    // Start the game server to receive packets
    game_server_main();
}
