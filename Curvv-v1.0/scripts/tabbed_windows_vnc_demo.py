"""
AMOS Desktop OS - Tabbed Windows VNC Demo

This script demonstrates the Fluxbox-inspired tabbed window functionality
of the AMOS Desktop OS in a VNC viewer.
"""

import pygame
import sys
import math
import time
import random
from collections import defaultdict

# Initialize pygame
pygame.init()

# Screen dimensions
SCREEN_WIDTH = 1024
SCREEN_HEIGHT = 768

# Colors
DESKTOP_COLOR = (45, 52, 54)
TASKBAR_COLOR = (45, 52, 54)
WINDOW_COLOR = (223, 230, 233)
TITLEBAR_COLOR = (9, 132, 227)
TITLEBAR_ACTIVE_COLOR = (45, 152, 247)
CLOSE_BTN_COLOR = (255, 71, 87)
MAX_BTN_COLOR = (253, 203, 110)
MIN_BTN_COLOR = (0, 184, 148)
TEXT_COLOR = (255, 255, 255)
TAB_ACTIVE_COLOR = (45, 152, 247)
TAB_INACTIVE_COLOR = (9, 132, 227)
TAB_TEXT_COLOR = (255, 255, 255)
TERMINAL_BG_COLOR = (30, 39, 46)
TERMINAL_TEXT_COLOR = (0, 255, 0)
CONTEXTMENU_BG_COLOR = (45, 52, 54)
CONTEXTMENU_FG_COLOR = (255, 255, 255)
CONTEXTMENU_HOVER_COLOR = (9, 132, 227)

# UI constants
TITLEBAR_HEIGHT = 30
TAB_HEIGHT = 25
BUTTON_SIZE = 15
BUTTON_MARGIN = 8
TASKBAR_HEIGHT = 40
ICON_SIZE = 32
DESKTOP_ICON_SIZE = 64
DESKTOP_ICON_SPACING = 80
CONTEXT_MENU_WIDTH = 200
CONTEXT_MENU_ITEM_HEIGHT = 30

# Initialize screen
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("AMOS Desktop OS - Tabbed Windows Demo")

# Clock for FPS control
clock = pygame.time.Clock()

# Load fonts
try:
    font = pygame.font.Font(None, 24)
    small_font = pygame.font.Font(None, 20)
    large_font = pygame.font.Font(None, 28)
except:
    print("Warning: Could not load fonts. Using system default.")
    font = pygame.font.SysFont(None, 24)
    small_font = pygame.font.SysFont(None, 20)
    large_font = pygame.font.SysFont(None, 28)

# Application state
class AppState:
    def __init__(self):
        self.windows = []
        self.active_window = None
        self.drag_window = None
        self.drag_offset = (0, 0)
        self.resize_window = None
        self.next_window_id = 0
        self.taskbar_buttons = []
        self.desktop_icons = []
        self.context_menu = None
        self.dark_mode = False
        self.theme = "blue"
        
        # Initialize desktop icons
        self.create_desktop_icons()
    
    def create_desktop_icons(self):
        icons = [
            {"name": "Terminal", "x": 40, "y": 40, "action": lambda: self.create_terminal_window()},
            {"name": "Files", "x": 40, "y": 120, "action": lambda: self.create_file_manager_window()},
            {"name": "Browser", "x": 40, "y": 200, "action": lambda: self.create_browser_window()},
            {"name": "Settings", "x": 40, "y": 280, "action": lambda: self.create_settings_window()},
        ]
        
        for icon in icons:
            self.desktop_icons.append(DesktopIcon(icon["x"], icon["y"], icon["name"], icon["action"]))
    
    def get_theme_color(self, element):
        if element == "titlebar":
            return (9, 132, 227) if not self.dark_mode else (24, 44, 97)
        elif element == "titlebar_active":
            return (45, 152, 247) if not self.dark_mode else (64, 115, 158)
        elif element == "window":
            return (223, 230, 233) if not self.dark_mode else (45, 52, 54)
        elif element == "taskbar":
            return (45, 52, 54) if not self.dark_mode else (30, 39, 46)
        elif element == "desktop":
            return (45, 52, 54) if not self.dark_mode else (30, 39, 46)
        return (255, 255, 255)
    
    def set_theme(self, theme_name):
        self.theme = theme_name
        if theme_name == "dark":
            self.dark_mode = True
        else:
            self.dark_mode = False

# Window class
class Window:
    def __init__(self, x, y, width, height, title, content_type="default"):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.title = title
        self.content_type = content_type
        self.visible = True
        self.minimized = False
        self.maximized = False
        self.saved_pos = (x, y)
        self.saved_size = (width, height)
        self.id = 0
        self.active = False
        self.terminal_content = []
        self.terminal_cursor_pos = 0
        self.terminal_input = ""
        self.browser_url = "https://amos-os.org"
        
        # Tab-related properties
        self.parent_window = None  # Parent window if this is a tab
        self.tabs = []  # List of child tab windows
        self.active_tab_index = 0  # Index of the active tab
        self.is_tab = False  # Whether this window is a tab
        
        if content_type == "terminal":
            self.terminal_content.append("AMOS Terminal v1.0")
            self.terminal_content.append("Type 'help' for available commands.")
            self.terminal_content.append("$ ")
        
        elif content_type == "file_manager":
            # Initialize with some dummy files
            self.files = [
                {"name": "Documents", "type": "folder", "size": "4.2 KB"},
                {"name": "Pictures", "type": "folder", "size": "8.7 MB"},
                {"name": "Downloads", "type": "folder", "size": "1.2 GB"},
                {"name": "readme.txt", "type": "file", "size": "2.5 KB"},
                {"name": "config.ini", "type": "file", "size": "1.8 KB"},
                {"name": "screenshot.png", "type": "file", "size": "345 KB"},
            ]
            self.selected_file = None
        
        elif content_type == "browser":
            self.browser_content = """
            <html>
                <head>
                    <title>AMOS OS</title>
                </head>
                <body>
                    <h1>Welcome to AMOS Desktop OS</h1>
                    <p>A graphical desktop environment for minimal OS</p>
                    <ul>
                        <li>Fluxbox-inspired window management</li>
                        <li>Tabbed windows</li>
                        <li>Native 3D rendering</li>
                    </ul>
                </body>
            </html>
            """
        
        elif content_type == "settings":
            self.settings_sections = ["Appearance", "Desktop", "System", "About"]
            self.active_section = 0
            self.themes = ["blue", "dark", "light"]
            self.current_theme = 0
    
    def add_tab(self, window):
        """Add a new tab to this window"""
        if self.is_tab:
            # Can't add tabs to a tab
            return False
        
        # Make the window a tab
        window.is_tab = True
        window.parent_window = self
        window.visible = False  # Hide the tab window, content shown in parent
        
        # Add to tab list
        self.tabs.append(window)
        
        # Make the new tab active
        self.active_tab_index = len(self.tabs) - 1
        
        # Adjust parent window size if needed
        if not self.tabs:  # If this is the first tab
            self.height += TAB_HEIGHT
        
        return True
    
    def remove_tab(self, tab_index):
        """Remove a tab from this window"""
        if tab_index < 0 or tab_index >= len(self.tabs):
            return False
        
        # Get the tab window
        tab = self.tabs[tab_index]
        
        # Restore tab to a normal window
        tab.is_tab = False
        tab.parent_window = None
        tab.visible = True
        
        # Remove from tab list
        self.tabs.pop(tab_index)
        
        # Update active tab index
        if self.active_tab_index >= len(self.tabs):
            self.active_tab_index = max(0, len(self.tabs) - 1)
        
        # Adjust parent window size if no tabs left
        if not self.tabs:
            self.height -= TAB_HEIGHT
        
        return True
    
    def switch_tab(self, tab_index):
        """Switch to a different tab"""
        if tab_index < 0 or tab_index >= len(self.tabs):
            return False
        
        self.active_tab_index = tab_index
        return True
    
    def get_active_tab(self):
        """Get the currently active tab window"""
        if not self.tabs:
            return None
        return self.tabs[self.active_tab_index]
    
    def process_terminal_command(self, command):
        """Process a command in the terminal"""
        self.terminal_content.append("$ " + command)
        
        if command.strip() == "":
            self.terminal_content.append("$ ")
            return
        
        cmd_parts = command.strip().split()
        cmd = cmd_parts[0].lower()
        
        if cmd == "help":
            self.terminal_content.append("Available commands:")
            self.terminal_content.append("  help     - Display this help message")
            self.terminal_content.append("  clear    - Clear the terminal screen")
            self.terminal_content.append("  echo     - Display text")
            self.terminal_content.append("  ls       - List files")
            self.terminal_content.append("  version  - Display AMOS version")
            self.terminal_content.append("  exit     - Close the terminal")
        
        elif cmd == "clear":
            self.terminal_content = ["$ "]
        
        elif cmd == "echo":
            echo_text = " ".join(cmd_parts[1:]) if len(cmd_parts) > 1 else ""
            self.terminal_content.append(echo_text)
        
        elif cmd == "ls":
            self.terminal_content.append("Documents/  Pictures/  Downloads/  readme.txt  config.ini  screenshot.png")
        
        elif cmd == "version":
            self.terminal_content.append("AMOS Desktop OS v1.0")
            self.terminal_content.append("Copyright (c) 2025 AMOS Team")
        
        elif cmd == "exit":
            # This would normally close the window, but here we'll just add a message
            self.terminal_content.append("Terminal closed.")
            return
        
        else:
            self.terminal_content.append(f"Command not found: {cmd}")
        
        self.terminal_content.append("$ ")
    
    def handle_event(self, event, app_state):
        """Handle events for the window"""
        if event.type == pygame.MOUSEBUTTONDOWN:
            # Get mouse position
            mx, my = event.pos
            
            # Check if clicking in the window
            if (self.x <= mx <= self.x + self.width and
                self.y <= my <= self.y + self.height):
                
                # Check if clicking on the title bar
                if self.y <= my <= self.y + TITLEBAR_HEIGHT:
                    # Check if clicking on close button
                    close_btn_x = self.x + self.width - BUTTON_MARGIN - BUTTON_SIZE
                    close_btn_y = self.y + BUTTON_MARGIN
                    if (close_btn_x <= mx <= close_btn_x + BUTTON_SIZE and
                        close_btn_y <= my <= close_btn_y + BUTTON_SIZE):
                        self.visible = False
                        return True
                    
                    # Check if clicking on maximize button
                    max_btn_x = close_btn_x - BUTTON_MARGIN - BUTTON_SIZE
                    max_btn_y = close_btn_y
                    if (max_btn_x <= mx <= max_btn_x + BUTTON_SIZE and
                        max_btn_y <= my <= max_btn_y + BUTTON_SIZE):
                        if self.maximized:
                            self.x, self.y = self.saved_pos
                            self.width, self.height = self.saved_size
                            self.maximized = False
                        else:
                            self.saved_pos = (self.x, self.y)
                            self.saved_size = (self.width, self.height)
                            self.x, self.y = 0, 0
                            self.width, self.height = SCREEN_WIDTH, SCREEN_HEIGHT - TASKBAR_HEIGHT
                            self.maximized = True
                        return True
                    
                    # Check if clicking on minimize button
                    min_btn_x = max_btn_x - BUTTON_MARGIN - BUTTON_SIZE
                    min_btn_y = max_btn_y
                    if (min_btn_x <= mx <= min_btn_x + BUTTON_SIZE and
                        min_btn_y <= my <= min_btn_y + BUTTON_SIZE):
                        self.minimized = True
                        return True
                    
                    # Start dragging the window
                    app_state.drag_window = self
                    app_state.drag_offset = (mx - self.x, my - self.y)
                    return True
                
                # Check if clicking on tab area
                if self.tabs and self.y + TITLEBAR_HEIGHT <= my <= self.y + TITLEBAR_HEIGHT + TAB_HEIGHT:
                    # Calculate tab width
                    tab_width = min(120, self.width / max(1, len(self.tabs)))
                    
                    # Check which tab was clicked
                    for i in range(len(self.tabs)):
                        tab_x = self.x + i * tab_width
                        if tab_x <= mx <= tab_x + tab_width:
                            self.active_tab_index = i
                            return True
                
                # Handle terminal input
                if self.content_type == "terminal" and not self.is_tab:
                    if event.button == 1:  # Left click
                        # Place cursor, simplified
                        self.terminal_cursor_pos = len(self.terminal_input)
                
                # Handle file manager selection
                elif self.content_type == "file_manager" and not self.is_tab:
                    if event.button == 1:  # Left click
                        # Calculate file list area
                        file_area_y = self.y + TITLEBAR_HEIGHT
                        if self.tabs:
                            file_area_y += TAB_HEIGHT
                        
                        if my > file_area_y:
                            # Calculate which file was clicked
                            item_height = 30
                            file_index = (my - file_area_y) // item_height
                            if 0 <= file_index < len(self.files):
                                self.selected_file = file_index
                                # Double-click to open folders
                                if event.button == 1 and pygame.time.get_ticks() % 5 == 0:
                                    if self.files[file_index]["type"] == "folder":
                                        # Simulate navigating into folder
                                        pass
                
                # Handle settings selection
                elif self.content_type == "settings" and not self.is_tab:
                    if event.button == 1:  # Left click
                        # Calculate settings area
                        settings_y = self.y + TITLEBAR_HEIGHT
                        if self.tabs:
                            settings_y += TAB_HEIGHT
                        
                        # Check if clicking on sections
                        section_width = self.width // len(self.settings_sections)
                        section_index = (mx - self.x) // section_width
                        if 0 <= section_index < len(self.settings_sections):
                            self.active_section = section_index
                        
                        # Check if clicking on theme options (when in Appearance section)
                        if self.active_section == 0:  # Appearance
                            theme_y = settings_y + 80
                            theme_height = 40
                            for i, theme in enumerate(self.themes):
                                theme_rect = pygame.Rect(self.x + 100, theme_y + i * theme_height, 100, 30)
                                if theme_rect.collidepoint(mx, my):
                                    self.current_theme = i
                                    app_state.set_theme(self.themes[i])
                
                # Check if clicking on resize corner
                resize_corner_size = 20
                if (self.x + self.width - resize_corner_size <= mx <= self.x + self.width and
                    self.y + self.height - resize_corner_size <= my <= self.y + self.height):
                    app_state.resize_window = self
                    return True
                
                return True
        
        elif event.type == pygame.MOUSEBUTTONUP:
            # Stop dragging or resizing
            if app_state.drag_window == self:
                app_state.drag_window = None
            if app_state.resize_window == self:
                app_state.resize_window = None
        
        elif event.type == pygame.KEYDOWN:
            # Handle keyboard input for the terminal
            if self.content_type == "terminal" and self.active and not self.is_tab:
                if event.key == pygame.K_RETURN:
                    self.process_terminal_command(self.terminal_input)
                    self.terminal_input = ""
                    self.terminal_cursor_pos = 0
                elif event.key == pygame.K_BACKSPACE:
                    if self.terminal_cursor_pos > 0:
                        self.terminal_input = (self.terminal_input[:self.terminal_cursor_pos-1] + 
                                               self.terminal_input[self.terminal_cursor_pos:])
                        self.terminal_cursor_pos -= 1
                elif event.key == pygame.K_DELETE:
                    if self.terminal_cursor_pos < len(self.terminal_input):
                        self.terminal_input = (self.terminal_input[:self.terminal_cursor_pos] + 
                                              self.terminal_input[self.terminal_cursor_pos+1:])
                elif event.key == pygame.K_LEFT:
                    self.terminal_cursor_pos = max(0, self.terminal_cursor_pos - 1)
                elif event.key == pygame.K_RIGHT:
                    self.terminal_cursor_pos = min(len(self.terminal_input), self.terminal_cursor_pos + 1)
                elif event.key == pygame.K_HOME:
                    self.terminal_cursor_pos = 0
                elif event.key == pygame.K_END:
                    self.terminal_cursor_pos = len(self.terminal_input)
                elif event.unicode and 32 <= ord(event.unicode) <= 126:
                    self.terminal_input = (self.terminal_input[:self.terminal_cursor_pos] + 
                                         event.unicode + 
                                         self.terminal_input[self.terminal_cursor_pos:])
                    self.terminal_cursor_pos += 1
        
        return False
    
    def draw(self, surface, app_state):
        """Draw the window"""
        if not self.visible or self.minimized:
            return
        
        # Get theme colors
        titlebar_color = (TITLEBAR_ACTIVE_COLOR if self.active else TITLEBAR_COLOR)
        if app_state.dark_mode:
            titlebar_color = (64, 115, 158) if self.active else (24, 44, 97)
            window_color = (45, 52, 54)
        else:
            window_color = WINDOW_COLOR
        
        # Draw window background
        pygame.draw.rect(surface, window_color, 
                         (self.x, self.y, self.width, self.height))
        
        # Draw title bar
        pygame.draw.rect(surface, titlebar_color, 
                         (self.x, self.y, self.width, TITLEBAR_HEIGHT))
        
        # Draw window title
        title_text = font.render(self.title, True, TEXT_COLOR)
        surface.blit(title_text, (self.x + 10, self.y + (TITLEBAR_HEIGHT - title_text.get_height()) // 2))
        
        # Draw window buttons (close, maximize, minimize)
        # Close button
        close_btn_x = self.x + self.width - BUTTON_MARGIN - BUTTON_SIZE
        close_btn_y = self.y + BUTTON_MARGIN
        pygame.draw.rect(surface, CLOSE_BTN_COLOR, 
                         (close_btn_x, close_btn_y, BUTTON_SIZE, BUTTON_SIZE))
        
        # Maximize button
        max_btn_x = close_btn_x - BUTTON_MARGIN - BUTTON_SIZE
        max_btn_y = close_btn_y
        pygame.draw.rect(surface, MAX_BTN_COLOR, 
                         (max_btn_x, max_btn_y, BUTTON_SIZE, BUTTON_SIZE))
        
        # Minimize button
        min_btn_x = max_btn_x - BUTTON_MARGIN - BUTTON_SIZE
        min_btn_y = max_btn_y
        pygame.draw.rect(surface, MIN_BTN_COLOR, 
                         (min_btn_x, min_btn_y, BUTTON_SIZE, BUTTON_SIZE))
        
        # Draw tabs if any
        if self.tabs:
            tab_area_y = self.y + TITLEBAR_HEIGHT
            tab_width = min(120, self.width / len(self.tabs))
            
            # Draw tab background
            pygame.draw.rect(surface, titlebar_color, 
                             (self.x, tab_area_y, self.width, TAB_HEIGHT))
            
            # Draw individual tabs
            for i, tab in enumerate(self.tabs):
                tab_x = self.x + i * tab_width
                tab_color = TAB_ACTIVE_COLOR if i == self.active_tab_index else TAB_INACTIVE_COLOR
                if app_state.dark_mode:
                    tab_color = (64, 115, 158) if i == self.active_tab_index else (24, 44, 97)
                
                pygame.draw.rect(surface, tab_color, 
                                 (tab_x, tab_area_y, tab_width, TAB_HEIGHT))
                
                # Draw tab title
                tab_title = small_font.render(tab.title, True, TAB_TEXT_COLOR)
                surface.blit(tab_title, 
                             (tab_x + 5, tab_area_y + (TAB_HEIGHT - tab_title.get_height()) // 2))
        
        # Calculate content area
        content_y = self.y + TITLEBAR_HEIGHT
        if self.tabs:
            content_y += TAB_HEIGHT
        content_height = self.height - (content_y - self.y)
        
        # Draw content based on type
        if self.content_type == "terminal" and not self.is_tab:
            self.draw_terminal(surface, content_y, content_height)
        elif self.content_type == "file_manager" and not self.is_tab:
            self.draw_file_manager(surface, content_y, content_height)
        elif self.content_type == "browser" and not self.is_tab:
            self.draw_browser(surface, content_y, content_height)
        elif self.content_type == "settings" and not self.is_tab:
            self.draw_settings(surface, content_y, content_height, app_state)
        elif self.tabs:
            # Draw active tab content
            active_tab = self.get_active_tab()
            if active_tab:
                if active_tab.content_type == "terminal":
                    active_tab.draw_terminal(surface, content_y, content_height)
                elif active_tab.content_type == "file_manager":
                    active_tab.draw_file_manager(surface, content_y, content_height)
                elif active_tab.content_type == "browser":
                    active_tab.draw_browser(surface, content_y, content_height)
                elif active_tab.content_type == "settings":
                    active_tab.draw_settings(surface, content_y, content_height, app_state)
        else:
            # Default window content
            self.draw_default(surface, content_y, content_height)
    
    def draw_terminal(self, surface, content_y, content_height):
        """Draw terminal content"""
        # Draw terminal background
        pygame.draw.rect(surface, TERMINAL_BG_COLOR, 
                        (self.x, content_y, self.width, content_height))
        
        # Draw terminal content
        line_height = small_font.get_height()
        max_lines = content_height // line_height
        
        # Show the last max_lines of terminal content
        visible_lines = self.terminal_content[-max_lines:] if len(self.terminal_content) > max_lines else self.terminal_content
        
        for i, line in enumerate(visible_lines):
            if i == len(visible_lines) - 1 and line.endswith("$ "):
                # Draw the current input line
                prompt = line
                input_text = small_font.render(prompt + self.terminal_input, True, TERMINAL_TEXT_COLOR)
                surface.blit(input_text, (self.x + 5, content_y + i * line_height))
                
                # Draw cursor
                cursor_x = self.x + 5 + small_font.size(prompt + self.terminal_input[:self.terminal_cursor_pos])[0]
                if pygame.time.get_ticks() % 1000 < 500:  # Blinking cursor
                    pygame.draw.line(surface, TERMINAL_TEXT_COLOR, 
                                    (cursor_x, content_y + i * line_height), 
                                    (cursor_x, content_y + (i+1) * line_height - 2))
            else:
                line_text = small_font.render(line, True, TERMINAL_TEXT_COLOR)
                surface.blit(line_text, (self.x + 5, content_y + i * line_height))
    
    def draw_file_manager(self, surface, content_y, content_height):
        """Draw file manager content"""
        # Draw file manager background
        pygame.draw.rect(surface, WINDOW_COLOR, 
                        (self.x, content_y, self.width, content_height))
        
        # Draw toolbar
        toolbar_height = 30
        pygame.draw.rect(surface, TITLEBAR_COLOR, 
                        (self.x, content_y, self.width, toolbar_height))
        
        # Draw navigation buttons
        nav_btn_width = 60
        back_btn = pygame.Rect(self.x + 5, content_y + 5, nav_btn_width, toolbar_height - 10)
        pygame.draw.rect(surface, TASKBAR_COLOR, back_btn)
        back_text = small_font.render("Back", True, TEXT_COLOR)
        surface.blit(back_text, (back_btn.x + (nav_btn_width - back_text.get_width()) // 2, 
                                back_btn.y + (back_btn.height - back_text.get_height()) // 2))
        
        forward_btn = pygame.Rect(self.x + 10 + nav_btn_width, content_y + 5, nav_btn_width, toolbar_height - 10)
        pygame.draw.rect(surface, TASKBAR_COLOR, forward_btn)
        forward_text = small_font.render("Forward", True, TEXT_COLOR)
        surface.blit(forward_text, (forward_btn.x + (nav_btn_width - forward_text.get_width()) // 2, 
                                   forward_btn.y + (forward_btn.height - forward_text.get_height()) // 2))
        
        # Draw address bar
        address_bar = pygame.Rect(self.x + 15 + nav_btn_width * 2, content_y + 5, 
                                 self.width - 20 - nav_btn_width * 2, toolbar_height - 10)
        pygame.draw.rect(surface, (255, 255, 255), address_bar)
        address_text = small_font.render("/home/user/", True, (0, 0, 0))
        surface.blit(address_text, (address_bar.x + 5, address_bar.y + (address_bar.height - address_text.get_height()) // 2))
        
        # Draw file list
        file_list_y = content_y + toolbar_height
        file_list_height = content_height - toolbar_height
        
        # Draw column headers
        header_height = 25
        pygame.draw.rect(surface, TASKBAR_COLOR, 
                        (self.x, file_list_y, self.width, header_height))
        
        name_header = small_font.render("Name", True, TEXT_COLOR)
        surface.blit(name_header, (self.x + 10, file_list_y + (header_height - name_header.get_height()) // 2))
        
        type_header = small_font.render("Type", True, TEXT_COLOR)
        surface.blit(type_header, (self.x + self.width - 200, file_list_y + (header_height - type_header.get_height()) // 2))
        
        size_header = small_font.render("Size", True, TEXT_COLOR)
        surface.blit(size_header, (self.x + self.width - 100, file_list_y + (header_height - size_header.get_height()) // 2))
        
        # Draw files
        file_y = file_list_y + header_height
        item_height = 30
        
        for i, file in enumerate(self.files):
            # Draw background (highlight selected file)
            bg_color = (200, 220, 240) if i == self.selected_file else WINDOW_COLOR
            pygame.draw.rect(surface, bg_color, 
                            (self.x, file_y + i * item_height, self.width, item_height))
            
            # Draw icon based on type
            icon_color = (255, 196, 0) if file["type"] == "folder" else (128, 128, 128)
            pygame.draw.rect(surface, icon_color, 
                            (self.x + 5, file_y + i * item_height + 5, 20, 20))
            
            # Draw file name
            name_text = small_font.render(file["name"], True, (0, 0, 0))
            surface.blit(name_text, (self.x + 30, file_y + i * item_height + (item_height - name_text.get_height()) // 2))
            
            # Draw file type
            type_text = small_font.render(file["type"], True, (0, 0, 0))
            surface.blit(type_text, (self.x + self.width - 200, file_y + i * item_height + (item_height - type_text.get_height()) // 2))
            
            # Draw file size
            size_text = small_font.render(file["size"], True, (0, 0, 0))
            surface.blit(size_text, (self.x + self.width - 100, file_y + i * item_height + (item_height - size_text.get_height()) // 2))
    
    def draw_browser(self, surface, content_y, content_height):
        """Draw browser content"""
        # Draw browser background
        pygame.draw.rect(surface, (255, 255, 255), 
                        (self.x, content_y, self.width, content_height))
        
        # Draw toolbar
        toolbar_height = 30
        pygame.draw.rect(surface, TITLEBAR_COLOR, 
                        (self.x, content_y, self.width, toolbar_height))
        
        # Draw navigation buttons
        nav_btn_width = 60
        back_btn = pygame.Rect(self.x + 5, content_y + 5, nav_btn_width, toolbar_height - 10)
        pygame.draw.rect(surface, TASKBAR_COLOR, back_btn)
        back_text = small_font.render("Back", True, TEXT_COLOR)
        surface.blit(back_text, (back_btn.x + (nav_btn_width - back_text.get_width()) // 2, 
                                back_btn.y + (back_btn.height - back_text.get_height()) // 2))
        
        forward_btn = pygame.Rect(self.x + 10 + nav_btn_width, content_y + 5, nav_btn_width, toolbar_height - 10)
        pygame.draw.rect(surface, TASKBAR_COLOR, forward_btn)
        forward_text = small_font.render("Forward", True, TEXT_COLOR)
        surface.blit(forward_text, (forward_btn.x + (nav_btn_width - forward_text.get_width()) // 2, 
                                   forward_btn.y + (forward_btn.height - forward_text.get_height()) // 2))
        
        refresh_btn = pygame.Rect(self.x + 15 + nav_btn_width * 2, content_y + 5, nav_btn_width, toolbar_height - 10)
        pygame.draw.rect(surface, TASKBAR_COLOR, refresh_btn)
        refresh_text = small_font.render("Refresh", True, TEXT_COLOR)
        surface.blit(refresh_text, (refresh_btn.x + (nav_btn_width - refresh_text.get_width()) // 2, 
                                   refresh_btn.y + (refresh_btn.height - refresh_text.get_height()) // 2))
        
        # Draw address bar
        address_bar = pygame.Rect(self.x + 20 + nav_btn_width * 3, content_y + 5, 
                                 self.width - 25 - nav_btn_width * 3, toolbar_height - 10)
        pygame.draw.rect(surface, (255, 255, 255), address_bar)
        address_text = small_font.render(self.browser_url, True, (0, 0, 0))
        surface.blit(address_text, (address_bar.x + 5, address_bar.y + (address_bar.height - address_text.get_height()) // 2))
        
        # Draw web content
        content_area_y = content_y + toolbar_height
        content_area_height = content_height - toolbar_height
        
        # Draw AMOS OS website
        title_text = large_font.render("Welcome to AMOS Desktop OS", True, (0, 0, 0))
        surface.blit(title_text, (self.x + 20, content_area_y + 20))
        
        subtitle_text = font.render("A graphical desktop environment for minimal OS", True, (0, 0, 0))
        surface.blit(subtitle_text, (self.x + 20, content_area_y + 60))
        
        # Feature list
        features = [
            "Fluxbox-inspired window management with tabs",
            "Responsive UI with advanced window resizing",
            "Full suite of desktop applications",
            "Native 3D rendering with C + Assembly",
            "Optimal performance on minimal hardware"
        ]
        
        for i, feature in enumerate(features):
            y_pos = content_area_y + 100 + i * 30
            feature_text = font.render("• " + feature, True, (0, 0, 0))
            surface.blit(feature_text, (self.x + 30, y_pos))
        
        # Draw image placeholder
        image_rect = pygame.Rect(self.x + self.width - 250, content_area_y + 80, 200, 150)
        pygame.draw.rect(surface, (240, 240, 240), image_rect)
        pygame.draw.rect(surface, (200, 200, 200), image_rect, 2)
        image_text = font.render("AMOS OS", True, (100, 100, 100))
        surface.blit(image_text, (image_rect.x + (image_rect.width - image_text.get_width()) // 2, 
                                 image_rect.y + (image_rect.height - image_text.get_height()) // 2))
    
    def draw_settings(self, surface, content_y, content_height, app_state):
        """Draw settings content"""
        # Draw settings background
        pygame.draw.rect(surface, WINDOW_COLOR, 
                        (self.x, content_y, self.width, content_height))
        
        # Draw tabs for settings sections
        section_height = 40
        section_width = self.width // len(self.settings_sections)
        
        for i, section in enumerate(self.settings_sections):
            section_x = self.x + i * section_width
            bg_color = TITLEBAR_ACTIVE_COLOR if i == self.active_section else TITLEBAR_COLOR
            if app_state.dark_mode:
                bg_color = (64, 115, 158) if i == self.active_section else (24, 44, 97)
            
            pygame.draw.rect(surface, bg_color, 
                            (section_x, content_y, section_width, section_height))
            
            section_text = font.render(section, True, TEXT_COLOR)
            surface.blit(section_text, (section_x + (section_width - section_text.get_width()) // 2, 
                                       content_y + (section_height - section_text.get_height()) // 2))
        
        # Draw content based on active section
        content_area_y = content_y + section_height
        content_area_height = content_height - section_height
        
        if self.active_section == 0:  # Appearance
            title_text = font.render("Theme", True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
            surface.blit(title_text, (self.x + 20, content_area_y + 20))
            
            theme_y = content_area_y + 60
            for i, theme in enumerate(self.themes):
                theme_rect = pygame.Rect(self.x + 100, theme_y + i * 40, 100, 30)
                bg_color = TITLEBAR_ACTIVE_COLOR if i == self.current_theme else TITLEBAR_COLOR
                if app_state.dark_mode:
                    bg_color = (64, 115, 158) if i == self.current_theme else (24, 44, 97)
                
                pygame.draw.rect(surface, bg_color, theme_rect)
                theme_text = font.render(theme.capitalize(), True, TEXT_COLOR)
                surface.blit(theme_text, (theme_rect.x + (theme_rect.width - theme_text.get_width()) // 2, 
                                         theme_rect.y + (theme_rect.height - theme_text.get_height()) // 2))
            
            # Font size option
            font_text = font.render("Font Size", True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
            surface.blit(font_text, (self.x + 20, content_area_y + 180))
            
            # Font size slider
            slider_rect = pygame.Rect(self.x + 100, content_area_y + 190, 150, 10)
            pygame.draw.rect(surface, (150, 150, 150), slider_rect)
            pygame.draw.rect(surface, TITLEBAR_ACTIVE_COLOR, 
                            (slider_rect.x, slider_rect.y, 75, slider_rect.height))
            pygame.draw.circle(surface, TITLEBAR_ACTIVE_COLOR, (slider_rect.x + 75, slider_rect.y + 5), 10)
        
        elif self.active_section == 1:  # Desktop
            title_text = font.render("Desktop Background", True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
            surface.blit(title_text, (self.x + 20, content_area_y + 20))
            
            # Background color options
            colors = [
                {"name": "Blue", "color": (9, 132, 227)},
                {"name": "Dark", "color": (45, 52, 54)},
                {"name": "Green", "color": (0, 184, 148)},
                {"name": "Purple", "color": (108, 92, 231)}
            ]
            
            for i, color_opt in enumerate(colors):
                color_rect = pygame.Rect(self.x + 50 + i * 80, content_area_y + 60, 60, 60)
                pygame.draw.rect(surface, color_opt["color"], color_rect)
                pygame.draw.rect(surface, (255, 255, 255), color_rect, 2)
                
                color_text = small_font.render(color_opt["name"], True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
                surface.blit(color_text, (color_rect.x + (color_rect.width - color_text.get_width()) // 2, 
                                         color_rect.y + color_rect.height + 5))
            
            # Desktop icons options
            icons_text = font.render("Desktop Icons", True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
            surface.blit(icons_text, (self.x + 20, content_area_y + 150))
            
            # Show desktop icons checkbox
            checkbox_rect = pygame.Rect(self.x + 100, content_area_y + 190, 20, 20)
            pygame.draw.rect(surface, (255, 255, 255), checkbox_rect)
            pygame.draw.rect(surface, (0, 0, 0), checkbox_rect, 2)
            # Draw check mark
            pygame.draw.line(surface, (0, 0, 0), 
                            (checkbox_rect.x + 5, checkbox_rect.y + 10), 
                            (checkbox_rect.x + 8, checkbox_rect.y + 15), 2)
            pygame.draw.line(surface, (0, 0, 0), 
                            (checkbox_rect.x + 8, checkbox_rect.y + 15), 
                            (checkbox_rect.x + 15, checkbox_rect.y + 5), 2)
            
            checkbox_text = font.render("Show desktop icons", True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
            surface.blit(checkbox_text, (checkbox_rect.x + 30, checkbox_rect.y))
        
        elif self.active_section == 2:  # System
            title_text = font.render("System Information", True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
            surface.blit(title_text, (self.x + 20, content_area_y + 20))
            
            system_info = [
                {"label": "OS Version", "value": "AMOS Desktop OS v1.0"},
                {"label": "Architecture", "value": "x86_64"},
                {"label": "Memory", "value": "512 MB"},
                {"label": "Disk Space", "value": "1 GB free of 4 GB"},
                {"label": "Display", "value": f"{SCREEN_WIDTH}x{SCREEN_HEIGHT}, 32-bit color"}
            ]
            
            for i, info in enumerate(system_info):
                y_pos = content_area_y + 60 + i * 30
                label_text = font.render(info["label"] + ":", True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
                surface.blit(label_text, (self.x + 30, y_pos))
                
                value_text = font.render(info["value"], True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
                surface.blit(value_text, (self.x + 170, y_pos))
        
        elif self.active_section == 3:  # About
            title_text = font.render("About AMOS Desktop OS", True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
            surface.blit(title_text, (self.x + 20, content_area_y + 20))
            
            about_text = [
                "AMOS Desktop OS v1.0",
                "Copyright © 2025 AMOS Team",
                "",
                "A graphical desktop environment for minimal OS",
                "with Fluxbox-inspired window management and tabbed windows.",
                "",
                "License: MIT",
                "Website: https://amos-os.org",
                "",
                "Built with C, Assembly, and love."
            ]
            
            for i, line in enumerate(about_text):
                y_pos = content_area_y + 60 + i * 25
                line_text = font.render(line, True, (0, 0, 0) if not app_state.dark_mode else (255, 255, 255))
                surface.blit(line_text, (self.x + 30, y_pos))
    
    def draw_default(self, surface, content_y, content_height):
        """Draw default window content"""
        # Draw content area
        pygame.draw.rect(surface, WINDOW_COLOR, 
                        (self.x, content_y, self.width, content_height))
        
        # Draw some decorative elements
        # Header
        header_height = 40
        pygame.draw.rect(surface, TITLEBAR_COLOR, 
                        (self.x, content_y, self.width, header_height))
        
        header_text = font.render("Window Content", True, TEXT_COLOR)
        surface.blit(header_text, (self.x + 10, content_y + (header_height - header_text.get_height()) // 2))
        
        # Content panels
        panel_width = self.width // 2 - 15
        panel_height = content_height - header_height - 20
        
        # Left panel
        left_panel = pygame.Rect(self.x + 10, content_y + header_height + 10, panel_width, panel_height)
        pygame.draw.rect(surface, (245, 245, 245), left_panel)
        pygame.draw.rect(surface, (200, 200, 200), left_panel, 2)
        
        panel_title = font.render("Panel 1", True, (0, 0, 0))
        surface.blit(panel_title, (left_panel.x + 10, left_panel.y + 10))
        
        # Right panel
        right_panel = pygame.Rect(self.x + 20 + panel_width, content_y + header_height + 10, panel_width, panel_height)
        pygame.draw.rect(surface, (245, 245, 245), right_panel)
        pygame.draw.rect(surface, (200, 200, 200), right_panel, 2)
        
        panel_title = font.render("Panel 2", True, (0, 0, 0))
        surface.blit(panel_title, (right_panel.x + 10, right_panel.y + 10))

# TaskBar class
class TaskBar:
    def __init__(self, width):
        self.width = width
        self.height = TASKBAR_HEIGHT
        self.y = SCREEN_HEIGHT - TASKBAR_HEIGHT
        self.buttons = []
        
        # Start menu button
        self.start_btn_width = 80
        self.start_menu_open = False
        self.start_menu = None
    
    def handle_event(self, event, app_state):
        """Handle taskbar events"""
        if event.type == pygame.MOUSEBUTTONDOWN:
            mx, my = event.pos
            
            # Check if clicking on start button
            if (0 <= mx <= self.start_btn_width and
                self.y <= my <= self.y + self.height):
                # Toggle start menu
                if self.start_menu_open:
                    self.start_menu_open = False
                    self.start_menu = None
                else:
                    self.start_menu_open = True
                    self.start_menu = StartMenu(0, self.y - 200)
                return True
            
            # Check if clicking on a taskbar button
            button_width = 160
            button_x = self.start_btn_width + 10
            
            for window in app_state.windows:
                if (button_x <= mx < button_x + button_width and
                    self.y <= my <= self.y + self.height):
                    if window.minimized:
                        window.minimized = False
                    app_state.active_window = window
                    window.active = True
                    return True
                button_x += button_width + 5
            
            # If start menu is open, check for clicks there
            if self.start_menu_open and self.start_menu:
                if self.start_menu.handle_event(event, app_state):
                    return True
                else:
                    # Close start menu if clicked elsewhere
                    self.start_menu_open = False
                    self.start_menu = None
        
        return False
    
    def draw(self, surface, app_state):
        """Draw the taskbar"""
        # Get taskbar color based on theme
        taskbar_color = app_state.get_theme_color("taskbar")
        
        # Draw taskbar background
        pygame.draw.rect(surface, taskbar_color, (0, self.y, self.width, self.height))
        
        # Draw start button
        start_btn_color = TITLEBAR_ACTIVE_COLOR if self.start_menu_open else TITLEBAR_COLOR
        if app_state.dark_mode:
            start_btn_color = (64, 115, 158) if self.start_menu_open else (24, 44, 97)
        
        pygame.draw.rect(surface, start_btn_color, (0, self.y, self.start_btn_width, self.height))
        start_text = font.render("Start", True, TEXT_COLOR)
        surface.blit(start_text, (10, self.y + (self.height - start_text.get_height()) // 2))
        
        # Draw window buttons
        button_width = 160
        button_x = self.start_btn_width + 10
        
        for window in app_state.windows:
            if not window.is_tab:  # Don't show tabs in taskbar
                button_color = TITLEBAR_ACTIVE_COLOR if window.active else TITLEBAR_COLOR
                if app_state.dark_mode:
                    button_color = (64, 115, 158) if window.active else (24, 44, 97)
                
                pygame.draw.rect(surface, button_color, 
                                (button_x, self.y + 5, button_width, self.height - 10))
                
                # Truncate title if too long
                title = window.title
                if len(title) > 15:
                    title = title[:12] + "..."
                
                button_text = font.render(title, True, TEXT_COLOR)
                surface.blit(button_text, (button_x + 10, self.y + (self.height - button_text.get_height()) // 2))
                
                button_x += button_width + 5
        
        # Draw start menu if open
        if self.start_menu_open and self.start_menu:
            self.start_menu.draw(surface, app_state)
        
        # Draw clock
        current_time = time.strftime("%H:%M:%S")
        clock_text = font.render(current_time, True, TEXT_COLOR)
        surface.blit(clock_text, (self.width - clock_text.get_width() - 10, 
                                 self.y + (self.height - clock_text.get_height()) // 2))

# Desktop icon class
class DesktopIcon:
    def __init__(self, x, y, name, action):
        self.x = x
        self.y = y
        self.width = DESKTOP_ICON_SIZE
        self.height = DESKTOP_ICON_SIZE
        self.name = name
        self.action = action
    
    def handle_event(self, event, app_state):
        """Handle icon events"""
        if event.type == pygame.MOUSEBUTTONDOWN:
            mx, my = event.pos
            
            # Check if clicking on the icon
            if (self.x <= mx <= self.x + self.width and
                self.y <= my <= self.y + self.height + 20):
                if event.button == 1:  # Left click
                    # Double-click detection (simplified)
                    if pygame.time.get_ticks() % 5 == 0:
                        self.action()
                        return True
                elif event.button == 3:  # Right click
                    # Show context menu
                    app_state.context_menu = ContextMenu(mx, my)
                    return True
        
        return False
    
    def draw(self, surface, app_state):
        """Draw the desktop icon"""
        # Draw icon background (highlight if being clicked)
        bg_color = app_state.get_theme_color("desktop")
        
        # Draw icon
        icon_color = (255, 196, 0) if self.name == "Files" else (9, 132, 227)
        if self.name == "Terminal":
            icon_color = (0, 184, 148)
        elif self.name == "Browser":
            icon_color = (214, 48, 49)
        elif self.name == "Settings":
            icon_color = (108, 92, 231)
        
        pygame.draw.rect(surface, icon_color, (self.x, self.y, self.width, self.height))
        
        # Draw icon name
        name_text = small_font.render(self.name, True, TEXT_COLOR)
        text_x = self.x + (self.width - name_text.get_width()) // 2
        text_y = self.y + self.height + 5
        
        # Draw text background for better visibility
        text_bg_rect = pygame.Rect(text_x - 2, text_y - 2, 
                                  name_text.get_width() + 4, name_text.get_height() + 4)
        pygame.draw.rect(surface, bg_color, text_bg_rect)
        
        surface.blit(name_text, (text_x, text_y))

# Start Menu class
class StartMenu:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.width = 200
        self.height = 200
        
        self.menu_items = [
            {"name": "Terminal", "action": lambda: app_state.create_terminal_window()},
            {"name": "File Manager", "action": lambda: app_state.create_file_manager_window()},
            {"name": "Web Browser", "action": lambda: app_state.create_browser_window()},
            {"name": "Settings", "action": lambda: app_state.create_settings_window()},
            {"name": "Log Out", "action": lambda: None}
        ]
        
        self.item_height = 40
    
    def handle_event(self, event, app_state):
        """Handle menu events"""
        if event.type == pygame.MOUSEBUTTONDOWN:
            mx, my = event.pos
            
            # Check if clicking inside menu
            if (self.x <= mx <= self.x + self.width and
                self.y <= my <= self.y + self.height):
                
                # Check which item was clicked
                item_index = (my - self.y) // self.item_height
                if 0 <= item_index < len(self.menu_items):
                    action = self.menu_items[item_index]["action"]
                    if action:
                        action()
                    return True
            
            return False
    
    def draw(self, surface, app_state):
        """Draw the start menu"""
        # Draw menu background
        pygame.draw.rect(surface, CONTEXTMENU_BG_COLOR, (self.x, self.y, self.width, self.height))
        pygame.draw.rect(surface, (100, 100, 100), (self.x, self.y, self.width, self.height), 1)
        
        # Draw menu items
        for i, item in enumerate(self.menu_items):
            item_y = self.y + i * self.item_height
            
            # Check if mouse is hovering over this item
            mx, my = pygame.mouse.get_pos()
            is_hovering = (self.x <= mx <= self.x + self.width and
                          item_y <= my < item_y + self.item_height)
            
            # Draw item background (highlight if hovering)
            if is_hovering:
                pygame.draw.rect(surface, CONTEXTMENU_HOVER_COLOR, 
                                (self.x, item_y, self.width, self.item_height))
            
            # Draw item text
            item_text = font.render(item["name"], True, CONTEXTMENU_FG_COLOR)
            surface.blit(item_text, (self.x + 10, item_y + (self.item_height - item_text.get_height()) // 2))

# Context Menu class
class ContextMenu:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.width = CONTEXT_MENU_WIDTH
        self.height = CONTEXT_MENU_ITEM_HEIGHT * 4
        
        # Ensure menu doesn't go off screen
        if self.x + self.width > SCREEN_WIDTH:
            self.x = SCREEN_WIDTH - self.width
        if self.y + self.height > SCREEN_HEIGHT:
            self.y = SCREEN_HEIGHT - self.height
        
        self.menu_items = [
            {"name": "New Window", "action": lambda: app_state.create_terminal_window()},
            {"name": "Create Tab", "action": lambda: self.create_tab_in_active_window(app_state)},
            {"name": "Change Background", "action": lambda: None},
            {"name": "Properties", "action": lambda: None}
        ]
    
    def create_tab_in_active_window(self, app_state):
        """Create a new tab in the active window"""
        if app_state.active_window:
            # Create a new terminal window as a tab
            new_tab = Window(0, 0, app_state.active_window.width, app_state.active_window.height, 
                         "Terminal Tab", "terminal")
            app_state.active_window.add_tab(new_tab)
            app_state.windows.append(new_tab)
            new_tab.id = app_state.next_window_id
            app_state.next_window_id += 1
    
    def handle_event(self, event, app_state):
        """Handle menu events"""
        if event.type == pygame.MOUSEBUTTONDOWN:
            mx, my = event.pos
            
            # Check if clicking inside menu
            if (self.x <= mx <= self.x + self.width and
                self.y <= my <= self.y + self.height):
                
                # Check which item was clicked
                item_index = (my - self.y) // CONTEXT_MENU_ITEM_HEIGHT
                if 0 <= item_index < len(self.menu_items):
                    action = self.menu_items[item_index]["action"]
                    if action:
                        action()
                    app_state.context_menu = None
                    return True
            else:
                # Close context menu if clicked elsewhere
                app_state.context_menu = None
                return True
        
        return False
    
    def draw(self, surface, app_state):
        """Draw the context menu"""
        # Draw menu background
        pygame.draw.rect(surface, CONTEXTMENU_BG_COLOR, 
                        (self.x, self.y, self.width, self.height))
        pygame.draw.rect(surface, (100, 100, 100), 
                        (self.x, self.y, self.width, self.height), 1)
        
        # Draw menu items
        for i, item in enumerate(self.menu_items):
            item_y = self.y + i * CONTEXT_MENU_ITEM_HEIGHT
            
            # Check if mouse is hovering over this item
            mx, my = pygame.mouse.get_pos()
            is_hovering = (self.x <= mx <= self.x + self.width and
                          item_y <= my < item_y + CONTEXT_MENU_ITEM_HEIGHT)
            
            # Draw item background (highlight if hovering)
            if is_hovering:
                pygame.draw.rect(surface, CONTEXTMENU_HOVER_COLOR, 
                                (self.x, item_y, self.width, CONTEXT_MENU_ITEM_HEIGHT))
            
            # Draw item text
            item_text = font.render(item["name"], True, CONTEXTMENU_FG_COLOR)
            surface.blit(item_text, (self.x + 10, 
                                    item_y + (CONTEXT_MENU_ITEM_HEIGHT - item_text.get_height()) // 2))

# Create application state
app_state = AppState()

# Initialize taskbar
taskbar = TaskBar(SCREEN_WIDTH)

# Helper functions for creating windows
def create_terminal_window(self):
    """Create a terminal window and add it to app state"""
    window = Window(100, 100, 600, 400, "Terminal", "terminal")
    self.windows.append(window)
    window.id = self.next_window_id
    self.next_window_id += 1
    self.active_window = window
    window.active = True
    return window

def create_file_manager_window(self):
    """Create a file manager window and add it to app state"""
    window = Window(150, 150, 700, 500, "File Manager", "file_manager")
    self.windows.append(window)
    window.id = self.next_window_id
    self.next_window_id += 1
    self.active_window = window
    window.active = True
    return window

def create_browser_window(self):
    """Create a browser window and add it to app state"""
    window = Window(200, 100, 800, 600, "Web Browser", "browser")
    self.windows.append(window)
    window.id = self.next_window_id
    self.next_window_id += 1
    self.active_window = window
    window.active = True
    return window

def create_settings_window(self):
    """Create a settings window and add it to app state"""
    window = Window(250, 150, 700, 500, "Settings", "settings")
    self.windows.append(window)
    window.id = self.next_window_id
    self.next_window_id += 1
    self.active_window = window
    window.active = True
    return window

# Add methods to AppState
AppState.create_terminal_window = create_terminal_window
AppState.create_file_manager_window = create_file_manager_window
AppState.create_browser_window = create_browser_window
AppState.create_settings_window = create_settings_window

# Create some initial windows
terminal = app_state.create_terminal_window()
file_manager = app_state.create_file_manager_window()

# Create some tabs in the terminal
terminal.add_tab(app_state.create_browser_window())
terminal.add_tab(app_state.create_settings_window())

# Simulate OS boot process
def simulate_boot_process(screen):
    """Simulate OS boot process with loading bars"""
    boot_background_color = (0, 0, 0)
    text_color = (255, 255, 255)
    progress_bar_color = (9, 132, 227)
    
    screen.fill(boot_background_color)
    
    # Display boot messages
    messages = [
        "AMOS Boot Loader v1.0",
        "Initializing system...",
        "Loading kernel...",
        "Starting services...",
        "Initializing desktop environment..."
    ]
    
    total_steps = len(messages)
    
    for i, message in enumerate(messages):
        # Clear the screen
        screen.fill(boot_background_color)
        
        # Display AMOS OS logo text
        title_text = large_font.render("AMOS Desktop OS", True, text_color)
        screen.blit(title_text, ((SCREEN_WIDTH - title_text.get_width()) // 2, SCREEN_HEIGHT // 4))
        
        # Display current boot message
        message_text = font.render(message, True, text_color)
        screen.blit(message_text, ((SCREEN_WIDTH - message_text.get_width()) // 2, 
                                  SCREEN_HEIGHT // 2))
        
        # Draw progress bar
        progress_width = 400
        progress_height = 20
        progress_x = (SCREEN_WIDTH - progress_width) // 2
        progress_y = SCREEN_HEIGHT // 2 + 50
        
        progress_percent = (i + 1) / total_steps
        
        # Draw progress bar background
        pygame.draw.rect(screen, (50, 50, 50), 
                        (progress_x, progress_y, progress_width, progress_height))
        
        # Draw progress bar fill
        pygame.draw.rect(screen, progress_bar_color, 
                        (progress_x, progress_y, int(progress_width * progress_percent), progress_height))
        
        # Display progress percentage
        percent_text = font.render(f"{int(progress_percent * 100)}%", True, text_color)
        screen.blit(percent_text, ((SCREEN_WIDTH - percent_text.get_width()) // 2, 
                                  progress_y + progress_height + 10))
        
        pygame.display.flip()
        pygame.time.delay(800)  # Delay to simulate loading time

# Main function
def main():
    print("AMOS Desktop OS - Tabbed Windows VNC Demo")
    
    running = True
    show_boot_sequence = True
    
    # Simulate boot process
    if show_boot_sequence:
        simulate_boot_process(screen)
    
    while running:
        # Clear the screen with desktop color
        screen.fill(app_state.get_theme_color("desktop"))
        
        # Process events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            
            # Handle context menu events
            if app_state.context_menu:
                if app_state.context_menu.handle_event(event, app_state):
                    continue
            
            # Handle taskbar events
            if taskbar.handle_event(event, app_state):
                continue
            
            # Handle desktop icon events
            handled = False
            for icon in app_state.desktop_icons:
                if icon.handle_event(event, app_state):
                    handled = True
                    break
            if handled:
                continue
            
            # Handle window events
            if event.type == pygame.MOUSEBUTTONDOWN:
                # First check if clicking on any window
                window_clicked = False
                for window in reversed(app_state.windows):  # Start from top window
                    if not window.is_tab and not window.minimized and window.visible:
                        if window.x <= event.pos[0] <= window.x + window.width and window.y <= event.pos[1] <= window.y + window.height:
                            window_clicked = True
                            # Make this window active
                            for w in app_state.windows:
                                w.active = False
                            window.active = True
                            app_state.active_window = window
                            
                            # Handle the event
                            window.handle_event(event, app_state)
                            break
                
                # If right-clicking on desktop, show context menu
                if not window_clicked and event.button == 3:
                    app_state.context_menu = ContextMenu(event.pos[0], event.pos[1])
                    
            elif event.type == pygame.MOUSEMOTION:
                # Handle window dragging
                if app_state.drag_window:
                    app_state.drag_window.x = event.pos[0] - app_state.drag_offset[0]
                    app_state.drag_window.y = event.pos[1] - app_state.drag_offset[1]
                
                # Handle window resizing
                if app_state.resize_window:
                    new_width = max(200, event.pos[0] - app_state.resize_window.x)
                    new_height = max(150, event.pos[1] - app_state.resize_window.y)
                    app_state.resize_window.width = new_width
                    app_state.resize_window.height = new_height
            
            # Forward events to active window
            if app_state.active_window:
                app_state.active_window.handle_event(event, app_state)
        
        # Draw desktop icons
        for icon in app_state.desktop_icons:
            icon.draw(screen, app_state)
        
        # Draw windows from bottom to top
        for window in app_state.windows:
            if not window.is_tab:  # Don't draw tab windows directly
                window.draw(screen, app_state)
        
        # Draw taskbar
        taskbar.draw(screen, app_state)
        
        # Draw context menu if open
        if app_state.context_menu:
            app_state.context_menu.draw(screen, app_state)
        
        # Update the display
        pygame.display.flip()
        
        # Control the frame rate
        clock.tick(60)
    
    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()