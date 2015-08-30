# TARGET DECLARATION
TARGET = i486-slitaz-linux-

#TOOLS DECLARATION
AS		= as
CC		= gcc
LD		= ld
DD		= dd
RM		= rm

#DIRECTORIES DECLARATION
OBJDIR = ./obj
SRCDIR = ./src
ASMDIR = ./asm
BINDIR = .
INCLUDEDIR = ./include

#FILES DECLARATION
LINKER_FILE = linker.ld

OBJS = VBR.o system.o string.o screen.o \
       GDT.o IDT.o ISR.o IRQ.o \
       timer.o keyboard.o cpu.o RTC.o \
       physicalMemoryManager.o \
       pageTableEntry.o pageDirectoryEntry.o \
       virtualMemoryManager.o \
       main.o \
       kTerm.o diskPIO.o fat12.o \
       file_system.o

KERNEL = kernel

vpath %.o   ${OBJDIR}
vpath %.h   ${INCLUDEDIR}
vpath %.inc ${ASMDIR}
vpath %.S   ${ASMDIR}
vpath %.c   ${SRCDIR}

#FLAGS DECLARATION
ASFLAGS = -I ${ASMDIR}/
CFLAGS = -Wall -Wextra -g -pedantic -finline-functions -nostdinc 
-ffreestanding -fno-builtin -I${INCLUDEDIR} -c #-O2 #optimizing flag...
LDFLAGS = -T ${LINKER_FILE}

all: kernel

%.o: %.S
	${TARGET}${AS} ${ASFLAGS} -o ${OBJDIR}/$@ $<

%.o: %.c
	${TARGET}${CC} ${CFLAGS} -o ${OBJDIR}/$@ $<

main.o: main.c ${filter-out main.o,${OBJS}} multiboot.h
	${TARGET}${CC} ${CFLAGS} -o ${OBJDIR}/main.o ${SRCDIR}/main.c

kernel: ${OBJS}
	${TARGET}${LD} ${LDFLAGS} -o ${BINDIR}/${KERNEL} ${OBJDIR}/*.o        

clean: clean_objs
	${RM} -f ${KERNEL}

clean_objs:
	${RM} -f ${OBJDIR}/*.o

VBR.o: 	     		GDT.inc IDT.inc

system.o:   		system.h

string.o:   		system.o string.h

GDT.o:      		system.o GDT.h

IDT.o:      		system.o IDT.h

IRQ.o:      		system.o IDT.o IRQ.h

screen.o:   		system.o string.o RTC.o screen.h header.h

RTC.o:      		system.o RTC.h

cpu.o:      		screen.o cpu.h

ISR.o:      		system.o screen.o IDT.o exception_messages.h ISR.h

timer.o:    		system.o screen.o IRQ.o timer.h

keyboard.o: 		screen.o system.o keyboard.h layouts.h IDT.o IRQ.o

physicalMemoryManager.o:screen.o system.o physicalMemoryManager.h multiboot.h

pageTableEntry.o: 	pageTableEntry.h

pageDirectoryEntry.o: 	pageDirectoryEntry.h

virtualMemoryManager.o: physicalMemoryManager.o pageTableEntry.o pageDirectoryEntry.o virtualMemoryManager.h

kTerm.o:                system.o screen.o string.o keyboard.o cpu.o physicalMemoryManager.o fat12.o kTerm.h

diskPIO.o:		system.o screen.o timer.o diskPIO.h

fat12.o:		system.o diskPIO.o screen.o file_system.o fat12.h BPB.h

file_system.o:          system.o file_system.h  
