#ifndef __FAT32_H
#define __FAT32_H

#define FAT32 0xB

/* Long File name Format */
struct fat32LFNEntry{
	char sequence;
	char name1[10];	 /* 1st to 5th character */
	char attribute;      /* always 0xf */
	char type;           /* always 0 */
	char checkSum;       /*checksum short name */
	char name2[12];      /* 5th to 11th character */
	short starting_cluster; /* always 0 */
	char name3[4];        /* 12th to 13th character */
}__attribute__((packed));

/*Short File name Format */
struct fat32SFNEntry{
	char name[8];
	char extension[3];
	char attribute;
	int  reserved1;
	int  reserved2;
	short reserved3;
	short time;
	short date;
	short startingCluster;
	int fileSize;
}__attribute__((packed));

struct MOUNT_INFO{
	int numSectors;
	int fatPosition;
	int fatSize;
	int rootPosition;
	int rootSize;
}__attribute__((packed));

int mountFAT32partition(unsigned int drive,unsigned int partition);
void unmountFAT32partition(unsigned int partition);
int readFAT32partition(unsigned int partition,char *fileName,char *fileBuffer);
void fsInit();

#endif
