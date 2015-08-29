#ifndef __SCREEN_H
#define __SCREEN_H

#include <system.h>
#include <varFunc.h>

enum vga_colors{
    BLACK=0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GREY,
    DARK_GREY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    LIGHT_BROWN,
    WHITE
};

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_BYTES_PER_CHAR 2        /* 1 byte color */
				    /* 1 byte character */  

void setBackground(enum vga_colors background);
void setForeground(enum vga_colors foreground);
void scrollEnable();
void scrollDisable();
void scrollDown();
void clearScreen();
void moveCursor(int x,int y);
int getXCursor();
int getYCursor();
void putChar(char c);
void putString(char *string);
void printNum(int num);
void print(char *str,...);
void initScreen();
void refreshTimer();
void fullScreen(int enable);
#endif
