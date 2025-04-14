#pragma once

#include "types.h"

static ui64 videoIndex = 0;
ui64 tick;
extern void outPort(ui16 port, ui32 value);
extern ui32 inPort(ui16 port);
extern void asm_cli();
extern void asm_sti();
#define VIDEO_MEMORY_ADDR 0xb8000
void putchar(char c) {
    *((char*)VIDEO_MEMORY_ADDR + videoIndex*2) = c;        // Character
    *((char*)VIDEO_MEMORY_ADDR + videoIndex*2 + 1) = 0x0F; // Attribute (white on black)
    videoIndex++;
}
void print(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        putchar(str[i]);
        i++;
    }
}
void printint(int in) {
    char buf[128];
    int tmp = in;
    int i = 0;
    while (1) {
        int digit = tmp % 10;
        buf[i] = 48 + digit;
        tmp -= digit;
        if (tmp == 0) {
            break;
        }
        tmp /= 10;
        i++;
    }
    for (int j = i; j >=0; j--) {
        putchar(buf[j]);
    }

}
void printnl() {
    videoIndex += 80 - (videoIndex % 80);
}
void printl(const char* str) {
    print(str);
    printnl();
}
void printat(const char* str, int x, int y) {
    int prevVidIdx = videoIndex;
    videoIndex = x + y * 80;
    print(str);
    videoIndex = prevVidIdx;
}
void printintat(int in, int x, int y) {
    int prevVidIdx = videoIndex;
    videoIndex = x + y * 80;
    printint(in);
    videoIndex = prevVidIdx;
}
char getCharFromScancode(ui8 scancode) {
    switch (scancode) {
        case 0x2: return '1';
        case 0x3: return '2';
        case 0x4: return '3';
        case 0x5: return '4';
        case 0x6: return '5';
        case 0x7: return '6';
        case 0x8: return '7';
        case 0x9: return '8';
        case 0xa: return '9';
        case 0xb: return '0';
        case 0xc: return '-';
        case 0xd: return '=';
        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x1a: return '[';
        case 0x1b: return ']';
        case 0x2b: return '\\';
        case 0x1e: return 'a';
        case 0x1f: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x27: return ';';
        case 0x28: return '\'';
        case 0x2c: return 'z';
        case 0x2d: return 'x';
        case 0x2e: return 'c';
        case 0x2f: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';
        case 0x33: return ',';
        case 0x34: return '.';
        case 0x35: return '/';
        default: return '\0';
    }
}