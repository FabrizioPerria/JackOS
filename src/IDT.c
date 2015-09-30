#include <IDT.h>
#include <system.h>

/* INTERRUPT DESCRIPTOR TABLE IMPLEMENTATION */

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

/******************************************
* Create an entry for the IDT             *
* indexEntry: index on the table          *
* base: offset on the table               *
* kSegment: segment selector in the GDT   *
* flags                                   *
*******************************************/
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
	/* Install the 256-items structure to register the interrupt service routines(ISR) */
	idtPtr.limit = (sizeof(struct idt_entry) * 256) -1;
	idtPtr.base = (unsigned int) &idtTable;
	memset ((unsigned char*)(&idtTable),0,sizeof(struct idt_entry) * 256);

	idt_flush();
}
