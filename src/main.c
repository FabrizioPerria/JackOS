#include <varFunc.h>
#include <system.h>
#include <screen.h>
#include <GDT.h>
#include <IDT.h>
#include <ISR.h>
#include <IRQ.h>
#include <timer.h>
#include <keyboard.h>
#include <multiboot.h> 
#include <physicalMemoryManager.h>
#include <virtualMemoryManager.h>
#include <cpu.h>
#include <RTC.h>
#include <diskPIO.h>
#include <kTerm.h>
#include <fat12.h>

void main(struct multiboot_info *mbootPtr){
	int kernelSize = 0;

	/* In EDX the bootloader writes the size of the kernel */
	asm("mov %%edx,%0\n" : "=m" (kernelSize):);
	/* Install tables and interrupts */
	gdt_install();
	idt_install();
	isr_install();
	irq_install();
	timer_install();

	/* Install the Memory managers */
	PHYinit(mbootPtr,kernelSize);
	VMMinit();

	keyboard_install();
	initDisk();
	initScreen();
	//FAT12Init();

	/* Enable interrupts and execute the terminal */
	__asm__ __volatile__ ("sti");

	kTerm();

	print("\r\nSystem quit!\r\n");
}
