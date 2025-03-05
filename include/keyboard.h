#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#define KEYBOARD_DATA 0x60
#define KEYBOARD_CMD 0x60
#define KEYBOARD_STATUS 0x64

#define OUT_BUFFER_READY 1 << 0
#define IN_BUFFER_FULL 1 << 1
#define KEY_NOT_LOCKED 1 << 4
#define TIMEOUT 1 << 6
#define PARITY_ERROR 1 << 7

enum keyLed
{
    SCROLL_LOCK = 1,
    NUM_LOCK = 2,
    CAPS_LOCK = 4
};

void keyboard_install();
void setLayout (int l);
unsigned char getLastKeyPressed();
unsigned char waitForKeyPress();
void resetKey();

#endif
