#include <diskPIO.h>
#include <partitionTable.h>
#include <system.h>
#include <screen.h>
#include <timer.h>

#define BOOT_SECTOR 0
#define PARTITIONS_PER_DISK 4
#define DISKS_PER_IDE 2
#define NUM_IDE_CHANNELS 2
#define NUM_DISKS (DISKS_PER_IDE * NUM_IDE_CHANNELS)

#define PARTITION_TABLE_OFFSET 446

static unsigned char drive[NUM_DISKS]={0};

partitionTableEntry pt[NUM_DISKS][PARTITIONS_PER_DISK];

/* Initialize the pt matrix with the partition table of the all connected hard disks */

void initPartitionTable(int currentDrive)
{
	unsigned char buffer[512];
    /* read the first sector of the disk
       and find the partition table starting 
       from byte 446 *address 0x1BE) */

	if(readLBA28(currentDrive,BOOT_SECTOR,1,buffer) > 0){
		/* TODO: what about FAT12? (Floppy don't have any MBR and partition table....) */
		memcpy((unsigned char *)pt[currentDrive] ,(buffer+PARTITION_TABLE_OFFSET), 16);
	} else {
		memset((unsigned char *)pt[currentDrive], 0 , 16);
	}
}

/* Return the partition table which can be read by the file system drivers */
partitionTableEntry *getPartitionTable(unsigned int drive)
{
	if(drive < 2)
		return pt[drive];
	else
		return NULL;
}

/* Find the available drives in the IDE buses */
void initDisk()
{
	outPortB(IDE1_ADDR_LOW_PORT,0x88);
	if(inPortB(IDE1_ADDR_LOW_PORT) == 0x88){
		outPortB(IDE1_TOP4LBA_PORT,0xA0);

		if(inPortB(IDE1_CMD_PORT) & 0x40){
			drive[0]=1;
			initPartitionTable(0);
		}

		outPortB(IDE1_TOP4LBA_PORT,0xB0);

		if(inPortB(IDE1_CMD_PORT) & 0x40){
			drive[1] = 1;
			initPartitionTable(1);
		}
	}

	outPortB(IDE2_ADDR_LOW_PORT,0x88);
	if(inPortB(IDE2_ADDR_LOW_PORT) == 0x88){
		outPortB(IDE2_TOP4LBA_PORT,0xA0);

		if(inPortB(IDE2_CMD_PORT) & 0x40){
			drive[2] = 1;
			initPartitionTable(2);
		}

		outPortB(IDE2_TOP4LBA_PORT,0xB0);

		if(inPortB(IDE2_CMD_PORT) & 0x40){
			drive[3] = 1;
			initPartitionTable(3);
		}
	}
}

/* Read sectors on a disk and put the bytes in the data buffer */

int readLBA28(int driveSel,int numblock,int count,unsigned char *data)
{
	int ide=(driveSel/2)+1;
	int cnt = 0;
	if(count <= 0 || data == NULL || driveSel < 0 || driveSel > 3 || drive[driveSel] == 0)
		return -1;
	if(ide ==1){
		while((inPortB(IDE1_CMD_PORT) & 0x88)!=0);

		outPortB(IDE1_TOP4LBA_PORT,0xE0|(driveSel%2)|((numblock>>24)&0x0F));
		outPortB(IDE1_SECTOR_CNT_PORT,count);
		outPortB(IDE1_ADDR_LOW_PORT,(numblock & 0xFF));
		outPortB(IDE1_ADDR_MID_PORT,((numblock>>8) & 0xFF));
		outPortB(IDE1_ADDR_HI_PORT,((numblock>>16)& 0xFF));
		outPortB(IDE1_CMD_PORT,LBA28_READ_COMMAND);

		/* Read the register 5 times to make sure that the error is not related to the latency */
		while(!(inPortB(IDE1_CMD_PORT) & 0x8) && (cnt < 5))
			cnt++;

		if(cnt == 5)
			return 0;

		count*=256;
		asm("mov %1,%%edi\n"
			"mov $0x1f0,%%dx\n"
			"loopIN:\n"
			"insw\n"
			"loop loopIN\n"::"c"(count),"m"(data));

	}else if(ide==2){
		outPortB(IDE2_TOP4LBA_PORT,0xE0|(driveSel%2)|((numblock>>24)&0x0F));
		outPortB(IDE2_SECTOR_CNT_PORT,count);
		outPortB(IDE2_ADDR_LOW_PORT,(numblock & 0xFF));
		outPortB(IDE2_ADDR_MID_PORT,((numblock>>8) & 0xFF));
		outPortB(IDE2_ADDR_HI_PORT,((numblock>>16)& 0xFF));
		outPortB(IDE2_CMD_PORT,LBA28_READ_COMMAND);

        /* Read the register 5 times to make sure that the error is not related to the latency */
		while(!(inPortB(IDE2_CMD_PORT) & 0x8) && (cnt < 5))
			cnt++;

		if(cnt == 5)
			return 0;

		count*=256;
		asm("mov %1,%%edi\n"
			"mov $0x170,%%dx\n"
			"rep insw\n"::"c"(count),"m"(data));
    }

	return count;
}

int writeLBA28(int driveSel,int numblock,int count,unsigned char *data)
{
	int ide=(driveSel%2)+1;
	if(count < 0 || data == NULL || driveSel < 0 || driveSel > 3 || drive[driveSel] == 0)
		return -1;
	if(ide ==1){
		outPortB(IDE1_TOP4LBA_PORT,0xE0|driveSel|((numblock>>24)&0x0F));
		outPortB(IDE1_SECTOR_CNT_PORT,count);
		outPortB(IDE1_ADDR_LOW_PORT,(numblock & 0xFF));
		outPortB(IDE1_ADDR_MID_PORT,((numblock>>8) & 0xFF));
		outPortB(IDE1_ADDR_HI_PORT,((numblock>>16)& 0xFF));
		outPortB(IDE1_CMD_PORT,LBA28_WRITE_COMMAND);
		while(!(inPortB(IDE1_CMD_PORT) & 0x40));
		count*=256;
		asm("mov %1,%%edi\n"
			"mov $0x1f0,%%dx\n"
			"rep outsw\n"::"c"(count),"m"(data));

	}else if(ide==2){
		outPortB(IDE2_TOP4LBA_PORT,0xE0|driveSel|((numblock>>24)&0x0F));
		outPortB(IDE2_SECTOR_CNT_PORT,count);
		outPortB(IDE2_ADDR_LOW_PORT,(numblock & 0xFF));
		outPortB(IDE2_ADDR_MID_PORT,((numblock>>8) & 0xFF));
		outPortB(IDE2_ADDR_HI_PORT,((numblock>>16)& 0xFF));
		outPortB(IDE2_CMD_PORT,LBA28_WRITE_COMMAND);
		while(!(inPortB(IDE2_CMD_PORT) & 0x40));
		count*=256;
		asm("mov %1,%%edi\n"
			"mov $0x170,%%dx\n"
			"rep outsw\n"::"c"(count),"m"(data));
	}

	return count;
}
