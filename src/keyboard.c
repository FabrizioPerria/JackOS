#include <screen.h>
#include <system.h>
#include <keyboard.h>
#include <layouts.h>
#include <IDT.h>
#include <IRQ.h>
 
static unsigned char flags=0;

static unsigned char lastKeyPressed=0;

static unsigned char *layout=layoutUS;

enum set{
    SHIFT= 0,
    ALT_LEFT,
    ALT_RIGHT,
    CTRL_LEFT,
    CTRL_RIGHT,
    EXTENDED,
    CAPS,
    SCROLL,
    NUM
};

/***************************************************
* Print on the screen the character pressed by     *
* the user basing on the layout chosen by the user *
****************************************************/

void keyboard_handler(struct registers *regs)
{
    unsigned char code=0;
    
    (void)regs;			/* Don't check regs
                                   because i'm not gonna use it!*/

    code=inPortB(KEYBOARD_DATA);

    if((code & 0x80) == 0){
        /* Scan code pressed */
        switch(code){
        case 0x2A:
        case 0x36:{
            /* LEFT/RIGHT SHIFT */
            flags|=(1<<SHIFT);
            break;
        }
        default:{
            lastKeyPressed=layout[code+(90*(flags&1))];
            break;
        }
        }
    } else {
        /* scan code released */
        switch(code){
        case 0xAA:
        case 0xB6:{
            /* LEFT/RIGHT SHIFT */
            flags&=~(1<<SHIFT);
            break;
        }
        default:{
            break;
        }
        }
    }
}

/*************************************
* install the keyboard in the system *
**************************************/
void keyboard_install(){
    irq_setHandler(1,&keyboard_handler);
}

void setLayout(int l)
{
    switch(l){
    case 0:{
        layout=layoutUS;
        break;
    }
    case 1:{
        layout=layoutIT;
        break;
    }
    default:{
        layout=layoutUS;
    }
    }
}

unsigned char getLastKeyPressed()
{
    unsigned char tmp;
    asm("cli");
    tmp=lastKeyPressed;
    lastKeyPressed=0;
    asm("sti");
    return tmp;
}
