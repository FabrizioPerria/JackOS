#ifndef __PARTITIONTABLE_H
#define __PARTITIONTABLE_H

#define FAT32 0xB

typedef struct _partitionTableEntry{
    char status;
    char start_head;
    char start_sector;
    char start_cylinder;
    char type;
    char end_head;
    char end_sector;
    char end_cylinder;
    int  LBA_absolute_start_sector;
    int  number_sectors;
}partitionTableEntry;

#endif
