#include <IDT.h>
#include <system.h>

extern void idt_flush();

struct idt_entry{
	unsigned short base_low;
	unsigned short segment;
	unsigned char zeroes;
	unsigned char flags;
	unsigned short base_high;
}__attribute__((packed));

struct idt_ptr
{
	unsigned short limit;
	unsigned int base;
}__attribute__((packed));

struct idt_ptr idtPtr;
struct idt_entry idtTable[256];

/******************************
* Create an entry for the IDT *
*******************************/

void idt_setEntry(unsigned char indexEntry,unsigned long base, unsigned short kSegment,unsigned char flags)
{
	idtTable[indexEntry].base_low = (base & 0xffff);
	idtTable[indexEntry].base_high = ((base >> 16) & 0xFFFF);
	idtTable[indexEntry].segment = kSegment;
	idtTable[indexEntry].zeroes = 0;
	idtTable[indexEntry].flags = flags;
}

/********************************
* Install the IDT in the system *
*********************************/

void idt_install()
{
	idtPtr.limit = (sizeof(struct idt_entry) * 256) -1;
	idtPtr.base = (unsigned int) &idtTable;
	memset ((unsigned char*)(&idtTable),0,sizeof(struct idt_entry) * 256);

	idt_flush();
}
