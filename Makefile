# TARGET DECLARATION
TARGET = i486-slitaz-linux-

#TOOLS DECLARATION
AS		= as
CC		= gcc
LD		= ld
DD		= dd
RM		= rm
OBJCOPY = objcopy
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
ASFLAGS = -I ${ASMDIR}/
CFLAGS = -g -Wall -Wextra -pedantic -finline-functions -nostdinc -ffreestanding -fno-builtin -I${INCLUDEDIR} -c #-O2
LDFLAGS = -T ${LINKER_FILE}
OBJCOPYFLAGS = --only-keep-debug

all: kernel

floppy:
	${RM} -f ${IMAGE}
	${TARGET}${AS} ${FLOPPYSRCDIR}/stage1.s -o ${FLOPPY_STAGE1_OBJ}
	${TARGET}${AS} ${FLOPPYSRCDIR}/stage2.s -o ${FLOPPY_STAGE2_OBJ}
	${TARGET}${LD} -Ttext=0x7c00 --oformat=binary ${FLOPPY_STAGE1_OBJ} -o ${FLOPPY_STAGE1_BIN}
	${DD} if=${ZERO} of=${IMAGE} bs=512 count=2880
	losetup /dev/loop0 ${IMAGE}
	${DD} if=${FLOPPY_STAGE1_BIN} of=${IMAGE} conv=notrunc
	${TARGET}${LD} --oformat=binary ${FLOPPY_STAGE2_OBJ} -o ${FLOPPY_STAGE2_BIN}
	mount /dev/loop0 /mnt/floppy -t msdos -o "fat=12"
	cp ${FLOPPY_STAGE2_BIN} /mnt/floppy
	touch /mnt/floppy/file.txt
	ls > /mnt/floppy/file.txt
	mkdir /mnt/floppy/folder
	echo "Se stampa questo, i path funzionano" > /mnt/floppy/folder/print.txt
	umount /mnt/floppy
	losetup -d /dev/loop0
	${RM} -f ${FLOPPY_STAGE1_OBJ}
	${RM} -f ${FLOPPY_STAGE2_OBJ}
	${RM} -f ${FLOPPY_STAGE1_BIN}
	${RM} -f ${FLOPPY_STAGE2_BIN}

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

diskPIO.o:		system.o screen.o timer.o diskPIO.h

fat12.o:		system.o diskPIO.o screen.o file_system.o fat12.h BPB.h

file_system.o:          system.o file_system.h  
