#include <IRQ.h>
#include <IDT.h>
#include <system.h>

/* INTERRUPT REQUESTS IMPLEMENTATION */

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/* Lookup table for the IRQ routines */
void (*irqRoutines[16])(struct registers*)={0};

/*
* Install an handler in the lookup table
* irqNum: number of interrupt to register
* handler: pointer to the handler of the interrupt
*/
void irq_setHandler(unsigned int irqNum,void (*handler)(struct registers *))
{
	if(handler != NULL && irqNum < 16)
		irqRoutines[irqNum]=handler;
}

/* Uninstall an handler from the lookup table given the number of interrupt*/
void irq_unsetHandler(unsigned int irqNum)
{
	if(irqNum < 16)
		irqRoutines[irqNum]=NULL;
}

/************************************************
*  Select the handler from the loopup table and *
*  executes it                                  *
*************************************************/

void irq_handler(struct registers *reg)
{
	void (*handler)(struct registers*);
	if(reg!=NULL){
		handler=irqRoutines[reg->intNum-32];
		if(handler != NULL)
			handler(reg);		/* If exists, execute the handler */

		if(reg->intNum >=40)
			outPortB(0xA0,0x20);
		outPortB(0x20,0x20);
	}
}

/*****************
* Setup the PIC  *
******************/
void PIC8259_remap()
{
    /*
        MASTER PIC COMMAND PORT 0x20
        MASTER PIC DATA PORT    0x21
        SLAVE PIC COMMAND PORT  0xA0
        SLAVE PIC DATA PORT     0xA1

        ICW1 -> basic function of the PIC:
            A0 = 0
            D7,D6,D5 = Don't care
            D4 = 1
            D3 = 0 level triggered | 1 edge triggered
            D2 = Don't care
            D1 = 0 no cascade | 1 2 PICs in cascade
            D0 = 0 no ICW4 required | 1 ICW4 required

        ICW2 -> initial vector number:
            A0 = 1
            D7,D6,d5,D4,D3 = init vector (always multiple of 8)
            D2,D1,D0 = 0

        ICW3 -> interrupt lines to connect master to slave(only in case of cascade among multiple PICs 8259
            MASTER:
                A0 = 1
                D7,D6,D5,D4,D3,D2,D1,D0 = lines of interrupt = 1 there's a PIC 8259 | 0 there's a device

            SLAVE:
                A0 = 1
                D7,D6,D5,D4,D3 = 0
                D2,D1,D0 = number of interrupt line where the MASTER is connected

        ICW4 -> extra features on the PIC
            A0 = 1
            D7,D6,D5 = 0
            D4 = Special Fully Nested (Priority among slaves preserved)
            D3 = Buffered
            D2 = if D3 == 1 1 means this PIC is the MASTER, 0 means this PIC is the slave
            D1 = activate Automatic end of Interrupt which allows to reset the irq bit right after the request
            D0 = 1 emulate 8086 | 0 emulate 8085

        MAP MASTER TO 32 (right after the ISRs)
        MAP SLAVE TO 32 + 8 = 40
    */

	outPortB(0x20,0x11);	/* MASTER ICW1 */
	outPortB(0xA0,0x11); 	/* SLAVE ICW1 */

	outPortB(0x21,0x20);	/* MASTER ICW2 */
	outPortB(0xA1,0x28);	/* SLAVE ICW2 */

	outPortB(0x21,0x04);	/* MASTER ICW3 */
	outPortB(0xA1,0x02);	/* SLAVE ICW3 */

	outPortB(0x21,0x01);	/* MASTER ICW4 */
	outPortB(0xA1,0x01);	/* SLAVE ICW4 */

	outPortB(0x21,0x00);	/* MASTER END OF COM */
	outPortB(0xA1,0x00);	/* SLAVE END OF COM */
}

/*************************************
* Install IRQ routines in the system *
**************************************/
void irq_install()
{
	PIC8259_remap();

	/* Right after the 32 exceptions we register the IRQs */

	idt_setEntry(32,(unsigned)irq0,0x8,0x8E);
	idt_setEntry(33,(unsigned)irq1,0x8,0x8E);
	idt_setEntry(34,(unsigned)irq2,0x8,0x8E);
	idt_setEntry(35,(unsigned)irq3,0x8,0x8E);
	idt_setEntry(36,(unsigned)irq4,0x8,0x8E);
	idt_setEntry(37,(unsigned)irq5,0x8,0x8E);
	idt_setEntry(38,(unsigned)irq6,0x8,0x8E);
	idt_setEntry(39,(unsigned)irq7,0x8,0x8E);
	idt_setEntry(40,(unsigned)irq8,0x8,0x8E);
	idt_setEntry(41,(unsigned)irq9,0x8,0x8E);
	idt_setEntry(42,(unsigned)irq10,0x8,0x8E);
	idt_setEntry(43,(unsigned)irq11,0x8,0x8E);
	idt_setEntry(44,(unsigned)irq12,0x8,0x8E);
	idt_setEntry(45,(unsigned)irq13,0x8,0x8E);
	idt_setEntry(46,(unsigned)irq14,0x8,0x8E);
	idt_setEntry(47,(unsigned)irq15,0x8,0x8E);
}
