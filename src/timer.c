#include <timer.h>
#include <screen.h>

/*
  PROGRAMMING 8253 Timer

  TIMER CLOCK 1193180 Hz
 
  COUNTER #0  port 0x40
  COUNTER #1  port 0x41
  COUNTER #2  port 0x42
  
  CONTROL REGISTER  port 0x43
    SC1,SC0 select counter #0,#1,#2
    RL1,RL0 
      0   0  counter latch -> value ready for the next read
      0   1  read/write LSB
      1   0  read/write MSB
      1   1  read/write LSB and then MSB
      
    M2,M1,M0 
     0  0  0  mode #0     interrupt after count (one shot)
     0  0  1  mode #1     retriggerable one shot
     X  1  0  mode #2     frequency generator
     X  1  1  mode #3     square wave generator
     1  0  0  mode #4     SW triggered strobe
     1  0  1  mode #5     HW triggered strobe
     
    BCD = 0 binary count | 1 BCD count 
*/

static int ticks=0;
static int seconds=0;

void timer_setup(int hz)
{
    int divisor =0;
    if(hz <= 0)
        return;
    divisor = 1193180/hz;
    outPortB(0x43,__extension__ 0b00110110);  /*counter #0 with 16 bit value in binary square wave mode*/
    outPortB(0x40,divisor & 0xFF);
    outPortB(0x40,divisor >> 8);
}

void timer_handler(struct registers *reg)
{
    (void)reg;
    ticks++;
    if(ticks % 121 == 0){
        /*putString("\r\nAnother second passed: ");*/
        /*printNum(++seconds);*/
        seconds++;
        refreshTimer();
    }
}

/* Install the timer into the system */
void timer_install()
{
    irq_setHandler(0,&timer_handler);
    timer_setup(121);	/* 1 second */
}

/* function to let the kernel sleep for x seconds */
void sleep(int secSleep)
{
    int start=seconds+secSleep;
    if(secSleep<0) return;
    while(seconds < (start)){
        putChar(0);		/*do something*/
    }
}

