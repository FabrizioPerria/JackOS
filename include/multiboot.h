#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H

struct multiboot_info
{
    int flags;
    int memoryLow;
    int memoryHigh;
    int bootDevice;
    int cmdLine;
    int modsCount;
    int modsAddress;
    int syms0;
    int syms1;
    int syms2;
    int sysm3;
    int mmapLength;
    int mmapAddress;
    int drivesLength;
    int drivesAddress;
    int configTable;
    int bootloaderName;
    int apmTable;
    int vbeControlInfo;
    int vbeModeInfo;
    short vbeMode;
    short vbeIfaceSegment;
    short vbeIfaceOffset;
    short vbeIfaceLength;
};

struct multiboot_mmap_entry
{
    int size;
    int addressLow;
    int addressHigh;
    int lengthLow;
    int lengthHigh;
    int type;
} __attribute__((packed));
#endif
