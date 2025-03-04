#include <GDT.h>
#include <IDT.h>
#include <IRQ.h>
#include <ISR.h>
#include <RTC.h>
#include <cpu.h>
#include <diskPIO.h>
#include <fat12.h>
#include <kTerm.h>
#include <keyboard.h>
#include <multiboot.h>
#include <physicalMemoryManager.h>
#include <screen.h>
#include <system.h>
#include <timer.h>
#include <varFunc.h>
#include <virtualMemoryManager.h>

void main(struct multiboot_info *mbootPtr) {
  int kernelSize = 0;

  /* In EDX the bootloader writes the size of the kernel */
  asm("mov %%edx,%0\n" : "=m"(kernelSize) :);
  /* Install tables for segmentation and interrupts */
  gdt_install();
  idt_install();
  isr_install();
  irq_install();
  timer_install();

  /* Install the Memory managers */
  PHYinit(mbootPtr, kernelSize);
  VMMinit();

  keyboard_install();
  initDisk();
  initScreen();

  /* Enable interrupts and execute the terminal */
  __asm__ __volatile__("sti");

  kTerm();

  print("\r\nSystem quit!\r\n");
}
