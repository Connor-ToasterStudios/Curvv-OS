#!/usr/bin/env python3
"""
Enhanced AMOS Desktop OS - Pure Native Implementation
This script creates a fully functional representation of the AMOS Desktop OS
that can be viewed through Replit's VNC viewer.

Features:
- Functional terminal with real command processing
- Working window management (drag, resize, minimize, maximize)
- Web browser simulation
- Settings menu with theme customization
- Native rendering using only Python/Pygame (no HTML)
"""

import pygame
import sys
import os
import time
import random
from datetime import datetime

# Initialize pygame
pygame.init()

# Set up the display
WIDTH, HEIGHT = 1024, 768
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("AMOS Desktop OS")

# Define colors
BLUE = (9, 132, 227)
DARK_BLUE = (45, 52, 54)
LIGHT_BLUE = (116, 185, 255)
GRAY = (223, 230, 233)
DARK_GRAY = (178, 190, 195)
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GREEN = (0, 184, 148)
RED = (255, 0, 0)
YELLOW = (253, 203, 110)
ORANGE = (255, 165, 0)
PURPLE = (128, 0, 128)
LIGHT_GREEN = (120, 224, 143)
DARK_GREEN = (0, 100, 0)
DARK_RED = (139, 0, 0)

# Load fonts
try:
    system_font = pygame.font.SysFont("Arial", 14)
    title_font = pygame.font.SysFont("Arial", 16)
    terminal_font = pygame.font.SysFont("Courier New", 14)
    large_font = pygame.font.SysFont("Arial", 20)
    menu_font = pygame.font.SysFont("Arial", 16)
except:
    # Fallback to default font if Arial is not available
    system_font = pygame.font.Font(None, 20)
    title_font = pygame.font.Font(None, 22)
    terminal_font = pygame.font.Font(None, 20)
    large_font = pygame.font.Font(None, 24)
    menu_font = pygame.font.Font(None, 22)

# Application state storage
class AppState:
    def __init__(self):
        self.windows = []
        self.active_window = None
        self.menu_open = False
        self.show_desktop = True
        self.right_click_menu = None
        self.start_menu_open = False
        self.settings_open = False
        self.current_theme = "blue"
        self.themes = {
            "blue": {
                "desktop": DARK_BLUE,
                "taskbar": BLUE,
                "window_title": BLUE,
                "button": LIGHT_BLUE,
                "accent": LIGHT_BLUE
            },
            "green": {
                "desktop": (34, 66, 41),
                "taskbar": (46, 139, 87),
                "window_title": (46, 139, 87),
                "button": (120, 224, 143),
                "accent": (120, 224, 143)
            },
            "dark": {
                "desktop": (15, 15, 15),
                "taskbar": (50, 50, 50),
                "window_title": (60, 60, 60),
                "button": (100, 100, 100),
                "accent": (80, 80, 80)
            },
            "purple": {
                "desktop": (40, 20, 60),
                "taskbar": (102, 51, 153),
                "window_title": (102, 51, 153),
                "button": (153, 102, 204),
                "accent": (153, 102, 204)
            }
        }
        
    def get_theme_color(self, element):
        return self.themes[self.current_theme][element]
    
    def set_theme(self, theme_name):
        if theme_name in self.themes:
            self.current_theme = theme_name
            return True
        return False

# Create app state
app_state = AppState()

# Window class
class Window:
    def __init__(self, x, y, width, height, title, content_type="default"):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.title = title
        self.content_type = content_type
        self.dragging = False
        self.drag_offset_x = 0
        self.drag_offset_y = 0
        self.content = []
        self.resizing = False
        self.minimized = False
        self.maximized = False
        self.original_dims = (x, y, width, height)
        self.active = True
        self.show_controls = True
        self.terminal_input = ""
        self.cursor_visible = True
        self.cursor_timer = 0
        self.terminal_prompt = "$ "
        self.terminal_history = [
            "AMOS Desktop OS Terminal v1.0",
            "Type 'help' for available commands",
            ""
        ]
        self.file_contents = [
            "AMOS File Manager v1.0",
            "",
            "/home/user/",
            "├── Documents/",
            "│   ├── work_notes.txt",
            "│   └── project_plans.md",
            "├── Pictures/",
            "│   ├── vacation.jpg",
            "│   └── screenshot.png",
            "├── Music/",
            "│   ├── playlist1/",
            "│   └── song.mp3",
            "└── readme.txt"
        ]
        self.current_directory = "/home/user/"
        self.browser_url = "https://amos-os.org"
        self.browser_content = [
            "Welcome to AMOS Desktop OS Browser",
            "",
            "This is a simulated web browser for the AMOS Desktop OS.",
            "It demonstrates how web browsing would work in the OS.",
            "",
            "Current URL: " + self.browser_url
        ]
        self.settings_tab = "general"
        self.settings_tabs = ["General", "Appearance", "Privacy", "Network", "About"]
        self.selected_theme = app_state.current_theme
        
    def process_terminal_command(self, command):
        parts = command.strip().split()
        if not parts:
            return
            
        cmd = parts[0].lower()
        args = parts[1:] if len(parts) > 1 else []
        
        if cmd == "help":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append("Available commands:")
            self.terminal_history.append("  help      - Display this help message")
            self.terminal_history.append("  clear     - Clear the terminal screen")
            self.terminal_history.append("  echo      - Display text")
            self.terminal_history.append("  version   - Display AMOS version")
            self.terminal_history.append("  ls        - List files in current directory")
            self.terminal_history.append("  cd        - Change directory")
            self.terminal_history.append("  cat       - Display file contents")
            self.terminal_history.append("  pwd       - Show current directory")
            self.terminal_history.append("  date      - Display current date and time")
            self.terminal_history.append("  uname     - Display system information")
            self.terminal_history.append("  shutdown  - Shutdown the system")
            self.terminal_history.append("  exit      - Close the terminal")
            
        elif cmd == "clear":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history = []
            
        elif cmd == "echo":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append(" ".join(args))
            
        elif cmd == "version":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append("AMOS Desktop OS v1.0")
            self.terminal_history.append("Kernel: AMOS 1.0.5")
            self.terminal_history.append("Build: 20250414")
            self.terminal_history.append("Copyright (c) 2025 AMOS Team")
            
        elif cmd == "ls":
            self.terminal_history.append(self.terminal_prompt + command)
            if self.current_directory == "/home/user/":
                self.terminal_history.append("Documents/  Pictures/  Music/  readme.txt")
            elif self.current_directory == "/home/user/Documents/":
                self.terminal_history.append("work_notes.txt  project_plans.md")
            elif self.current_directory == "/home/user/Pictures/":
                self.terminal_history.append("vacation.jpg  screenshot.png")
            elif self.current_directory == "/home/user/Music/":
                self.terminal_history.append("playlist1/  song.mp3")
            else:
                self.terminal_history.append("No files found")
                
        elif cmd == "cd":
            self.terminal_history.append(self.terminal_prompt + command)
            if not args:
                self.current_directory = "/home/user/"
                self.terminal_history.append("Changed to: /home/user/")
            elif args[0] == "..":
                if self.current_directory != "/home/user/":
                    self.current_directory = "/home/user/"
                    self.terminal_history.append("Changed to: /home/user/")
                else:
                    self.terminal_history.append("Already at root directory")
            elif args[0] == "Documents" or args[0] == "Documents/":
                self.current_directory = "/home/user/Documents/"
                self.terminal_history.append("Changed to: /home/user/Documents/")
            elif args[0] == "Pictures" or args[0] == "Pictures/":
                self.current_directory = "/home/user/Pictures/"
                self.terminal_history.append("Changed to: /home/user/Pictures/")
            elif args[0] == "Music" or args[0] == "Music/":
                self.current_directory = "/home/user/Music/"
                self.terminal_history.append("Changed to: /home/user/Music/")
            else:
                self.terminal_history.append(f"Directory not found: {args[0]}")
                
        elif cmd == "pwd":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append(self.current_directory)
            
        elif cmd == "cat":
            self.terminal_history.append(self.terminal_prompt + command)
            if not args:
                self.terminal_history.append("Error: No file specified")
            elif args[0] == "readme.txt":
                self.terminal_history.append("# AMOS Desktop OS")
                self.terminal_history.append("Welcome to AMOS Desktop OS, a lightweight desktop environment")
                self.terminal_history.append("designed for efficiency and customizability.")
                self.terminal_history.append("")
                self.terminal_history.append("Feel free to explore the system and try out the features.")
            elif args[0] == "work_notes.txt" and self.current_directory == "/home/user/Documents/":
                self.terminal_history.append("Meeting notes from 2025-04-10:")
                self.terminal_history.append("- Discussed OS architecture improvements")
                self.terminal_history.append("- Assigned tasks for next sprint")
                self.terminal_history.append("- Next review: 2025-04-20")
            elif args[0] == "project_plans.md" and self.current_directory == "/home/user/Documents/":
                self.terminal_history.append("# AMOS Desktop Project Plan")
                self.terminal_history.append("## Timeline")
                self.terminal_history.append("- Phase 1: Core Desktop Environment (DONE)")
                self.terminal_history.append("- Phase 2: Application Framework (DONE)")
                self.terminal_history.append("- Phase 3: System Integration (IN PROGRESS)")
                self.terminal_history.append("- Phase 4: Performance Optimization (PENDING)")
            else:
                self.terminal_history.append(f"File not found: {args[0]}")
                
        elif cmd == "date":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append(datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
            
        elif cmd == "uname":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append("AMOS 1.0.5 x86_64")
            if "-a" in args:
                self.terminal_history.append("AMOS 1.0.5 desktop-vm 1.0.5-generic #1 SMP PREEMPT")
                self.terminal_history.append("2025-04-14 x86_64 x86_64 x86_64 AMOS Desktop")
                
        elif cmd == "shutdown":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append("Shutting down system...")
            self.terminal_history.append("System will now halt.")
            self.terminal_history.append("")
            self.terminal_history.append("(This is a simulation - the system won't actually shut down)")
            
        elif cmd == "exit":
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append("Closing terminal...")
            return "close"
            
        else:
            self.terminal_history.append(self.terminal_prompt + command)
            self.terminal_history.append(f"Command not found: {cmd}")
            self.terminal_history.append("Type 'help' for available commands")
            
        self.terminal_history.append("")
        self.terminal_history.append(self.terminal_prompt)
        self.terminal_input = ""
        return None
        
    def handle_event(self, event):
        if self.minimized:
            return
            
        if event.type == pygame.MOUSEBUTTONDOWN:
            # Check if click is on the window
            if (self.x <= event.pos[0] <= self.x + self.width and
                self.y <= event.pos[1] <= self.y + self.height):
                
                # Set as active window
                self.active = True
                app_state.active_window = self
                
                # Check for title bar controls (close, maximize, minimize)
                if self.show_controls:
                    # Close button
                    if (self.x + self.width - 25 <= event.pos[0] <= self.x + self.width - 10 and
                        self.y + 8 <= event.pos[1] <= self.y + 23):
                        if self.content_type == "settings":
                            app_state.settings_open = False
                        return "close"
                        
                    # Maximize button
                    elif (self.x + self.width - 45 <= event.pos[0] <= self.x + self.width - 30 and
                          self.y + 8 <= event.pos[1] <= self.y + 23):
                        if not self.maximized:
                            self.original_dims = (self.x, self.y, self.width, self.height)
                            self.x = 0
                            self.y = 30  # Below taskbar
                            self.width = WIDTH
                            self.height = HEIGHT - 30
                            self.maximized = True
                        else:
                            self.x, self.y, self.width, self.height = self.original_dims
                            self.maximized = False
                        return "maximize"
                        
                    # Minimize button
                    elif (self.x + self.width - 65 <= event.pos[0] <= self.x + self.width - 50 and
                          self.y + 8 <= event.pos[1] <= self.y + 23):
                        self.minimized = True
                        return "minimize"
                
                # Check for click in title bar (for dragging)
                if (self.y <= event.pos[1] <= self.y + 30 and not self.maximized):
                    self.dragging = True
                    self.drag_offset_x = event.pos[0] - self.x
                    self.drag_offset_y = event.pos[1] - self.y
                    
                # Check for click on resize handle (bottom right)
                elif (not self.maximized and
                      self.x + self.width - 20 <= event.pos[0] <= self.x + self.width and
                      self.y + self.height - 20 <= event.pos[1] <= self.y + self.height):
                    self.resizing = True
                    
                # Handle content-specific interactions
                elif self.y + 30 <= event.pos[1] <= self.y + self.height:
                    # Handle settings tabs
                    if self.content_type == "settings":
                        tab_width = self.width / len(self.settings_tabs)
                        if self.y + 30 <= event.pos[1] <= self.y + 60:
                            for i, tab in enumerate(self.settings_tabs):
                                if self.x + i * tab_width <= event.pos[0] <= self.x + (i + 1) * tab_width:
                                    self.settings_tab = tab.lower()
                                    break
                                    
                        # Handle theme selection
                        if self.settings_tab == "appearance":
                            button_height = 40
                            button_width = self.width - 40
                            # Blue theme button
                            if (self.x + 20 <= event.pos[0] <= self.x + button_width and
                                self.y + 100 <= event.pos[1] <= self.y + 100 + button_height):
                                self.selected_theme = "blue"
                                app_state.set_theme("blue")
                            # Green theme button
                            elif (self.x + 20 <= event.pos[0] <= self.x + button_width and
                                  self.y + 150 <= event.pos[1] <= self.y + 150 + button_height):
                                self.selected_theme = "green"
                                app_state.set_theme("green")
                            # Dark theme button
                            elif (self.x + 20 <= event.pos[0] <= self.x + button_width and
                                  self.y + 200 <= event.pos[1] <= self.y + 200 + button_height):
                                self.selected_theme = "dark"
                                app_state.set_theme("dark")
                            # Purple theme button
                            elif (self.x + 20 <= event.pos[0] <= self.x + button_width and
                                  self.y + 250 <= event.pos[1] <= self.y + 250 + button_height):
                                self.selected_theme = "purple"
                                app_state.set_theme("purple")
                                
                    # Handle browser navigation buttons
                    elif self.content_type == "browser":
                        button_y = self.y + 65
                        
                        # Back button
                        if (self.x + 10 <= event.pos[0] <= self.x + 70 and
                            button_y <= event.pos[1] <= button_y + 25):
                            # Simulate going back
                            self.browser_content.append("Navigating back...")
                            
                        # Refresh button
                        elif (self.x + 80 <= event.pos[0] <= self.x + 160 and
                              button_y <= event.pos[1] <= button_y + 25):
                            # Simulate refreshing
                            self.browser_content = [
                                f"Browsing: {self.browser_url} (Refreshed)",
                                "",
                                "Page has been refreshed.",
                                "",
                                "This is a simulated web page.",
                                "Current URL: " + self.browser_url
                            ]
                            
                        # Home button
                        elif (self.x + 170 <= event.pos[0] <= self.x + 230 and
                              button_y <= event.pos[1] <= button_y + 25):
                            # Go to home page
                            self.browser_url = "https://amos-os.org"
                            self.browser_content = [
                                "AMOS Desktop OS - Home Page",
                                "",
                                "Welcome to the AMOS Desktop OS official website!",
                                "",
                                "• Download the latest version",
                                "• View documentation",
                                "• Report issues",
                                "• Join the community",
                                "",
                                "Current URL: " + self.browser_url
                            ]
            else:
                self.active = False
                
        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:  # Left mouse button
                self.dragging = False
                self.resizing = False
                
        elif event.type == pygame.MOUSEMOTION:
            if self.dragging and not self.maximized:
                self.x = event.pos[0] - self.drag_offset_x
                self.y = event.pos[1] - self.drag_offset_y
                
                # Keep window on screen
                self.x = max(0, min(self.x, WIDTH - 100))
                self.y = max(30, min(self.y, HEIGHT - 100))
                
            elif self.resizing and not self.maximized:
                self.width = max(200, event.pos[0] - self.x)
                self.height = max(150, event.pos[1] - self.y)
                
        elif event.type == pygame.KEYDOWN and self.active:
            if self.content_type == "terminal":
                if event.key == pygame.K_RETURN:
                    result = self.process_terminal_command(self.terminal_input)
                    if result == "close":
                        return "close"
                elif event.key == pygame.K_BACKSPACE:
                    self.terminal_input = self.terminal_input[:-1]
                else:
                    # Only add printable characters
                    if event.unicode.isprintable():
                        self.terminal_input += event.unicode
            
            elif self.content_type == "browser":
                if event.key == pygame.K_RETURN:
                    # Simulate URL navigation
                    if self.browser_url.startswith("http"):
                        self.browser_content = [
                            f"Browsing: {self.browser_url}",
                            "",
                            "This is a simulated web page.",
                            "In a real implementation, this would render the actual web content.",
                            "",
                            "Current URL: " + self.browser_url
                        ]
                elif event.key == pygame.K_BACKSPACE:
                    self.browser_url = self.browser_url[:-1]
                else:
                    # Only add printable characters to URL
                    if event.unicode.isprintable():
                        self.browser_url += event.unicode
        
        return None  # No window action performed
    
    def draw(self, surface):
        if self.minimized:
            return
            
        title_bar_color = app_state.get_theme_color("window_title")
        button_color = app_state.get_theme_color("button")
        
        # Draw window background
        pygame.draw.rect(surface, GRAY, (self.x, self.y, self.width, self.height))
        
        # Draw window border
        border_color = BLUE if self.active else DARK_GRAY
        pygame.draw.rect(surface, border_color, (self.x, self.y, self.width, self.height), 1)
        
        # Draw title bar
        pygame.draw.rect(surface, title_bar_color, (self.x, self.y, self.width, 30))
        
        # Draw window title
        title_text = title_font.render(self.title, True, WHITE)
        surface.blit(title_text, (self.x + 10, self.y + 7))
        
        # Draw window controls (close, maximize, minimize)
        if self.show_controls:
            pygame.draw.rect(surface, RED, (self.x + self.width - 25, self.y + 8, 15, 15))
            pygame.draw.rect(surface, YELLOW, (self.x + self.width - 45, self.y + 8, 15, 15))
            pygame.draw.rect(surface, GREEN, (self.x + self.width - 65, self.y + 8, 15, 15))
        
        # Draw window content based on type
        if self.content_type == "terminal":
            self.draw_terminal(surface)
        elif self.content_type == "file_manager":
            self.draw_file_manager(surface)
        elif self.content_type == "browser":
            self.draw_browser(surface)
        elif self.content_type == "settings":
            self.draw_settings(surface)
        else:
            self.draw_default(surface)
        
        # Draw resize handle (if not maximized)
        if not self.maximized:
            pygame.draw.lines(surface, DARK_GRAY, False, [
                (self.x + self.width - 15, self.y + self.height),
                (self.x + self.width, self.y + self.height),
                (self.x + self.width, self.y + self.height - 15)
            ], 2)
    
    def draw_terminal(self, surface):
        # Draw terminal background
        pygame.draw.rect(surface, BLACK, (self.x, self.y + 30, self.width, self.height - 30))
        
        # Draw terminal content
        line_height = 20
        visible_lines = (self.height - 40) // line_height
        
        # Calculate how many lines to show (including current input)
        content_to_show = self.terminal_history[-min(len(self.terminal_history), visible_lines-1):]
        
        # Draw history lines
        for i, line in enumerate(content_to_show):
            line_text = terminal_font.render(line, True, GREEN)
            surface.blit(line_text, (self.x + 10, self.y + 35 + i * line_height))
        
        # Draw current input line
        input_y = self.y + 35 + (len(content_to_show)) * line_height
        
        # Only show cursor if window is active
        if self.active:
            # Blink cursor
            self.cursor_timer += 1
            if self.cursor_timer > 30:
                self.cursor_visible = not self.cursor_visible
                self.cursor_timer = 0
        
            current_input = self.terminal_prompt + self.terminal_input
            if self.cursor_visible:
                current_input += "_"
        else:
            current_input = self.terminal_prompt + self.terminal_input
        
        input_text = terminal_font.render(current_input, True, GREEN)
        surface.blit(input_text, (self.x + 10, input_y))
    
    def draw_file_manager(self, surface):
        # Draw file manager background
        pygame.draw.rect(surface, WHITE, (self.x, self.y + 30, self.width, self.height - 30))
        
        # Draw location bar
        pygame.draw.rect(surface, LIGHT_BLUE, (self.x + 5, self.y + 35, self.width - 10, 25))
        location_text = system_font.render(self.current_directory, True, WHITE)
        surface.blit(location_text, (self.x + 10, self.y + 40))
        
        # Draw file manager content
        line_height = 20
        for i, line in enumerate(self.file_contents):
            text_color = BLUE if "├──" in line or "└──" in line or line.endswith('/') else BLACK
            
            # Indent subdirectory content
            indent = 0
            if "│   " in line:
                indent = 20
                
            line_text = system_font.render(line, True, text_color)
            surface.blit(line_text, (self.x + 10 + indent, self.y + 65 + i * line_height))
    
    def draw_browser(self, surface):
        # Draw browser background
        pygame.draw.rect(surface, WHITE, (self.x, self.y + 30, self.width, self.height - 30))
        
        # Draw URL bar
        pygame.draw.rect(surface, LIGHT_BLUE, (self.x + 5, self.y + 35, self.width - 10, 25))
        url_text = system_font.render(self.browser_url, True, WHITE)
        surface.blit(url_text, (self.x + 10, self.y + 40))
        
        # Draw browser navigation buttons
        button_y = self.y + 65
        pygame.draw.rect(surface, LIGHT_BLUE, (self.x + 10, button_y, 60, 25))
        back_text = system_font.render("Back", True, WHITE)
        surface.blit(back_text, (self.x + 20, button_y + 5))
        
        pygame.draw.rect(surface, LIGHT_BLUE, (self.x + 80, button_y, 80, 25))
        refresh_text = system_font.render("Refresh", True, WHITE)
        surface.blit(refresh_text, (self.x + 90, button_y + 5))
        
        pygame.draw.rect(surface, LIGHT_BLUE, (self.x + 170, button_y, 60, 25))
        home_text = system_font.render("Home", True, WHITE)
        surface.blit(home_text, (self.x + 180, button_y + 5))
        
        # Draw browser content
        content_start_y = button_y + 35
        for i, line in enumerate(self.browser_content):
            color = BLACK
            if i == 0:  # Title line
                color = BLUE
            line_text = system_font.render(line, True, color)
            surface.blit(line_text, (self.x + 10, content_start_y + i * 20))
            
        # Draw a native browser rendering (purely visual, not actual HTML)
        if "Home Page" in self.browser_content[0]:
            # Home page content - render natively
            logo_width = 200
            logo_height = 100
            logo_x = self.x + (self.width - logo_width) // 2
            logo_y = content_start_y + 200
            
            # Draw AMOS logo
            pygame.draw.rect(surface, BLUE, (logo_x, logo_y, logo_width, logo_height), 0, 10)
            pygame.draw.rect(surface, LIGHT_BLUE, (logo_x + 10, logo_y + 10, logo_width - 20, logo_height - 20), 0, 5)
            
            # Draw logo text
            logo_text = large_font.render("AMOS OS", True, WHITE)
            surface.blit(logo_text, (logo_x + (logo_width - logo_text.get_width()) // 2, 
                                      logo_y + (logo_height - logo_text.get_height()) // 2))
            
            # Draw download button
            download_btn_x = self.x + (self.width - 150) // 2
            download_btn_y = logo_y + logo_height + 20
            pygame.draw.rect(surface, GREEN, (download_btn_x, download_btn_y, 150, 40), 0, 5)
            download_text = system_font.render("Download v1.0.5", True, WHITE)
            surface.blit(download_text, (download_btn_x + (150 - download_text.get_width()) // 2, 
                                         download_btn_y + (40 - download_text.get_height()) // 2))
    
    def draw_settings(self, surface):
        # Draw settings background
        pygame.draw.rect(surface, WHITE, (self.x, self.y + 30, self.width, self.height - 30))
        
        # Draw settings tabs
        tab_width = self.width / len(self.settings_tabs)
        for i, tab in enumerate(self.settings_tabs):
            # Highlight selected tab
            tab_color = LIGHT_BLUE if tab.lower() == self.settings_tab else GRAY
            pygame.draw.rect(surface, tab_color, (self.x + i * tab_width, self.y + 30, tab_width, 30))
            
            # Draw tab divider
            pygame.draw.line(surface, DARK_GRAY, 
                            (self.x + (i + 1) * tab_width, self.y + 30),
                            (self.x + (i + 1) * tab_width, self.y + 60), 1)
            
            # Draw tab text
            tab_text = system_font.render(tab, True, BLACK)
            text_x = self.x + i * tab_width + (tab_width - tab_text.get_width()) / 2
            surface.blit(tab_text, (text_x, self.y + 38))
        
        # Draw selected tab content
        content_y = self.y + 70
        content_x = self.x + 20
        
        if self.settings_tab == "general":
            title_text = large_font.render("General Settings", True, BLACK)
            surface.blit(title_text, (content_x, content_y))
            
            option_list = [
                "Startup applications",
                "Default applications",
                "Power management",
                "Menu behavior"
            ]
            
            for i, option in enumerate(option_list):
                y_pos = content_y + 40 + i * 30
                pygame.draw.rect(surface, LIGHT_BLUE, (content_x, y_pos, self.width - 40, 25))
                option_text = system_font.render(option, True, WHITE)
                surface.blit(option_text, (content_x + 10, y_pos + 5))
        
        elif self.settings_tab == "appearance":
            title_text = large_font.render("Appearance Settings", True, BLACK)
            surface.blit(title_text, (content_x, content_y))
            
            theme_text = system_font.render("Select a theme:", True, BLACK)
            surface.blit(theme_text, (content_x, content_y + 40))
            
            # Blue theme button
            theme_button_width = self.width - 40
            pygame.draw.rect(surface, BLUE, (content_x, content_y + 60, theme_button_width, 40))
            if self.selected_theme == "blue":
                pygame.draw.rect(surface, WHITE, (content_x, content_y + 60, theme_button_width, 40), 2)
            theme_text = system_font.render("Blue Theme", True, WHITE)
            surface.blit(theme_text, (content_x + 10, content_y + 70))
            
            # Green theme button
            pygame.draw.rect(surface, (46, 139, 87), (content_x, content_y + 110, theme_button_width, 40))
            if self.selected_theme == "green":
                pygame.draw.rect(surface, WHITE, (content_x, content_y + 110, theme_button_width, 40), 2)
            theme_text = system_font.render("Green Theme", True, WHITE)
            surface.blit(theme_text, (content_x + 10, content_y + 120))
            
            # Dark theme button
            pygame.draw.rect(surface, (50, 50, 50), (content_x, content_y + 160, theme_button_width, 40))
            if self.selected_theme == "dark":
                pygame.draw.rect(surface, WHITE, (content_x, content_y + 160, theme_button_width, 40), 2)
            theme_text = system_font.render("Dark Theme", True, WHITE)
            surface.blit(theme_text, (content_x + 10, content_y + 170))
            
            # Purple theme button
            pygame.draw.rect(surface, (102, 51, 153), (content_x, content_y + 210, theme_button_width, 40))
            if self.selected_theme == "purple":
                pygame.draw.rect(surface, WHITE, (content_x, content_y + 210, theme_button_width, 40), 2)
            theme_text = system_font.render("Purple Theme", True, WHITE)
            surface.blit(theme_text, (content_x + 10, content_y + 220))
            
        elif self.settings_tab == "privacy":
            title_text = large_font.render("Privacy Settings", True, BLACK)
            surface.blit(title_text, (content_x, content_y))
            
            # Privacy options
            options = [
                "Location services: OFF",
                "Usage statistics: OFF",
                "Cookie management",
                "Data collection: MINIMAL"
            ]
            
            for i, option in enumerate(options):
                y_pos = content_y + 40 + i * 30
                pygame.draw.rect(surface, LIGHT_BLUE, (content_x, y_pos, self.width - 40, 25))
                option_text = system_font.render(option, True, WHITE)
                surface.blit(option_text, (content_x + 10, y_pos + 5))
                
        elif self.settings_tab == "network":
            title_text = large_font.render("Network Settings", True, BLACK)
            surface.blit(title_text, (content_x, content_y))
            
            # Network status
            status_text = system_font.render("Network Status: Connected", True, BLACK)
            surface.blit(status_text, (content_x, content_y + 40))
            
            # IP info
            ip_text = system_font.render("IP Address: 192.168.1.100", True, BLACK)
            surface.blit(ip_text, (content_x, content_y + 70))
            
            # Network options
            pygame.draw.rect(surface, LIGHT_BLUE, (content_x, content_y + 100, 180, 30))
            option_text = system_font.render("Connection Settings", True, WHITE)
            surface.blit(option_text, (content_x + 10, content_y + 105))
            
            pygame.draw.rect(surface, LIGHT_BLUE, (content_x, content_y + 140, 180, 30))
            option_text = system_font.render("Configure Proxy", True, WHITE)
            surface.blit(option_text, (content_x + 10, content_y + 145))
            
        elif self.settings_tab == "about":
            title_text = large_font.render("About AMOS Desktop OS", True, BLACK)
            surface.blit(title_text, (content_x, content_y))
            
            info_lines = [
                "AMOS Desktop OS v1.0",
                "Copyright © 2025 AMOS Team",
                "",
                "A lightweight desktop environment built for efficiency and flexibility.",
                "",
                "System Information:",
                "- Kernel: AMOS 1.0.5",
                "- Build: 20250414",
                "- Memory: 512 MB",
                "- Storage: 1 GB",
                "",
                "License: Open Source (MIT)"
            ]
            
            for i, line in enumerate(info_lines):
                line_text = system_font.render(line, True, BLACK)
                surface.blit(line_text, (content_x, content_y + 40 + i * 20))
            
            # Draw AMOS logo
            logo_x = content_x + 300
            logo_y = content_y + 60
            pygame.draw.rect(surface, BLUE, (logo_x, logo_y, 100, 50), 0, 5)
            logo_text = system_font.render("AMOS", True, WHITE)
            surface.blit(logo_text, (logo_x + 25, logo_y + 15))
    
    def draw_default(self, surface):
        # Draw default content
        content_text = system_font.render("Window Content", True, BLACK)
        surface.blit(content_text, (self.x + 10, self.y + 40))

class TaskBar:
    def __init__(self, width):
        self.width = width
        self.height = 30
        self.buttons = [
            {"text": "Menu", "width": 50, "action": "menu"},
            {"text": "Terminal", "width": 80, "action": "terminal"},
            {"text": "Files", "width": 60, "action": "files"},
            {"text": "Browser", "width": 80, "action": "browser"},
            {"text": "Settings", "width": 80, "action": "settings"}
        ]
        self.active_button = None
        self.minimized_windows = []
    
    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1 and 0 <= event.pos[1] <= self.height:
                x_offset = 10
                for button in self.buttons:
                    if x_offset <= event.pos[0] <= x_offset + button["width"]:
                        self.active_button = button["action"]
                        return button["action"]
                    x_offset += button["width"] + 5
                    
                # Check for clicks on minimized windows
                x_pos = self.width - 200
                for i, window in enumerate(self.minimized_windows):
                    if x_pos <= event.pos[0] <= x_pos + 120:
                        return ["restore", i]
                    x_pos -= 130
        
        # No action performed
        return None
    
    def draw(self, surface):
        # Draw taskbar background
        taskbar_color = app_state.get_theme_color("taskbar")
        button_color = app_state.get_theme_color("button")
        
        pygame.draw.rect(surface, taskbar_color, (0, 0, self.width, self.height))
        
        # Draw buttons
        x_offset = 10
        for button in self.buttons:
            # Use a brighter color for active button
            current_color = button_color
            if self.active_button == button["action"]:
                # Lighten the color for active button
                current_color = tuple(min(255, c + 40) for c in button_color)
                
            pygame.draw.rect(surface, current_color, (x_offset, 5, button["width"], 20))
            button_text = system_font.render(button["text"], True, WHITE)
            surface.blit(button_text, (x_offset + 5, 7))
            x_offset += button["width"] + 5
        
        # Draw time
        current_time = datetime.now().strftime("%H:%M:%S")
        time_text = system_font.render(current_time, True, WHITE)
        surface.blit(time_text, (self.width - 70, 7))
        
        # Draw date
        current_date = datetime.now().strftime("%Y-%m-%d")
        date_text = system_font.render(current_date, True, WHITE)
        surface.blit(date_text, (self.width - 180, 7))
        
        # Draw minimized windows
        x_pos = self.width - 200
        for window in self.minimized_windows:
            pygame.draw.rect(surface, LIGHT_BLUE, (x_pos, 5, 120, 20))
            win_text = system_font.render(window.title, True, WHITE)
            surface.blit(win_text, (x_pos + 5, 7))
            x_pos -= 130

class DesktopIcon:
    def __init__(self, x, y, icon_type):
        self.x = x
        self.y = y
        self.width = 60
        self.height = 60
        self.icon_type = icon_type
        
        # Set icon text based on type
        if icon_type == "system":
            self.text = "System"
            self.icon_char = "🖥️"
        elif icon_type == "files":
            self.text = "Files"
            self.icon_char = "📁"
        elif icon_type == "settings":
            self.text = "Settings"
            self.icon_char = "⚙️"
        elif icon_type == "browser":
            self.text = "Browser"
            self.icon_char = "🌐"
        elif icon_type == "terminal":
            self.text = "Terminal"
            self.icon_char = "📺"
        else:
            self.text = "Icon"
            self.icon_char = "📄"
    
    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if (self.x <= event.pos[0] <= self.x + self.width and
                self.y <= event.pos[1] <= self.y + self.height):
                return self.icon_type
        return None
    
    def draw(self, surface):
        # Draw icon background
        icon_color = app_state.get_theme_color("accent")
        pygame.draw.rect(surface, icon_color, (self.x, self.y, self.width, self.height), 0, 5)
        
        # Draw icon
        icon_text = pygame.font.SysFont("Arial", 30).render(self.icon_char, True, WHITE)
        surface.blit(icon_text, (self.x + self.width//2 - 12, self.y + 10))
        
        # Draw text
        text_surface = system_font.render(self.text, True, WHITE)
        surface.blit(text_surface, (self.x + self.width//2 - text_surface.get_width()//2, self.y + 40))

class StartMenu:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.width = 200
        self.height = 350
        self.options = [
            {"icon": "📺", "text": "Terminal", "action": "terminal"},
            {"icon": "📁", "text": "File Manager", "action": "files"},
            {"icon": "🌐", "text": "Web Browser", "action": "browser"},
            {"icon": "⚙️", "text": "Settings", "action": "settings"},
            {"icon": "🔍", "text": "Search", "action": "search"},
            {"icon": "📋", "text": "Clipboard", "action": "clipboard"},
            {"icon": "📝", "text": "Text Editor", "action": "editor"},
            {"icon": "🖼️", "text": "Image Viewer", "action": "viewer"},
            {"icon": "⏻", "text": "Power Options", "action": "power"}
        ]
    
    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if (self.x <= event.pos[0] <= self.x + self.width and
                self.y <= event.pos[1] <= self.y + self.height):
                # Check which option was clicked
                option_height = 35
                for i, option in enumerate(self.options):
                    option_y = self.y + 30 + i * option_height
                    if option_y <= event.pos[1] <= option_y + option_height:
                        return option["action"]
            else:
                # Clicked outside menu, close it
                return "close"
        return None
    
    def draw(self, surface):
        # Draw menu background
        background_color = app_state.get_theme_color("taskbar")
        pygame.draw.rect(surface, background_color, (self.x, self.y, self.width, self.height))
        pygame.draw.rect(surface, DARK_GRAY, (self.x, self.y, self.width, self.height), 1)
        
        # Draw menu header
        pygame.draw.rect(surface, BLUE, (self.x, self.y, self.width, 30))
        header_text = menu_font.render("AMOS Desktop OS", True, WHITE)
        surface.blit(header_text, (self.x + 10, self.y + 5))
        
        # Draw options
        option_height = 35
        for i, option in enumerate(self.options):
            option_y = self.y + 30 + i * option_height
            
            # Highlight on hover
            mouse_pos = pygame.mouse.get_pos()
            if (self.x <= mouse_pos[0] <= self.x + self.width and
                option_y <= mouse_pos[1] <= option_y + option_height):
                highlight_color = app_state.get_theme_color("button")
                pygame.draw.rect(surface, highlight_color, (self.x, option_y, self.width, option_height))
            
            # Draw icon and text
            icon_text = menu_font.render(option["icon"], True, WHITE)
            surface.blit(icon_text, (self.x + 10, option_y + 8))
            
            option_text = menu_font.render(option["text"], True, WHITE)
            surface.blit(option_text, (self.x + 40, option_y + 8))

class ContextMenu:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.width = 150
        self.height = 160
        self.options = [
            {"text": "New Folder", "action": "new_folder"},
            {"text": "New File", "action": "new_file"},
            {"text": "-----", "action": None},
            {"text": "View", "action": "view"},
            {"text": "Sort By", "action": "sort"},
            {"text": "-----", "action": None},
            {"text": "Desktop Settings", "action": "settings"},
            {"text": "Refresh", "action": "refresh"}
        ]
    
    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:
                if (self.x <= event.pos[0] <= self.x + self.width and
                    self.y <= event.pos[1] <= self.y + self.height):
                    # Check which option was clicked
                    for i, option in enumerate(self.options):
                        if self.y + i * 20 <= event.pos[1] <= self.y + (i + 1) * 20 and option["action"]:
                            return option["action"]
                else:
                    # Clicked outside menu, close it
                    return "close"
        return None
    
    def draw(self, surface):
        # Draw menu background
        pygame.draw.rect(surface, WHITE, (self.x, self.y, self.width, self.height))
        pygame.draw.rect(surface, DARK_GRAY, (self.x, self.y, self.width, self.height), 1)
        
        # Draw options
        for i, option in enumerate(self.options):
            if option["text"] == "-----":
                # Draw separator
                pygame.draw.line(surface, DARK_GRAY, 
                                (self.x, self.y + (i * 20) + 10),
                                (self.x + self.width, self.y + (i * 20) + 10), 1)
            else:
                # Check for mouse hover
                mouse_pos = pygame.mouse.get_pos()
                if (self.x <= mouse_pos[0] <= self.x + self.width and
                    self.y + i * 20 <= mouse_pos[1] <= self.y + (i + 1) * 20):
                    pygame.draw.rect(surface, LIGHT_BLUE, (self.x, self.y + i * 20, self.width, 20))
                    option_text = system_font.render(option["text"], True, WHITE)
                else:
                    option_text = system_font.render(option["text"], True, BLACK)
                surface.blit(option_text, (self.x + 10, self.y + (i * 20) + 5))

def simulate_boot_process(screen):
    """Simulate OS boot process with loading bars"""
    screen.fill(BLACK)
    
    # Draw AMOS logo
    logo_size = 200
    logo_x = WIDTH // 2 - logo_size // 2
    logo_y = HEIGHT // 3 - logo_size // 2
    
    # Draw logo background
    pygame.draw.rect(screen, DARK_BLUE, (logo_x, logo_y, logo_size, logo_size), 0, 20)
    pygame.draw.rect(screen, BLUE, (logo_x + 10, logo_y + 10, logo_size - 20, logo_size - 20), 0, 15)
    
    # Draw logo text
    logo_text = pygame.font.SysFont("Arial", 48).render("AMOS", True, WHITE)
    screen.blit(logo_text, (logo_x + (logo_size - logo_text.get_width()) // 2, 
                          logo_y + (logo_size - logo_text.get_height()) // 2 - 10))
    
    logo_subtext = pygame.font.SysFont("Arial", 16).render("Desktop OS", True, WHITE)
    screen.blit(logo_subtext, (logo_x + (logo_size - logo_subtext.get_width()) // 2, 
                             logo_y + (logo_size + logo_text.get_height()) // 2 + 5))
    
    pygame.display.flip()
    pygame.time.wait(1000)
    
    loading_steps = [
        "BIOS Initialization...",
        "Bootloader Starting...",
        "Loading Kernel...",
        "Initializing Hardware...",
        "Starting System Services...",
        "Loading Desktop Environment..."
    ]
    
    y_pos = HEIGHT // 2 + 100
    for step in loading_steps:
        # Keep the logo visible
        screen.fill(BLACK)
        # Redraw logo
        pygame.draw.rect(screen, DARK_BLUE, (logo_x, logo_y, logo_size, logo_size), 0, 20)
        pygame.draw.rect(screen, BLUE, (logo_x + 10, logo_y + 10, logo_size - 20, logo_size - 20), 0, 15)
        screen.blit(logo_text, (logo_x + (logo_size - logo_text.get_width()) // 2, 
                              logo_y + (logo_size - logo_text.get_height()) // 2 - 10))
        screen.blit(logo_subtext, (logo_x + (logo_size - logo_subtext.get_width()) // 2, 
                                 logo_y + (logo_size + logo_text.get_height()) // 2 + 5))
        
        # Draw step text
        step_text = title_font.render(step, True, WHITE)
        screen.blit(step_text, (WIDTH // 2 - step_text.get_width() // 2, y_pos))
        
        # Draw progress bar outline
        bar_width = 400
        bar_height = 20
        bar_x = WIDTH // 2 - bar_width // 2
        bar_y = y_pos + 30
        pygame.draw.rect(screen, WHITE, (bar_x, bar_y, bar_width, bar_height), 1)
        
        # Fill progress bar
        for i in range(101):
            fill_width = int(bar_width * i / 100)
            pygame.draw.rect(screen, BLUE, (bar_x, bar_y, fill_width, bar_height))
            
            # Draw percentage text
            percent_text = system_font.render(f"{i}%", True, WHITE)
            screen.blit(percent_text, (bar_x + bar_width + 10, bar_y))
            
            pygame.display.flip()
            pygame.time.wait(5)  # Adjust for faster/slower loading
        
        pygame.time.wait(200)
        
    # Transition to desktop
    screen.fill(BLACK)
    pygame.display.flip()
    pygame.time.wait(300)

def create_terminal_window():
    terminal = Window(50, 50, 500, 350, "Terminal", "terminal")
    terminal.terminal_history = [
        "AMOS Desktop OS Terminal v1.0",
        "Type 'help' for available commands",
        "",
        "$ "
    ]
    terminal.terminal_input = ""
    return terminal

def create_file_manager_window():
    return Window(600, 100, 380, 300, "File Manager", "file_manager")

def create_browser_window():
    browser = Window(300, 150, 600, 400, "Web Browser", "browser")
    browser.browser_url = "https://amos-os.org"
    return browser

def create_settings_window():
    return Window(200, 100, 600, 450, "System Settings", "settings")

def main():
    # Simulate OS boot
    simulate_boot_process(screen)
    
    # Set up desktop components
    taskbar = TaskBar(WIDTH)
    
    # Create desktop icons
    icons = [
        DesktopIcon(50, 100, "terminal"),
        DesktopIcon(50, 180, "files"),
        DesktopIcon(50, 260, "browser"),
        DesktopIcon(50, 340, "settings")
    ]
    
    # Create initial windows
    terminal_window = create_terminal_window()
    file_manager_window = create_file_manager_window()
    
    # Window management
    app_state.windows = [terminal_window, file_manager_window]
    app_state.active_window = terminal_window
    
    # Create context menu (initially hidden)
    context_menu = None
    start_menu = None
    
    # Main loop
    running = True
    clock = pygame.time.Clock()
    
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            
            # Handle taskbar events first
            taskbar_action = taskbar.handle_event(event)
            if taskbar_action:
                if isinstance(taskbar_action, list) and taskbar_action[0] == "restore":
                    # Restore minimized window
                    idx = taskbar_action[1]
                    if idx < len(taskbar.minimized_windows):
                        window = taskbar.minimized_windows.pop(idx)
                        window.minimized = False
                        app_state.windows.append(window)
                        app_state.active_window = window
                        window.active = True
                        
                elif taskbar_action == "menu":
                    if not app_state.start_menu_open:
                        start_menu = StartMenu(10, 30)
                        app_state.start_menu_open = True
                    else:
                        start_menu = None
                        app_state.start_menu_open = False
                        
                elif taskbar_action == "terminal":
                    # Check if terminal is already open
                    terminal_exists = False
                    for window in app_state.windows:
                        if window.content_type == "terminal" and not window.minimized:
                            terminal_exists = True
                            app_state.active_window = window
                            window.active = True
                            break
                    
                    if not terminal_exists:
                        new_terminal = create_terminal_window()
                        app_state.windows.append(new_terminal)
                        app_state.active_window = new_terminal
                        
                elif taskbar_action == "files":
                    # Check if file manager is already open
                    files_exists = False
                    for window in app_state.windows:
                        if window.content_type == "file_manager" and not window.minimized:
                            files_exists = True
                            app_state.active_window = window
                            window.active = True
                            break
                    
                    if not files_exists:
                        new_files = create_file_manager_window()
                        app_state.windows.append(new_files)
                        app_state.active_window = new_files
                        
                elif taskbar_action == "browser":
                    # Check if browser is already open
                    browser_exists = False
                    for window in app_state.windows:
                        if window.content_type == "browser" and not window.minimized:
                            browser_exists = True
                            app_state.active_window = window
                            window.active = True
                            break
                    
                    if not browser_exists:
                        new_browser = create_browser_window()
                        app_state.windows.append(new_browser)
                        app_state.active_window = new_browser
                        
                elif taskbar_action == "settings":
                    if not app_state.settings_open:
                        settings_window = create_settings_window()
                        app_state.windows.append(settings_window)
                        app_state.active_window = settings_window
                        app_state.settings_open = True
            
            # Handle start menu if it's open
            if app_state.start_menu_open and start_menu:
                start_menu_action = start_menu.handle_event(event)
                if start_menu_action:
                    if start_menu_action == "close":
                        start_menu = None
                        app_state.start_menu_open = False
                    elif start_menu_action == "terminal":
                        new_terminal = create_terminal_window()
                        app_state.windows.append(new_terminal)
                        app_state.active_window = new_terminal
                        start_menu = None
                        app_state.start_menu_open = False
                    elif start_menu_action == "files":
                        new_files = create_file_manager_window()
                        app_state.windows.append(new_files)
                        app_state.active_window = new_files
                        start_menu = None
                        app_state.start_menu_open = False
                    elif start_menu_action == "browser":
                        new_browser = create_browser_window()
                        app_state.windows.append(new_browser)
                        app_state.active_window = new_browser
                        start_menu = None
                        app_state.start_menu_open = False
                    elif start_menu_action == "settings":
                        new_settings = create_settings_window()
                        app_state.windows.append(new_settings)
                        app_state.active_window = new_settings
                        app_state.settings_open = True
                        start_menu = None
                        app_state.start_menu_open = False
            
            # Handle desktop icons
            for icon in icons:
                icon_action = icon.handle_event(event)
                if icon_action:
                    if icon_action == "terminal":
                        new_terminal = create_terminal_window()
                        app_state.windows.append(new_terminal)
                        app_state.active_window = new_terminal
                    elif icon_action == "files":
                        new_files = create_file_manager_window()
                        app_state.windows.append(new_files)
                        app_state.active_window = new_files
                    elif icon_action == "browser":
                        new_browser = create_browser_window()
                        app_state.windows.append(new_browser)
                        app_state.active_window = new_browser
                    elif icon_action == "settings":
                        if not app_state.settings_open:
                            new_settings = create_settings_window()
                            app_state.windows.append(new_settings)
                            app_state.active_window = new_settings
                            app_state.settings_open = True
            
            # Handle context menu if it's open
            if context_menu:
                menu_action = context_menu.handle_event(event)
                if menu_action:
                    if menu_action == "close":
                        context_menu = None
                    elif menu_action == "settings":
                        if not app_state.settings_open:
                            new_settings = create_settings_window()
                            app_state.windows.append(new_settings)
                            app_state.active_window = new_settings
                            app_state.settings_open = True
                        context_menu = None
            
            # Handle right-click for context menu
            if event.type == pygame.MOUSEBUTTONDOWN and event.button == 3:
                # Right-click on desktop (not on any window)
                if event.pos[1] > 30 and all(not (window.x <= event.pos[0] <= window.x + window.width and
                          window.y <= event.pos[1] <= window.y + window.height)
                      for window in app_state.windows if not window.minimized):
                    context_menu = ContextMenu(event.pos[0], event.pos[1])
            
            # Handle window events
            for i, window in enumerate(app_state.windows):
                if window.active:
                    window_action = window.handle_event(event)
                    if window_action:
                        if window_action == "close":
                            # Special handling for settings window
                            if window.content_type == "settings":
                                app_state.settings_open = False
                            # Remove the window
                            app_state.windows.pop(i)
                            # Set next active window if there are any
                            if app_state.windows:
                                app_state.active_window = app_state.windows[-1]
                                app_state.active_window.active = True
                            break
                        elif window_action == "minimize":
                            window.minimized = True
                            taskbar.minimized_windows.append(window)
                            app_state.windows.pop(i)
                            if app_state.windows:
                                app_state.active_window = app_state.windows[-1]
                                app_state.active_window.active = True
                            break
                else:
                    # Simplified event handling for inactive windows
                    if (event.type == pygame.MOUSEBUTTONDOWN and
                        window.x <= event.pos[0] <= window.x + window.width and
                        window.y <= event.pos[1] <= window.y + window.height):
                        # Make this window active
                        for w in app_state.windows:
                            w.active = False
                        window.active = True
                        app_state.active_window = window
                        
                        # Bring window to front (move to end of list)
                        app_state.windows.pop(i)
                        app_state.windows.append(window)
                        break
        
        # Draw desktop background
        desktop_color = app_state.get_theme_color("desktop")
        screen.fill(desktop_color)
        
        # Draw desktop icons
        for icon in icons:
            icon.draw(screen)
        
        # Draw windows (in order, so later windows are on top)
        for window in app_state.windows:
            window.draw(screen)
        
        # Draw context menu (if open)
        if context_menu:
            context_menu.draw(screen)
        
        # Draw start menu (if open)
        if app_state.start_menu_open and start_menu:
            start_menu.draw(screen)
        
        # Draw taskbar (always on top)
        taskbar.draw(screen)
        
        # Update screen
        pygame.display.flip()
        
        # Cap at 30 FPS
        clock.tick(30)
    
    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    print("Starting Enhanced AMOS Desktop OS visualization...")
    print("This will display in the VNC window.")
    main()