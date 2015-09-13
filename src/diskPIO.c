#include <diskPIO.h>
#include <partitionTable.h>
#include <system.h>
#include <screen.h>
#include <string.h>
#include <fat12.h>
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
		/* TODO: check file system before loading anything to the structures */
		memcpy((unsigned char *)pt[currentDrive] ,(buffer+PARTITION_TABLE_OFFSET), 16);
		FAT12Init(currentDrive);
	} else {
		memset((unsigned char *)pt[currentDrive], 0 , 16);
	}
}

static int ATA_getPort(int ide){
	if(ide == 1)
		return IDE1_PORTS;
	else if (ide == 2)
		return IDE2_PORTS;
	else
		return 0;
}

/* Return the partition table which can be read by the file system drivers */
partitionTableEntry *getPartitionTable(unsigned int drive)
{
	if(drive < 2)
		return pt[drive];
	else
		return NULL;
}

static void pio_400ns_delay(int port)
{
	inPortB(port + ATA_STATUS_PORT);
    inPortB(port + ATA_STATUS_PORT);
    inPortB(port + ATA_STATUS_PORT);
    inPortB(port + ATA_STATUS_PORT);
}

static void pio_reset(int port)
{
	outPortB(port + ATA_CTRL_PORT,PIO_SOFTWARE_RESET);
	outPortB(port + ATA_CTRL_PORT,0);
}

/* Find the available drives in the IDE buses */
void ATA_probe(int driveToProbe)
{
	char res = 0;
	char master_slave;
	int ide = ATA_getPort((driveToProbe / 2)+1);
	if(!ide)
		return;
	pio_reset(ide);
	master_slave = (driveToProbe % 2 == 0)? ATA_MASTER : ATA_SLAVE;

	outPortB(ide + ATA_DRIVE_SEL_PORT,master_slave);
	pio_400ns_delay(ide);
	outPortB(ide + ATA_SECTOR_CNT_PORT,0);
	outPortB(ide + ATA_ADDR_LOW_PORT,0);
	outPortB(ide + ATA_ADDR_MID_PORT,0);
	outPortB(ide + ATA_ADDR_HI_PORT,0);
	outPortB(ide + ATA_CMD_PORT,ATA_IDENTIFY_COMMAND);
	res = inPortB(ide + ATA_STATUS_PORT);
	if(res){
		/* The drive exists */
		while(inPortB(ide + ATA_STATUS_PORT) & ATA_BSY_BIT);

        /* is it an ata drive? if not, don't do anything */
        /* otherwise let's register the partition table */
		do{
			res = inPortB(ide + ATA_STATUS_PORT);
			if(res & ATA_ERR_BIT) return;
		}while(!(res & ATA_DRQ_BIT));
		drive[driveToProbe] = 1;
		initPartitionTable(driveToProbe);
	}
}

void initDisk()
{
	int i = 0;
	for(i=0;i<4;i++)
		ATA_probe(i);
}

/* Read sectors on a disk and put the bytes in the data buffer */

int readLBA28(int driveSel,int numblock,int count,unsigned char *data)
{
	int ide=ATA_getPort((driveSel/2)+1);
	char master_slave;
	char res;
	if(count <= 0 || data == NULL || ide == 0 || drive[driveSel] == 0)
		return -1;

    master_slave = (driveSel % 2 == 0)? ATA_MASTER : ATA_SLAVE;
	master_slave += 0x40;		/*0xE0->Master; 0xF0->Slave */

	outPortB(ide + ATA_DRIVE_SEL_PORT,master_slave|((numblock>>24)&0x0F));
	outPortB(ide + ATA_SECTOR_CNT_PORT,count);
	outPortB(ide + ATA_ADDR_LOW_PORT,(numblock & 0xFF));
	outPortB(ide + ATA_ADDR_MID_PORT,((numblock>>8) & 0xFF));
	outPortB(ide + ATA_ADDR_HI_PORT,((numblock>>16)& 0xFF));
	outPortB(ide + ATA_CMD_PORT,LBA28_READ_COMMAND);

	/* POLLING */
	pio_400ns_delay(ide);
	while(inPortB(ide + ATA_STATUS_PORT) & ATA_BSY_BIT);
	do{
		res = inPortB(ide + ATA_STATUS_PORT);
		if(res & ATA_ERR_BIT)
			return -1;
	}while(!(res & ATA_DRQ_BIT));

	count*=256;
	asm("cli\n"
		"mov %1,%%edi\n"
		"loopIN:\n"
		"insw\n"
		"loop loopIN\n"::"c"(count),"m"(data),"d"(ide));
	pio_400ns_delay(ide);
	asm("sti");
	return strlen((char *)data);
}

int writeLBA28(int driveSel,int numblock,int count,unsigned char *data)
{
    int ide=ATA_getPort((driveSel/2)+1);
    char master_slave;
    char res;
    if(count <= 0 || data == NULL || ide == 0 || drive[driveSel] == 0)
        return -1;

    master_slave = (driveSel % 2 == 0)? ATA_MASTER : ATA_SLAVE;
    master_slave += 0x40;       /*0xE0->Master; 0xF0->Slave */

    outPortB(ide + ATA_DRIVE_SEL_PORT,master_slave|((numblock>>24)&0x0F));
    outPortB(ide + ATA_SECTOR_CNT_PORT,count);
    outPortB(ide + ATA_ADDR_LOW_PORT,(numblock & 0xFF));
    outPortB(ide + ATA_ADDR_MID_PORT,((numblock>>8) & 0xFF));
    outPortB(ide + ATA_ADDR_HI_PORT,((numblock>>16)& 0xFF));
    outPortB(ide + ATA_CMD_PORT,LBA28_WRITE_COMMAND);

    /* POLLING */
    pio_400ns_delay(ide);
    while(inPortB(ide + ATA_STATUS_PORT) & ATA_BSY_BIT);
    do{
        res = inPortB(ide + ATA_STATUS_PORT);
        if(res & ATA_ERR_BIT)
            return -1;
    }while(!(res & ATA_DRQ_BIT));

    count*=256;

    asm("mov %1,%%esi\n"
        "loopOUT:\n"
        "outsw\n"
        "loop loopOUT\n"::"c"(count),"m"(data),"d"(ide));

    pio_400ns_delay(ide);
    asm("sti");
    return strlen((char *)data);

}
/*	int ide=(driveSel/2)+1;
	int cnt = 0;

	if(count < 0 || data == NULL || driveSel < 0 || driveSel > 3 || drive[driveSel] == 0)
		return -1;
	if(ide == 1){
		while((inPortB(IDE1_CMD_PORT) & 0x88) && (cnt < 10))
			cnt++;
		if(cnt == 10)
			return -1;

		cnt = 0;

		outPortB(IDE1_TOP4LBA_PORT,0xE0|(driveSel/2)|((numblock>>24)&0x0F));
		outPortB(IDE1_SECTOR_CNT_PORT,count);
		outPortB(IDE1_ADDR_LOW_PORT,(numblock & 0xFF));
		outPortB(IDE1_ADDR_MID_PORT,((numblock>>8) & 0xFF));
		outPortB(IDE1_ADDR_HI_PORT,((numblock>>16)& 0xFF));
		outPortB(IDE1_CMD_PORT,LBA28_WRITE_COMMAND);

//		while(!(inPortB(IDE1_CMD_PORT) & 0x8) && (cnt < 10))
//			cnt++;

//		if(cnt == 10)
//			return -1;

		count*=256;
		asm("mov %1,%%esi\n"
			"mov $0x1f0,%%dx\n"
			"loopOUT:\n"
			"outsw\n"
			"loop loopOUT\n"::"c"(count),"m"(data));

	}else if(ide==2){
		while((inPortB(IDE1_CMD_PORT) & 0x88) && (cnt < 10))
			cnt++;
		if(cnt == 10)
			return -1;

		cnt = 0;

		outPortB(IDE2_TOP4LBA_PORT,0xE0|(driveSel/2)|((numblock>>24)&0x0F));
		outPortB(IDE2_SECTOR_CNT_PORT,count);
		outPortB(IDE2_ADDR_LOW_PORT,(numblock & 0xFF));
		outPortB(IDE2_ADDR_MID_PORT,((numblock>>8) & 0xFF));
		outPortB(IDE2_ADDR_HI_PORT,((numblock>>16)& 0xFF));
		outPortB(IDE2_CMD_PORT,LBA28_WRITE_COMMAND);
		while(!(inPortB(IDE2_CMD_PORT) & 0x8) && (cnt < 10))
			cnt++;

		if(cnt == 10)
			return -1;

		count*=256;
		asm("mov %1,%%esi\n"
			"mov $0x170,%%dx\n"
			"loopOUT2:\n"
			"outsw\n"
			"loop loopOUT2\n"::"c"(count),"m"(data));
	}

	return count;
}
*/
