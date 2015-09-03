.code16

.section .text
.globl _start

_start:
    jmp _main			
##############################################################################
#DESCRIPTOR TABLE
nameTable:           .ascii "JackBoot"           #must be 8 bytes!!!
bytesPerSector:      .word 512
sectorsPerCluster:   .byte 1
reservedBootSectors: .word 1			 #number of sectors without file allocation table
numberFATs:	     .byte 2                     #FAT12 has always 2 tables
numberEntriesRoot:   .word 224                   #at most 224 directories in root
totalSectors:        .word 2880                  #number of sectors in a floppy disk
mediaDescriptorByte: .byte 0xF0                  #byte pattern where:
                                                 #bit0 -> 0 single side       --     1 double side
                                                 #bit1 -> 0 9 sectors per FAT --     1 8 sectors per FAT
                                                 #bit2 -> 0 80 tracks         --     1 40 tracks
                                                 #bit3 -> 0 fixed disk (HD)   --     1 removable (floppy)
                                                 #bit 4 to 7 always 1
sectorsPerFAT:       .word 9
sectorsPerTrack:     .word 18                    #a floppy has always 18 sectors for each track
headsPerCylinder:    .word 2
hiddenSectors:       .long 0
sectorsBig:          .long 0
sectorsPerFATBig:    .long 0
flagsBig:            .word 0
FATVersionBig:       .word 0
rootDirBig:          .long 0
FSInfoLBABig:        .word 0
BackupVBRLBABig:     .word 0
unused1:             .long 0
unused2:             .word 0
driveNumber:         .byte 0
unused3:             .byte 0
GDTsignature:        .byte 0x29
serialNumber:        .long 0xa1a2a3a4
volumeLabel:         .ascii "FLOPPY DISK"             #length must be 11
FileSystem:          .ascii "FAT12   "                #length must be 8 
 
##############################################################################
#VARIABLES

fatLocation:
    .word 0

stage2name:
    .asciz "STAGE2  BIN"

loadedDrive:
    .byte 0

headStage2:
    .byte 0

trackStage2:
    .byte 0

sectorStage2:
    .byte 0

dataSector:
    .word 0

stage2Cluster:
    .word 0

###############################################################################
#FUNCIONS

print:
    pushw %ax
    addw $4,%sp
    popw %si
    subw $6,%sp
    movb $0xe,%ah			#print character function (see int 10h for details)
    movb $0,%bh                         #page 0

    printLoop:
        lodsb
        cmpb $0,%al
        je printDone
        int $0x10
        jmp printLoop
printDone:
    pop %ax
    ret

readSectors:
    addw $2,%sp
    popw %ax
    popw %cx
    subw $6,%sp
    
loadSectorsLoop:
    movw $0,%dx
    divw (sectorsPerTrack)
    inc %dl
    movb %dl,(sectorStage2)
    movw $0,%dx
    divw (headsPerCylinder)
    movb %dl,(headStage2)
    movb %al,(trackStage2)
    
readFloppy:
    movb $0x2,%ah			#read function
    movb %cl,%al			#all sectors
    movb (sectorStage2),%cl             #(the second one)
    movb (trackStage2),%ch              #on the first track
    movb (headStage2),%dh               #on the first head
    movb (loadedDrive),%dl              #on the floppy drive (usually is 0)

    int $0x13
    jc readFloppy
    
    ret

################################################################
# MAIN

_main:	
    cli
    movw $0x0,%ax
    movw %ax,%ds
    movw %ax,%es
    movw $0xFFFF,%ax
    movw %ax,%sp
    movw $0x0000,%ax
    movw %ax,%ss
    sti

    movb $0,%dh
    movw (loadedDrive),%dx

resetFloppy:
    mov $0,%ax
    mov $0,%dx
    int $0x13
    jc resetFloppy

#here we'll parse the FAT and we'll find the head/cylinder/sector of the stage2 bootloader

#get the size of root directory
    movw $0x20,%ax			#32 bytes of entry directory
    mulw (numberEntriesRoot)            #for 224 possible entries 
    divw (bytesPerSector)               #dividing by bytes per sector we'll find how many secotrs the root
    movw %ax,%cx                        #entry uses; store this value in cx (in FAT12 is 14)
    pushw %cx

#find the location of the root directory in the disk
# | bootloader | other reserved sectors(if there are....) | FAT1 | FAT2 | Root directory | files and directories|
    movb $0,%ah
    movb (numberFATs),%al	        #number of FAT
    mulw (sectorsPerFAT)               #total number of sectors used by both FATs
    addw (reservedBootSectors),%ax       #add the number of reserved sectors (in FAT12 is 19)
    pushw %ax

#store the address of the dataSector
    movw %ax,(dataSector)
    addw %cx,(dataSector)   
#In order to read correctly the disk we need to convert LBA to CHS
#LBA (Logical Block addressing) allows to us to see the disk as a contigous memory(without taking care about the geometry)
#CHS (Cylinder Head Sector) represent the disk as a geometrical structure (as it actually is...)

#The formula which converts CHS to LBA is:
#    LBA = (cluster - 2) * sectorsPerCluster

#The formula which converts LBA to CHS is:
#    Sector             S = (LBA % sectorsPerTrack) + 1
#    Head               H = (LBA / sectorsPerTrack) % (numberHeads)
#    Cylinder(or Track) C = (LBA) / (sectorsPerTrack * numberHeads)

#Now, ax = LBA root....let's change it into CHS
    movw $0x200,%bx
    push %ax
    mov $0x7c0,%ax
    mov %ax,%es
    pop %ax
    call readSectors
    addw $6,%sp
   
#now the root directory is loaded into memory
    
#find stage2.bin binary in the root directory
    movw (numberEntriesRoot),%cx	#number of allowed entries
    mov $0,%ax
    mov %ax,%es
    movw $0x7e00,%di			#starting from the root directory

findLoop:
    pushw %cx
    pushw %di    
    mov $11,%cx				#file names have at most 11 characters
    mov $stage2name,%si			#name of the file we are searching for
    cmpLoop:
        mov (%di),%al
        cmpb (%si),%al
        jne cmpFail
        inc %si
        inc %di
        loop cmpLoop
        
    jmp loadFAT
cmpFail:
    pop %di
    popw %cx
    addw $0x20,%di                      #otherwise let's try with the next entry
    loop findLoop
    cli
    hlt
    
loadFAT:
    popw %di
    popw %cx    
           
    addw $0x1A,%di
    movw (%di),%ax

    #here AX = 3

    movw %ax,(stage2Cluster)		#dx now stores the cluster start location
    movb (numberFATs),%al    
    movb $0,%ah
    mulw (sectorsPerFAT)
    mov %ax,%cx
    pushw %cx                           #total number of sectors to read when loading the FAT
    pushw (reservedBootSectors)   
    pushw %ax
    movw $0x7c0,%ax
    movw %ax,%es
    popw %ax
    movw $0x200,%bx			#load the FAT right after the bootloader code
    call readSectors
    addw $6,%sp

#now the FAT (and its backup) it's loaded in the memory, so we can find the cluster in
#which we have our second stage bootloader and load it and then....jump into it!
    movw $0x50,%ax
    movw %ax,%es
    mov $0,%bx
    pushw %bx
    
loadStage2:
   #LOAD stage2 code on 0x500
    movw (stage2Cluster),%ax
    
    popw %bx
    subw $2,%ax
    
    movb (sectorsPerCluster),%cl
    mov $0,%ch
    pushw %cx
    mulb %cl
    addw (dataSector),%ax
    
    pushw %ax
    call readSectors
    addw $6,%sp
    push %bx		#bx is the index in memory for the next sector

   #LOOK the FAT to see if there are other sectors to load

    movw (stage2Cluster),%ax
    movw %ax,%cx
    movw %ax,%dx
    shrw $1,%dx		#dx = dx/2		
    addw %dx,%cx         #cx = (clusterPos)* (3/2)   cx = 4

    #let's search next cluster in the FAT
    movw $0x7e00,%bx
    addw %cx,%bx	#next cluster in the FAT
    movw (%bx),%dx	#dx = FAT entry for the next cluster dx = 0xfff0
    
    test $1,%ax
    jnz oddCluster

evenCluster:
    andw $4095,%dx			#take low 12 bits 
    jmp stage2Continue

oddCluster:
    shrw $4,%dx                          #take high 12 bits dx becomes 0xfff 

stage2Continue:
    mov %dx,(stage2Cluster)
    cmp $0xff0,%dx			#end of file pattern in FAT12
    jb loadStage2
    
    jmp $0x50,$0x0
    
partition_table:
	.org 446
    #partition entries
    #1st partition
pt1_status:
    .byte 0x80
pt1_head_start:
    .byte 0
pt1_sector_start:
    .byte 1
pt1_cylinder_start:
    .byte 0
pt1_type_partition:
    .byte 0x1               #FAT12
pt1_head_end:
    .byte 1
pt1_sector_end:
    .byte 18
pt1_cylinder_end:
    .byte 79
pt1_num_sectors_MBR_to_partition:
    .long 0
pt1_num_sectors:
    .long 2880
	

boot_signature:
    .org 510
    .byte 0x55
    .byte 0xaa

fat1:
    .byte 0xf0
    .byte 0xff
    .byte 0xff
    .skip (512*9)-3
fat2:
    .byte 0xf0
    .byte 0xff
    .byte 0xff



