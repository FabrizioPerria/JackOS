.code32

.set MBOOT_ALIGN, 1<<0
.set MBOOT_MAGIC, 0x1BADB002
.set MBOOT_MEMORY, 1<<1
.set MBOOT_AOUT_KLUDGE, 1<<16
.set MBOOT_FLAGS, (MBOOT_ALIGN | MBOOT_MEMORY)
.set MBOOT_CHECKSUM, -(MBOOT_MAGIC + MBOOT_FLAGS)

.section .text
.global _start

_start:
    movl $sys_stack,%esp
    jmp resume

.align 4
.long MBOOT_MAGIC
.long MBOOT_FLAGS
.long MBOOT_CHECKSUM    

resume:
    push %ebx
    call main

quit:
    cli
    hlt

# GDT declaration
.include "GDT.inc"

# Interrupt Service Routines
.include "IDT.inc"

# Stack declaration 
.section .bss
    .skip 32768
sys_stack:
