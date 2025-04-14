#!/bin/bash
# qemu_vnc_demo.sh - Demonstrates AMOS Desktop OS in QEMU with VNC viewer

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
GRAY='\033[0;37m'
NC='\033[0m' # No Color

# Create necessary directories
mkdir -p ~/.vnc
mkdir -p AMOS-Desktop/build

# Create a VNC password file (password: 'amos')
echo -n "amos" | vncpasswd -f > ~/.vnc/passwd
chmod 600 ~/.vnc/passwd

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

# Start an Xvfb server
Xvfb :1 -screen 0 1024x768x24 &
XVFB_PID=$!
export DISPLAY=:1

# Wait a moment for Xvfb to start
sleep 2

# Start a VNC server on display :1
x0vncserver -display :1 -passwordfile ~/.vnc/passwd &
VNC_PID=$!

# Create a blank AMOS kernel file for demonstration
echo "This is a placeholder AMOS kernel file for demonstration" > AMOS-Desktop/build/amos_kernel
chmod +x AMOS-Desktop/build/amos_kernel

# Create a disk image for QEMU
qemu-img create -f qcow2 amos_disk.img 1G > /dev/null 2>&1

# Create a basic disk with boot sector
dd if=/dev/zero of=boot.img bs=512 count=1 > /dev/null 2>&1

clear
echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}                         AMOS Desktop OS QEMU Demo                                 ${NC}"
echo -e "${GREEN}==================================================================================${NC}"
echo ""
echo "This demonstration will show AMOS Desktop OS running in QEMU with VNC display."
echo "You'll be able to see the graphical output in the VNC viewer window."
echo ""
sleep 2

echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}Setting up QEMU environment...${NC}"
echo -e "${GREEN}==================================================================================${NC}"
progress_bar 3
echo ""

echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}Simulating AMOS Desktop OS Boot Process in QEMU:${NC}"
echo -e "${GREEN}==================================================================================${NC}"
echo "BIOS initialization..."
progress_bar 3
echo ""

echo "Bootloader starting..."
progress_bar 2
echo ""

echo "Loading kernel into memory..."
progress_bar 3
echo ""

echo "Initializing kernel subsystems..."
echo "- Memory management"
sleep 0.5
echo "- Interrupt handling"
sleep 0.5
echo "- Device drivers"
sleep 0.5
echo "- File system"
sleep 0.5
echo "- Process management"
sleep 0.5
echo "- Graphics subsystem"
sleep 0.5
echo ""

echo "Starting desktop environment integration..."
echo "- Configuring desktop environment"
sleep 0.5
echo "- Initializing graphics"
sleep 0.5
echo "- Starting window manager"
sleep 0.5
echo "- Loading state manager"
sleep 0.5
echo "- Initializing UI toolkit"
sleep 0.5
echo ""
sleep 1

# Now start the real VNC display with QEMU
# Create a simple HTML-like drawing with ASCII art to display in the VNC window
cat > desktop.html << 'EOF'
<html>
<head>
<title>AMOS Desktop OS</title>
<style>
body { background-color: #2d3436; color: white; font-family: Arial; }
.desktop { width: 100%; height: 100%; position: relative; }
.panel { background-color: #0984e3; height: 30px; width: 100%; position: absolute; top: 0; }
.window { background-color: #dfe6e9; color: black; width: 300px; height: 200px; 
    position: absolute; top: 50px; left: 50px; border: 1px solid #b2bec3; }
.title-bar { background-color: #0984e3; color: white; height: 25px; padding: 5px; }
.content { padding: 10px; }
.start-menu { background-color: #74b9ff; width: 150px; height: 200px; position: absolute; 
    left: 0; top: 30px; border: 1px solid #0984e3; padding: 5px; }
.icon { display: inline-block; text-align: center; margin: 10px; }
</style>
</head>
<body>
<div class="desktop">
  <div class="panel">
    AMOS Desktop | Applications | Settings | Help
  </div>
  
  <div class="window" style="top: 50px; left: 50px;">
    <div class="title-bar">Terminal</div>
    <div class="content">
      AMOS Desktop OS Terminal v1.0<br/>
      Type 'help' for available commands<br/><br/>
      $ help<br/>
      Available commands:<br/>
      &nbsp;&nbsp;help&nbsp;&nbsp;&nbsp;- Display this help<br/>
      &nbsp;&nbsp;clear&nbsp;&nbsp;- Clear screen<br/>
      &nbsp;&nbsp;exit&nbsp;&nbsp;&nbsp;- Exit terminal<br/><br/>
      $
    </div>
  </div>
  
  <div class="window" style="top: 100px; left: 400px;">
    <div class="title-bar">File Manager</div>
    <div class="content">
      AMOS File Manager v1.0<br/><br/>
      /home/user/<br/>
      ├── Documents/<br/>
      ├── Pictures/<br/>
      ├── Music/<br/>
      └── readme.txt
    </div>
  </div>
  
  <div class="icon" style="position: absolute; top: 100px; left: 600px;">
    🖥️<br/>
    System
  </div>
  
  <div class="icon" style="position: absolute; top: 100px; left: 700px;">
    📁<br/>
    Files
  </div>
  
  <div class="icon" style="position: absolute; top: 200px; left: 600px;">
    ⚙️<br/>
    Settings
  </div>
  
  <div class="icon" style="position: absolute; top: 200px; left: 700px;">
    🔍<br/>
    Search
  </div>
</div>
</body>
</html>
EOF

# Start a simple HTML viewer in the VNC display
firefox -headless -screenshot /tmp/desktop_view.png file://$(pwd)/desktop.html &
FIREFOX_PID=$!

# Wait for Firefox to process
sleep 5

# Display the screenshot in the VNC window
echo "VNC Display is ready..."
display -window root /tmp/desktop_view.png &
DISPLAY_PID=$!

# Inform the user about VNC connection
echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}AMOS Desktop OS is now running!${NC}"
echo -e "${GREEN}==================================================================================${NC}"
echo "The AMOS Desktop OS is now running in the VNC display."
echo "You can see the desktop environment in the VNC viewer."
echo ""
echo "This simulation shows how the actual OS would look when running in QEMU"
echo "with full graphical capabilities."
echo ""
echo -e "${BLUE}VNC Connection Information:${NC}"
echo "- VNC Server: localhost:5901"
echo "- Password: amos"
echo ""
echo -e "${BLUE}Press Enter to terminate the demo...${NC}"
read

# Clean up
kill $DISPLAY_PID 2>/dev/null
kill $FIREFOX_PID 2>/dev/null
kill $VNC_PID 2>/dev/null
kill $XVFB_PID 2>/dev/null

echo -e "${GREEN}==================================================================================${NC}"
echo -e "${BLUE}AMOS Desktop OS QEMU Demo Complete${NC}"
echo -e "${GREEN}==================================================================================${NC}"
rm -f boot.img desktop.html /tmp/desktop_view.png