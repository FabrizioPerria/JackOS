#ifndef __TIMER_H
#define __TIMER_H

#include <system.h>
#include <IRQ.h>

void timer_setup(int hz);
void timer_handler(struct registers *reg);
void timer_install();
void sleep(int seconds);
#endif
