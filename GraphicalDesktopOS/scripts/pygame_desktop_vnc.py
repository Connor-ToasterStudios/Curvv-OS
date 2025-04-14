#!/usr/bin/env python3
"""
AMOS Desktop OS - PyGame VNC Visualization
This script creates a visual representation of the AMOS Desktop OS
that can be viewed through Replit's VNC viewer.
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

# Load fonts
try:
    system_font = pygame.font.SysFont("Arial", 14)
    title_font = pygame.font.SysFont("Arial", 16)
    terminal_font = pygame.font.SysFont("Courier New", 14)
except:
    # Fallback to default font if Arial is not available
    system_font = pygame.font.Font(None, 20)
    title_font = pygame.font.Font(None, 22)
    terminal_font = pygame.font.Font(None, 20)

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
        self.terminal_prompt = "$ "
        self.terminal_history = [
            "AMOS Desktop OS Terminal v1.0",
            "Type 'help' for available commands",
            "",
            "$ help",
            "Available commands:",
            "  help     - Display this help message",
            "  clear    - Clear the terminal screen",
            "  echo     - Display text",
            "  version  - Display AMOS version",
            "  exit     - Close the terminal",
            "",
            "$ version",
            "AMOS Desktop OS v1.0",
            "Copyright (c) 2025 AMOS Team",
            "",
            "$ "
        ]
        self.file_contents = [
            "AMOS File Manager v1.0",
            "",
            "/home/user/",
            "├── Documents/",
            "├── Pictures/",
            "├── Music/",
            "└── readme.txt"
        ]
        
    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:  # Left mouse button
                # Check if click is on title bar
                if (self.x <= event.pos[0] <= self.x + self.width and
                    self.y <= event.pos[1] <= self.y + 30):
                    self.dragging = True
                    self.drag_offset_x = event.pos[0] - self.x
                    self.drag_offset_y = event.pos[1] - self.y
                # Check if click is on resize handle (bottom right)
                elif (self.x + self.width - 20 <= event.pos[0] <= self.x + self.width and
                      self.y + self.height - 20 <= event.pos[1] <= self.y + self.height):
                    self.resizing = True
        
        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:  # Left mouse button
                self.dragging = False
                self.resizing = False
        
        elif event.type == pygame.MOUSEMOTION:
            if self.dragging:
                self.x = event.pos[0] - self.drag_offset_x
                self.y = event.pos[1] - self.drag_offset_y
            elif self.resizing:
                self.width = max(200, event.pos[0] - self.x)
                self.height = max(150, event.pos[1] - self.y)
    
    def draw(self, surface):
        # Draw window background
        pygame.draw.rect(surface, GRAY, (self.x, self.y, self.width, self.height))
        
        # Draw window border
        pygame.draw.rect(surface, DARK_GRAY, (self.x, self.y, self.width, self.height), 1)
        
        # Draw title bar
        pygame.draw.rect(surface, BLUE, (self.x, self.y, self.width, 30))
        
        # Draw window title
        title_text = title_font.render(self.title, True, WHITE)
        surface.blit(title_text, (self.x + 10, self.y + 7))
        
        # Draw window controls (close, maximize, minimize)
        pygame.draw.rect(surface, RED, (self.x + self.width - 25, self.y + 8, 15, 15))
        pygame.draw.rect(surface, YELLOW, (self.x + self.width - 45, self.y + 8, 15, 15))
        pygame.draw.rect(surface, GREEN, (self.x + self.width - 65, self.y + 8, 15, 15))
        
        # Draw window content
        if self.content_type == "terminal":
            self.draw_terminal(surface)
        elif self.content_type == "file_manager":
            self.draw_file_manager(surface)
        else:
            self.draw_default(surface)
        
        # Draw resize handle
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
        for i, line in enumerate(self.terminal_history[-min(len(self.terminal_history), (self.height - 40) // line_height):]):
            line_text = terminal_font.render(line, True, GREEN)
            surface.blit(line_text, (self.x + 10, self.y + 35 + i * line_height))
    
    def draw_file_manager(self, surface):
        # Draw file manager background
        pygame.draw.rect(surface, WHITE, (self.x, self.y + 30, self.width, self.height - 30))
        
        # Draw file manager content
        line_height = 20
        for i, line in enumerate(self.file_contents):
            text_color = BLUE if line.endswith('/') else BLACK
            line_text = system_font.render(line, True, text_color)
            surface.blit(line_text, (self.x + 10, self.y + 35 + i * line_height))
    
    def draw_default(self, surface):
        # Draw default content
        content_text = system_font.render("Window Content", True, BLACK)
        surface.blit(content_text, (self.x + 10, self.y + 40))

class TaskBar:
    def __init__(self, width):
        self.width = width
        self.height = 30
        self.buttons = [
            {"text": "Menu", "width": 50},
            {"text": "Terminal", "width": 80},
            {"text": "Files", "width": 60},
            {"text": "Settings", "width": 80}
        ]
    
    def draw(self, surface):
        # Draw taskbar background
        pygame.draw.rect(surface, BLUE, (0, 0, self.width, self.height))
        
        # Draw buttons
        x_offset = 10
        for button in self.buttons:
            pygame.draw.rect(surface, LIGHT_BLUE, (x_offset, 5, button["width"], 20))
            button_text = system_font.render(button["text"], True, WHITE)
            surface.blit(button_text, (x_offset + 5, 7))
            x_offset += button["width"] + 5
        
        # Draw time
        current_time = datetime.now().strftime("%H:%M:%S")
        time_text = system_font.render(current_time, True, WHITE)
        surface.blit(time_text, (self.width - 70, 7))

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
        elif icon_type == "search":
            self.text = "Search"
            self.icon_char = "🔍"
        else:
            self.text = "Icon"
            self.icon_char = "📄"
    
    def draw(self, surface):
        # Draw icon background
        pygame.draw.rect(surface, LIGHT_BLUE, (self.x, self.y, self.width, self.height), 0, 5)
        
        # Draw icon
        icon_text = pygame.font.SysFont("Arial", 30).render(self.icon_char, True, WHITE)
        surface.blit(icon_text, (self.x + self.width//2 - 12, self.y + 10))
        
        # Draw text
        text_surface = system_font.render(self.text, True, WHITE)
        surface.blit(text_surface, (self.x + self.width//2 - text_surface.get_width()//2, self.y + 40))

class ContextMenu:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.width = 150
        self.height = 120
        self.options = [
            "New Folder",
            "New File",
            "-----",
            "View",
            "Sort By",
            "-----",
            "Desktop Settings"
        ]
    
    def draw(self, surface):
        # Draw menu background
        pygame.draw.rect(surface, WHITE, (self.x, self.y, self.width, self.height))
        pygame.draw.rect(surface, DARK_GRAY, (self.x, self.y, self.width, self.height), 1)
        
        # Draw options
        for i, option in enumerate(self.options):
            if option == "-----":
                # Draw separator
                pygame.draw.line(surface, DARK_GRAY, 
                                (self.x, self.y + (i * 20) + 10),
                                (self.x + self.width, self.y + (i * 20) + 10), 1)
            else:
                # Draw option text
                option_text = system_font.render(option, True, BLACK)
                surface.blit(option_text, (self.x + 10, self.y + (i * 20) + 5))

def simulate_boot_process():
    """Simulate OS boot process with loading bars"""
    screen.fill(BLACK)
    
    loading_steps = [
        "BIOS Initialization...",
        "Bootloader Starting...",
        "Loading Kernel...",
        "Initializing Hardware...",
        "Starting System Services...",
        "Loading Desktop Environment..."
    ]
    
    y_pos = HEIGHT // 3
    for step in loading_steps:
        screen.fill(BLACK)
        
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
        
        y_pos += 70
        pygame.time.wait(200)

def main():
    # Simulate OS boot
    simulate_boot_process()
    
    # Set up desktop components
    taskbar = TaskBar(WIDTH)
    
    # Create windows
    terminal_window = Window(50, 50, 500, 350, "Terminal", "terminal")
    file_manager_window = Window(600, 100, 380, 300, "File Manager", "file_manager")
    
    # Create desktop icons
    icons = [
        DesktopIcon(50, 100, "system"),
        DesktopIcon(50, 180, "files"),
        DesktopIcon(50, 260, "settings"),
        DesktopIcon(50, 340, "search")
    ]
    
    # Create context menu (initially hidden)
    context_menu = None
    
    # Main loop
    running = True
    clock = pygame.time.Clock()
    show_desktop = True
    
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            
            # Handle window dragging and resizing
            terminal_window.handle_event(event)
            file_manager_window.handle_event(event)
            
            # Context menu
            if event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 3:  # Right mouse button
                    context_menu = ContextMenu(event.pos[0], event.pos[1])
                elif event.button == 1 and context_menu:  # Left click to close menu
                    context_menu = None
        
        # Draw desktop background
        screen.fill(DARK_BLUE)
        
        # Draw desktop icons
        for icon in icons:
            icon.draw(screen)
        
        # Draw windows
        if show_desktop:
            terminal_window.draw(screen)
            file_manager_window.draw(screen)
        
        # Draw context menu
        if context_menu:
            context_menu.draw(screen)
        
        # Draw taskbar
        taskbar.draw(screen)
        
        # Update screen
        pygame.display.flip()
        
        # Cap at 30 FPS
        clock.tick(30)
    
    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    print("Starting AMOS Desktop OS PyGame visualization...")
    print("This will display in the VNC window.")
    main()