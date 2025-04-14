void kernel_main() {
    // Initialize the NE2000 network interface
    ne2000_init();

    // Start the game server
    game_server_main();
}
