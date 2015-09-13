#ifndef __DISKPIO_H
#define __DISKPIO_H

#include <partitionTable.h>

#define IDE1_PORTS			0x1F0
#define IDE2_PORTS			0x170

#define ATA_DATA_PORT       0x0
#define ATA_ERROR_PORT      0x1
#define ATA_SECTOR_CNT_PORT 0x2
#define ATA_ADDR_LOW_PORT   0x3
#define ATA_ADDR_MID_PORT   0x4
#define ATA_ADDR_HI_PORT    0x5
#define ATA_TOP4LBA_PORT    0x6
#define ATA_DRIVE_SEL_PORT	0x6
#define ATA_CMD_PORT        0x7
#define ATA_STATUS_PORT		0x7

#define ATA_CTRL_PORT		0x206

#define ATA_MASTER			0xA0
#define ATA_SLAVE			0xB0

#define PIO_SOFTWARE_RESET	0x4

#define ATA_IDENTIFY_COMMAND 0xEC

#define ATA_BSY_BIT			0x80
#define ATA_ERR_BIT			0x01
#define ATA_DRQ_BIT			0x08

#define LBA28_READ_COMMAND 	0x20
#define LBA28_WRITE_COMMAND	0x30

void initDisk();
partitionTableEntry *getPartitionTable(unsigned int drive);
int readLBA28(int driveSel,int address,int size,unsigned char *data);
int writeLBA28(int driveSel,int address,int size,unsigned char *data);

#endif
