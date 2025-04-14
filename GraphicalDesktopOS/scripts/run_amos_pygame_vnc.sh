#!/bin/bash
# run_amos_pygame_vnc.sh - Run AMOS Desktop visualization in Pygame with VNC support

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
GRAY='\033[0;37m'
NC='\033[0m' # No Color

# Set display environment variable
export DISPLAY=:0

# Function to display progress bar
progress_bar() {
    local duration=$1
    local size=40
    
    # Display initial bar
    printf "["
    printf "%${size}s" | tr ' ' ' '
    printf "] 0%%"
    
    # Update the bar
    for ((i=0; i<$duration; i++)); do
        # Calculate percentage
        percent=$((i * 100 / duration))
        
        # Calculate bar size
        bar_size=$((i * size / duration))
        
        # Move cursor back to start of bar
        printf "\r["
        printf "%${bar_size}s" | tr ' ' '#'
        printf "%$((size - bar_size))s" | tr ' ' ' '
        printf "] $percent%%"
        sleep 0.1
    done
    
    # Complete the bar
    printf "\r["
    printf "%${size}s" | tr ' ' '#'
    printf "] 100%%\n"
}

clear
echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}                          AMOS Desktop OS VNC Demo                                 ${NC}"
echo -e "${GREEN}==================================================================================${NC}"
echo ""
echo "This demonstration will show AMOS Desktop OS running with a fully interactive"
echo "desktop environment that you can see in the Replit VNC viewer."
echo ""
sleep 2

echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}Preparing AMOS Desktop environment...${NC}"
echo -e "${GREEN}==================================================================================${NC}"
progress_bar 3
echo ""

echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}Starting AMOS Desktop OS in VNC viewer:${NC}"
echo -e "${GREEN}==================================================================================${NC}"
echo "AMOS Desktop OS is starting..."
echo ""
echo "This will open in the VNC viewer with a fully interactive desktop environment."
echo "You'll see a terminal window, file manager, and desktop interface."
echo ""
echo "The interface includes:"
echo "- Draggable windows (click and drag titlebar)"
echo "- Resizable windows (drag bottom-right corner)"
echo "- Desktop icons and taskbar"
echo "- Right-click context menu"
echo ""
echo -e "${BLUE}Starting VNC display...${NC}"
echo ""

# Run the Pygame visualization
python scripts/pygame_desktop_vnc.py

echo ""
echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}AMOS Desktop OS VNC Demo Completed${NC}"
echo -e "${GREEN}==================================================================================${NC}"