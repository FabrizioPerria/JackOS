#include <ISR.h>
#include <system.h>
#include <screen.h>
#include <IDT.h>
#include <exception_messages.h>

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

/******************************************
* Print the description of the exception  *
* and forces a triple fault to the system *
*******************************************/

void isr_handler(struct registers *reg)
{
	if(reg != NULL){
		if(reg->intNum < 19){
			putString(exception_messages[reg->intNum]);
			putString(". System Halted.");
			asm("cli");
			asm("hlt");
		}
		if(reg->intNum < 32){
			putString(exception_messages[19]);
			putString(". System Halted.");
			asm("cli");
			asm("hlt");
		}
	}
}

/***********************************************
* Install the exception handlers on the system *
************************************************/

void isr_install()
{
	idt_setEntry(0,(unsigned)isr0,0x8,0x8E);
	idt_setEntry(1,(unsigned)isr1,0x8,0x8E);
	idt_setEntry(2,(unsigned)isr2,0x8,0x8E);
	idt_setEntry(3,(unsigned)isr3,0x8,0x8E);
	idt_setEntry(4,(unsigned)isr4,0x8,0x8E);
	idt_setEntry(5,(unsigned)isr5,0x8,0x8E);
	idt_setEntry(6,(unsigned)isr6,0x8,0x8E);
	idt_setEntry(7,(unsigned)isr7,0x8,0x8E);
	idt_setEntry(8,(unsigned)isr8,0x8,0x8E);
	idt_setEntry(9,(unsigned)isr9,0x8,0x8E);
	idt_setEntry(10,(unsigned)isr10,0x8,0x8E);
	idt_setEntry(11,(unsigned)isr11,0x8,0x8E);
	idt_setEntry(12,(unsigned)isr12,0x8,0x8E);
	idt_setEntry(13,(unsigned)isr13,0x8,0x8E);
	idt_setEntry(14,(unsigned)isr14,0x8,0x8E);
	idt_setEntry(15,(unsigned)isr15,0x8,0x8E);
	idt_setEntry(16,(unsigned)isr16,0x8,0x8E);
	idt_setEntry(17,(unsigned)isr17,0x8,0x8E);
	idt_setEntry(18,(unsigned)isr18,0x8,0x8E);
	idt_setEntry(19,(unsigned)isr19,0x8,0x8E);
	idt_setEntry(20,(unsigned)isr20,0x8,0x8E);
	idt_setEntry(21,(unsigned)isr21,0x8,0x8E);
	idt_setEntry(22,(unsigned)isr22,0x8,0x8E);
	idt_setEntry(23,(unsigned)isr23,0x8,0x8E);
	idt_setEntry(24,(unsigned)isr24,0x8,0x8E);
	idt_setEntry(25,(unsigned)isr25,0x8,0x8E);
	idt_setEntry(26,(unsigned)isr26,0x8,0x8E);
	idt_setEntry(27,(unsigned)isr27,0x8,0x8E);
	idt_setEntry(28,(unsigned)isr28,0x8,0x8E);
	idt_setEntry(29,(unsigned)isr29,0x8,0x8E);
	idt_setEntry(30,(unsigned)isr30,0x8,0x8E);
	idt_setEntry(31,(unsigned)isr31,0x8,0x8E);
}
