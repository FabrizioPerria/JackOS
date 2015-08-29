#ifndef __BPB_H
#define __BPB_H

/* BIOS PARAMETER BLOCK */

struct bpb{
    unsigned short jmpCmd;
    unsigned char  nullChar;
    unsigned char  id[8];
    unsigned short bytesPerSector;
    unsigned char  sectorsPerCluster;
    unsigned short reservedBootSectors;
    unsigned char  numberFATs;
    unsigned short maxNumberEntriesRoot;
    unsigned short totalSectors;
    unsigned char  mediaDescriptorByte;
    unsigned short sectorsPerFAT;
    unsigned short sectorsPerTrack;
    unsigned short headsPerCylinder;
    unsigned int   hiddenSectors;
    unsigned int   extendedTotalSectors;
    unsigned int   extendedSectorsPerFAT;
    unsigned short extendedFlags;
    unsigned short extendedFATVersion;
    unsigned int   extendedRootDirectoryCluster;
    unsigned short extendedFSInfoLBA;
    unsigned short extendedBackupVBRLBA;
    unsigned int   notUsed1;
    unsigned short notUsed2;
    unsigned char  driveNumber;
    unsigned char  notUsed3;
    unsigned char  GDTSignature;
    unsigned int   serialNumber;
    unsigned char  volumeLabel[11];
    unsigned char  FileSystem[8];
}__attribute__((packed));

#endif
