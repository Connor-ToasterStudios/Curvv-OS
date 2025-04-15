#!/bin/bash
# start_desktop.sh - Script to start the AMOS graphical desktop environment

# Function to check if X is running
check_x_running() {
    pgrep -x Xorg >/dev/null
    return $?
}

# Function to start X if not already running
start_x() {
    if ! check_x_running; then
        echo "Starting X server..."
        startx -- -nolisten tcp &
        sleep 3  # Give X time to start
    else
        echo "X server is already running"
    fi
}

# Function to start the window manager
start_window_manager() {
    echo "Starting AMOS Desktop Environment..."
    
    # Set the default background color
    xsetroot -solid "#2f343f"
    
    # Create state directories if they don't exist
    mkdir -p /var/amos/state
    mkdir -p /var/amos/state/app_state
    
    # Start the window manager
    ./bin/amos-wm 2>&1 | tee /var/log/amos-wm.log &
    WM_PID=$!
    
    echo "Window manager started with PID $WM_PID"
}

# Main script execution
echo "Initializing AMOS Desktop Environment..."

# Start X if needed
start_x

# Start the window manager
start_window_manager

# Wait for window manager to initialize
sleep 1

# Start some initial applications
echo "Starting default applications..."

# Terminal
xterm -geometry 80x24+50+50 &

# Set up environment variables
export AMOS_SESSION_PID=$$
export AMOS_WM_PID=$WM_PID

echo "AMOS Desktop Environment is running"
echo "Press Ctrl+C to exit"

# Keep the script running to maintain the session
wait $WM_PID
echo "Window manager has exited"
