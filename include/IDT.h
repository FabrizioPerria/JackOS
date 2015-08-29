#ifndef __IDT_H
#define __IDT_H

void idt_setEntry(unsigned char indexEntry,unsigned long base,unsigned short kSegment,unsigned char flags);
void idt_install();

#endif
