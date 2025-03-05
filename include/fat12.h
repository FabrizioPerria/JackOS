#ifndef __FAT12_H
#define __FAT12_H

#include <file_system.h>

#define FAT12_NAME_LENGTH 8
#define FAT12_EXT_LENGTH 3
#define FAT12_SECTOR_SIZE 512
#define FAT12_FAT_SECTORS 9

/*File Entry Format */
struct fat12Entry
{
    char name[FAT12_NAME_LENGTH];
    char extension[FAT12_EXT_LENGTH];
    char attribute;
    int reserved1;
    int reserved2;
    short reserved3;
    short time;
    short date;
    short startingCluster;
    int fileSize;
} __attribute__ ((packed));

struct MOUNT_INFO
{
    int numSectors;
    int sectorSize;
    int fatPosition;
    int numRootEntries;
    int rootPosition;
    int rootSize;
    int fatSize;
    int fatEntrySize;
    unsigned char fat[FAT12_FAT_SECTORS * FAT12_SECTOR_SIZE];
} __attribute__ ((packed));

int FAT12Write (FILE_PTR file, unsigned char* buffer, unsigned int length);
int FAT12Read (FILE_PTR file, unsigned char* buffer, unsigned int length);
void FAT12Init (int drive);
void FAT12Directory (int drive, char* dirName, FILE* folder, FILE* f);
void FAT12Open (char* fileName, char mode, FILE_PTR file);
void FAT12Close (FILE_PTR file);
int FAT12Mount (int drive);
void FAT12Remove (char* fileName);
void FAT12List (FILE* folder, FILE chain[]);
void FAT12Create (char* fileName, FILE* folder, FILE* file);

#endif
