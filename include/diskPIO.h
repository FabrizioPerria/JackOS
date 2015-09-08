#ifndef __DISKPIO_H
#define __DISKPIO_H

#include <partitionTable.h>

#define IDE1_DATA_PORT       0x1F0
#define IDE1_ERROR_PORT      0x1F1
#define IDE1_SECTOR_CNT_PORT 0x1F2
#define IDE1_ADDR_LOW_PORT   0x1F3
#define IDE1_ADDR_MID_PORT   0x1F4
#define IDE1_ADDR_HI_PORT    0x1F5
#define IDE1_TOP4LBA_PORT    0x1F6
#define IDE1_CMD_PORT        0x1F7

#define IDE2_DATA_PORT       0x170
#define IDE2_ERROR_PORT      0x171
#define IDE2_SECTOR_CNT_PORT 0x172
#define IDE2_ADDR_LOW_PORT   0x173
#define IDE2_ADDR_MID_PORT   0x174
#define IDE2_ADDR_HI_PORT    0x175
#define IDE2_TOP4LBA_PORT    0x176
#define IDE2_CMD_PORT        0x177

#define LBA28_READ_COMMAND 0x20
#define LBA28_WRITE_COMMAND 0x30

void initDisk();
partitionTableEntry *getPartitionTable(unsigned int drive);
int readLBA28(int driveSel,int address,int size,unsigned char *data);
int writeLBA28(int driveSel,int address,int size,unsigned char *data);

#endif
