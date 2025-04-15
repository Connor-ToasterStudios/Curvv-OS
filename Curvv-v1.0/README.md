# AMOS Desktop Environment

The **AMOS Desktop Environment** is a graphical desktop environment extension for AMOS minimal OS. It provides a complete desktop experience with window management, UI toolkit, and persistent state management.

## Key Features

- **Fluxbox-like Window Management** - Window tabbing, stacking, and tiling
- **Resizable Windows** - Full support for window resizing with mouse or keyboard
- **UI Toolkit Integration** - Comprehensive widgets and controls
- **Persistent Application State** - Save and restore application states between sessions
- **Multiple Workspace Support** - Virtual desktops for better organization
- **Themeable User Interface** - Customizable appearance

## Project Structure

The project is organized as follows:

```
AMOS-Desktop/
├── bootloader/        # AMOS bootloader
├── kernel/            # AMOS kernel
│   ├── drivers/       # Device drivers
│   ├── fs/            # File system
│   ├── interrupts/    # Interrupt handlers
│   ├── mm/            # Memory management
│   └── process/       # Process management
├── desktop/           # Desktop environment
│   ├── apps/          # Desktop applications
│   ├── integration/   # Integration layer
│   ├── state/         # State management
│   ├── ui/            # UI toolkit
│   └── wm/            # Window manager
└── scripts/           # Build and utility scripts
```

## Integration with AMOS OS

The desktop environment is tightly integrated with the AMOS kernel through a dedicated integration layer located in `desktop/integration/`. This integration provides:

1. **Seamless Boot Process** - The AMOS kernel automatically initializes the desktop environment
2. **Event Handling** - Keyboard and mouse events are passed from kernel to desktop
3. **Graphics Integration** - Direct framebuffer access for efficient rendering
4. **Resource Management** - Kernel-level resource allocation for desktop components

For more details, see [INTEGRATION.md](AMOS-Desktop/INTEGRATION.md).

## Getting Started

### Running the Demo

To run a demonstration of AMOS Desktop OS:

```bash
bash AMOS-Desktop/run_demo_replit.sh
```

This script simulates the boot process and demonstrates the various components of the desktop environment.

### Building the OS

To build the complete AMOS Desktop OS:

```bash
cd AMOS-Desktop
./build.sh
```

## Sample Applications

The project includes sample applications to demonstrate how applications integrate with the OS:

1. **Terminal** - A command-line interface for interacting with the OS
2. **File Manager** - A graphical file browser

These applications showcase how to:
- Create windows using the window manager
- Use UI components from the toolkit
- Maintain persistent state between sessions

## Technical Details

### Window Manager

The window manager provides the following functionality:

- Window creation, destruction, and management
- Window decorations (title bar, close/minimize/maximize buttons)
- Window positioning and resizing
- Fluxbox-like tabs for grouping windows
- Multiple workspace support

### UI Toolkit

The UI toolkit includes:

- Basic widgets (buttons, text fields, etc.)
- Layout management
- Theming support
- Event handling

### State Management

The state management system provides:

- Persistent storage for application settings and state
- Automatic state saving and restoration
- Configuration management

## Further Development

Future improvements include:

1. Enhanced integration with kernel services
2. More efficient graphics rendering
3. Additional desktop features
4. Expanded application framework