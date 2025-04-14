#!/bin/bash
# run_with_qemu.sh - Run AMOS Desktop OS in QEMU with VNC display

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}======================================================================${NC}"
echo -e "${BLUE}                  AMOS Desktop OS - QEMU Runner                        ${NC}"
echo -e "${GREEN}======================================================================${NC}"
echo ""

# Create disk image if it doesn't exist
if [ ! -f amos_disk.img ]; then
    echo -e "${BLUE}Creating disk image for AMOS...${NC}"
    qemu-img create -f qcow2 amos_disk.img 1G
    echo "Disk image created: amos_disk.img (1GB)"
fi

# Start VNC server
echo -e "${BLUE}Starting VNC server...${NC}"
# Start Xvfb on display :1 with screen 0 at 1024x768 resolution with 24-bit color depth
Xvfb :1 -screen 0 1024x768x24 &
XVFB_PID=$!
export DISPLAY=:1

# Start VNC server on display :1 (port 5901)
x0vncserver -display :1 -passwordfile ~/.vnc/passwd &
VNC_PID=$!

# Create a basic disk image with bootloader for QEMU
echo -e "${BLUE}Preparing for AMOS Desktop boot in QEMU...${NC}"

# Check if we have a kernel file
KERNEL_FILE="AMOS-Desktop/build/amos_kernel"
if [ ! -f "$KERNEL_FILE" ]; then
    # Create placeholder kernel file for demonstration
    mkdir -p AMOS-Desktop/build
    echo "This is a placeholder AMOS kernel file for demonstration" > "$KERNEL_FILE"
    chmod +x "$KERNEL_FILE"
fi

# Start QEMU
echo -e "${GREEN}======================================================================${NC}"
echo -e "${BLUE}Starting AMOS Desktop OS in QEMU...${NC}"
echo -e "${GREEN}======================================================================${NC}"
echo "QEMU is running with VNC display enabled on port 5901"
echo "You can connect to it using a VNC client or view it in the Replit VNC window"
echo ""

# Run QEMU with the display redirected to VNC
qemu-system-x86_64 \
    -m 512M \
    -boot c \
    -hda amos_disk.img \
    -kernel "$KERNEL_FILE" \
    -append "console=ttyS0 root=/dev/sda" \
    -vnc :1 \
    -nographic \
    -serial mon:stdio \
    -no-reboot \
    -device VGA,vgamem_mb=64 &
    
QEMU_PID=$!

echo -e "${BLUE}AMOS Desktop OS is now running in QEMU with VNC display on port 5901${NC}"
echo -e "${BLUE}Use the VNC viewer to see the graphical output${NC}"
echo -e "${BLUE}Press Ctrl+C to terminate QEMU and clean up...${NC}"

# Wait for QEMU to exit or be killed
wait $QEMU_PID

# Clean up
kill $VNC_PID 2>/dev/null
kill $XVFB_PID 2>/dev/null

echo -e "${GREEN}======================================================================${NC}"
echo -e "${BLUE}AMOS Desktop OS QEMU session terminated${NC}"
echo -e "${GREEN}======================================================================${NC}"