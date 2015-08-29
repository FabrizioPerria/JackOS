#ifndef __FAT12_H
#define __FAT12_H

#include <file_system.h>

/*File Entry Format */
struct fat12Entry{
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
	int sectorSize;
    int fatPosition;
    int numRootEntries;
    int rootPosition;
    int rootSize;
    int fatSize;
    int fatEntrySize;
}__attribute__((packed));

void FAT12Read(FILE_PTR file,unsigned char *buffer,unsigned int length);
void FAT12Init();
FILE FAT12Directory(const char *dirName,FILE* folder);
FILE FAT12Open(const char *fileName);
void FAT12Close(FILE_PTR file);
void FAT12Mount();

#endif
