#ifndef __IRQ_H
#define __IRQ_H

#include <system.h>

void irq_setHandler(unsigned int irqNum, void (*handler)(struct registers*));
void irq_unset_handler(unsigned int irqNum);
void irq_handler(struct registers *reg);
void irq_install();

#endif
