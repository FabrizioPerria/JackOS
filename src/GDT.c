#include <GDT.h>
#include <system.h>

/* GLOBAL DESCRIPTOR TABLE IMPLEMENTATION */
/* Define memory segments */

/* GDT Entry structure */
struct gdtEntry
{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	/* ACCESS:
		8       7    5        4            3             2  1
		|PRESENT|RING|ALWAYS 1|CODE SEGMENT|DIRECTION BIT|RW|ACCESS(SET 0)|
	*/
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
}__attribute__((packed));

struct gdtPtr
{
	unsigned short limit;
	unsigned int base;
}__attribute__((packed));

struct gdtEntry gdtTable[3];
struct gdtPtr gdt_ptr;

extern void gdt_flush();

/*****************************
* Create an entry in the GDT *
******************************/

void gdt_setEntry(int numIndex,unsigned long base,unsigned long limit, unsigned char access, unsigned char granularity)
{
	/* No more than 3 entries possible */
	if(numIndex >=0 && numIndex < 3){
		/* Load the descriptor's characteristics on the structure's fields */
		gdtTable[numIndex].base_low = (base & 0xFFFF);
		gdtTable[numIndex].base_middle = ((base >> 16) & 0xFF);
		gdtTable[numIndex].base_high = ((base >> 24) & 0xFF);
		gdtTable[numIndex].limit_low = (limit & 0xFFFF);
		gdtTable[numIndex].granularity = (((limit >> 16)& 0x0F)|(granularity & 0xF0));
		gdtTable[numIndex].access = access;
	}
}

/****************************
* Install GDT in the system *
*****************************/
void gdt_install()
{
/* Define the table; this pointer will be given directly to the GDT register when the table is filled up */
	gdt_ptr.limit = (sizeof(struct gdtEntry) * 3) - 1;
	gdt_ptr.base = (unsigned int)&gdtTable;

/*  Usually we gotta create 3 entries; the first will be NULL, the second is going to be the code 
	descriptor and the last one will contain the data dewscriptor */

	gdt_setEntry(0,0,0,0,0);			/* NULL DESCRIPTOR */
	gdt_setEntry(1,0,0xFFFFFFFF,0x9A,0xCF);     /* SUPERVISOR CODE DESCRIPTOR */
	gdt_setEntry(2,0,0xFFFFFFFF,0x92,0xCF);     /* SUPERVISOR DATA DESCRIPTOR */
	gdt_setEntry(3,0,0xFFFFFFFF,0xFA,0xCF);		/*USER CODE DESCRIPTOR */
	gdt_setEntry(4,0,0xFFFFFFFF,0xF2,0xCF);		/* USER DATA DESCRIPTOR */

/* The flush will install the table on the system */
/* This function is written in assembly language because we gotta use the lgdt function
   to load the table in the GDT register */
	gdt_flush();
}
