# TARGET DECLARATION
TARGET = x86_64-elf-

#TOOLS DECLARATION
AS		= as
CC		= gcc
LD		= ld
DD		= dd
RM		= rm
OBJCOPY = llvm-objcopy
DD		= dd

#DIRECTORIES DECLARATION
OBJDIR = ./obj
SRCDIR = ./src
ASMDIR = ./asm
BINDIR = .
INCLUDEDIR = ./include
FLOPPYSRCDIR = ./newFloppy

#FILES DECLARATION
LINKER_FILE = linker.ld
ZERO   = /dev/zero
IMAGE  = floppy.img
MOUNT_POINT = /mnt/floppy
FLOPPY_STAGE1_BIN   = ${FLOPPYSRCDIR}/stage1.bin
FLOPPY_STAGE2_BIN   = ${FLOPPYSRCDIR}/stage2.bin
FLOPPY_STAGE1_OBJ   = ${FLOPPYSRCDIR}/stage1.o
FLOPPY_STAGE2_OBJ   = ${FLOPPYSRCDIR}/stage2.o


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
KERNELSYM = kernel.sym

vpath %.o   ${OBJDIR}
vpath %.h   ${INCLUDEDIR}
vpath %.inc ${ASMDIR}
vpath %.S   ${ASMDIR}
vpath %.c   ${SRCDIR}

#FLAGS DECLARATION
ASFLAGS = -I ${ASMDIR}/ --32
CFLAGS = -g -Wall -Wextra -v -pedantic -finline-functions -nostdinc -ffreestanding -fno-builtin -I${INCLUDEDIR} -c -m32 -mno-red-zone
LDFLAGS = -T ${LINKER_FILE} -m elf_i386
OBJCOPYFLAGS = --only-keep-debug

all: kernel

floppy:
	${RM} -f ${IMAGE}.dmg
	${TARGET}${AS} ${FLOPPYSRCDIR}/stage1.s -o ${FLOPPY_STAGE1_OBJ}
	${TARGET}${AS} ${FLOPPYSRCDIR}/stage2.s -o ${FLOPPY_STAGE2_OBJ}
	${TARGET}${LD} -Ttext=0x7c00 --oformat=binary ${FLOPPY_STAGE1_OBJ} -o ${FLOPPY_STAGE1_BIN}
	${TARGET}${LD} -Ttext=0x0000 --oformat=binary ${FLOPPY_STAGE2_OBJ} -o ${FLOPPY_STAGE2_BIN}

	hdiutil create -size 1440k -fs MS-DOS -layout NONE -o ${IMAGE}.dmg

	@DEVICE=$(shell hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount ${IMAGE}.dmg | grep /dev/disk | awk '{print $$1}'); \
		echo "Mounted device: $$DEVICE"; \
		sudo mkdir -p /Volumes/Floppy; \
		sudo mount -t msdos $$DEVICE /Volumes/Floppy; \
		sudo cp ${FLOPPY_STAGE2_BIN} /Volumes/Floppy/; \
		sudo touch /Volumes/Floppy/file.txt; \
		sudo bash -c "dmesg > /Volumes/Floppy/file.txt"; \
		sudo bash -c "echo FINITO >> /Volumes/Floppy/file.txt"; \
		sudo mkdir -p /Volumes/Floppy/folder; \
		sudo bash -c "echo 'Se stampa questo, i path funzionano' > /Volumes/Floppy/folder/print.txt"; \
		sudo umount /Volumes/Floppy; \
		sudo rmdir /Volumes/Floppy; \
		hdiutil detach $$DEVICE; \
		${RM} -f ${FLOPPY_STAGE1_OBJ} ${FLOPPY_STAGE2_OBJ} ${FLOPPY_STAGE1_BIN} ${FLOPPY_STAGE2_BIN}
	
%.o: %.S
	${TARGET}${AS} ${ASFLAGS} -o ${OBJDIR}/$@ $<

%.o: %.c
	${TARGET}${CC} ${CFLAGS} -o ${OBJDIR}/$@ $<

main.o: main.c ${filter-out main.o,${OBJS}} multiboot.h
	${TARGET}${CC} ${CFLAGS} -o ${OBJDIR}/main.o ${SRCDIR}/main.c

kernel: ${OBJS}
	${TARGET}${LD} ${LDFLAGS} -o ${BINDIR}/${KERNEL} ${OBJDIR}/*.o
	${OBJCOPY} ${OBJCOPYFLAGS} ${BINDIR}/${KERNEL} ${BINDIR}/${KERNELSYM}

clean: clean_objs
	${RM} -f ${KERNEL}
	${RM} -f ${KERNELSYM}

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

diskPIO.o:		system.o screen.o string.o timer.o diskPIO.h

fat12.o:		system.o diskPIO.o screen.o file_system.o RTC.o fat12.h BPB.h

file_system.o:          system.o string.o file_system.h  
