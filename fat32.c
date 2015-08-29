#include <fat32.h>
#include <string.h>
#include <diskPIO.h>
#include <system.h>
#include <partitionTable.h>
#include <BPB.h>
#include <screen.h>
#include <file_system.h>

/* Disk layout (| means 1 sector)
   | MBR + Partition Table        |
   | FS Info                      |
   --------------------------------
   | VBR+Bios Parameter Block     |    Start partition
   | FS Info                      |
   | Empty                        |
   | VBR+Bios Parameter Block copy|
   | FS Info copy                 |
   | Empty                        |
   --------------------------------
   | FAT(#sec specified in BPB)   |
   | FAT copy                     |
   --------------------------------
   | Root directory + data        |
   ================================

*/

typedef struct _mountedPartitionTableEntry{
    char present;
    int drive;
    partitionTableEntry *pte;
    MOUNT_INFO *info;
}mountedPartitionTableEntry;

static mountedPartitionTableEntry mountedPartitions[8];

static partitionTableEntry *getFAT32partition(unsigned int driveSelected,unsigned int partition)
{
    partitionTableEntry *pt=getPartitionTable(driveSelected);
    if(partition > 4 || driveSelected > 4)
        return NULL;

    if((pt[partition].status & 0x80) && (pt[partition].type == FAT32)){
            return &pt[partition];
    }

    return NULL;
}

/* Returns the index of the partition mounted in the mounted partition table */
int mountFAT32partition(unsigned int drive,unsigned int partition){
    partitionTableEntry *pte=getFAT32partition(drive,partition);
    int LBA_partition = pte->LBA_absolute_start_sector;
    char buffer[512];
    struct bpb params[1];

    memset(buffer,0,512);
    
    readLBA28(drive,LBA_partition,1,buffer);
    
    memcpy((unsigned char *)params,buffer,sizeof(struct bpb));

    if(pte != NULL){
        for(i=0;i<8;i++){
            if(mountedPartitions[i].present == 0){
                
                mountedPartitions[i].present=1;
                mountedPartitions[i].drive=drive;
                mountedPartitions[i].pte=pte;
                (mountedPartitions[i].info)->rootPosition=LBA_partition + params-> reservedBootSectors + 
                                                (params-> numberFATs * params -> extendedSectorsPerFAT)+
                                                ((params-> extendedRootDirectoryCluster - 2)* 
                                                (params->sectorsPerCluster)); 
                (mountedPartitions[i].info)->fatPosition=LBA_partition + params-> reservedBootSectors + 
                                                ((params-> extendedRootDirectoryCluster - 2)* 
                                                (params->sectorsPerCluster)); 
                print("%d, %d\r\n",(mountedPartitions

                return i;
            }
        }
    }
    return -1;
} 

/* Delete the partition Entry from the mounted partition table */
void unmountFAT32partition(unsigned int partition)
{
    memset((unsigned char*)(mountedPartitions+partition),0,sizeof(mountedPartitionTableEntry));
}

int readFAT32partition(unsigned int partition,char *fileName, char *fileBuffer)
{
    /* on LBA_partition we put the number of sectors before the beginning of the partition */
    unsigned char buffer[512];
    static char nameBuffer[256];
    int LBA_root=0;
    int LFN_sequence=-1;
    int index=0,i=0;
    struct bpb params[1];
    struct fat32LFNEntry LFNEntry[1];
    int drive = mountedPartitions[partition].drive;
    int LBA_partition = (mountedPartitions[partition].pte)->LBA_absolute_start_sector;
    memset(buffer,0,512);
    
    readLBA28(drive,LBA_partition,1,buffer);
    
    memcpy((unsigned char *)params,buffer,sizeof(struct bpb));
    
    LBA_root=LBA_partition + params-> reservedBootSectors + 
             (params-> numberFATs * params -> extendedSectorsPerFAT)+
             ((params-> extendedRootDirectoryCluster - 2)* (params->sectorsPerCluster));    


    //while a new root sector exists
    while(LBA_root != -1){
        memset(buffer,0,512);
        readLBA28(drive,LBA_root,1,buffer);
        index=0;
        /* While index points to a root Entry */
        while(index < 512){
            if(buffer[index+11] == 0xf){
                /*Long file name */
                memcpy((unsigned char *)LFNEntry,(buffer+index),sizeof(struct fat32LFNEntry));

                if(LFN_sequence < LFNEntry->sequence - 0x40){
                    LFN_sequence = (LFNEntry->sequence - 0x41);
                    memset((unsigned char*)nameBuffer,0,256);
                } else {
                    LFN_sequence = LFNEntry->sequence;
                }

                strcpyWord((nameBuffer+5+6+(13 * LFN_sequence)),LFNEntry->name3,2);
                strcpyWord((nameBuffer+5+(13 * LFN_sequence)),LFNEntry->name2,6);
                strcpyWord((nameBuffer+(13 * LFN_sequence)),LFNEntry->name1,5);
                
                if(strncmp(fileName,nameBuffer,strlen(fileName))){
                    print("NOT FOUND!");
                    index+=32;
                } else {
                    print("FOUND! ");
                    return 0;
                }
            }else{
            /* Short File Name */
                print("SHORT");
                index+=32;
            }
        }
        print("NONE");
    } 

    //get the file size and allocate a buffer in the virtual memory
    //get the address of the file in the disk
    //read the entire file using the chain and put into the buffer
    //return the bytes read
    return 0;
}

void fsInit(){
    FILESYSTEM fs;
    strcpy(fs->name,"FAT 32",7);
    fs.open=openFAT32;
    fs.close=closeFAT32;
    fs.read=readFAT32;
    registerFS(fs);
    mountFAT32();
}    

//TODO locate file or directory in the disk starting from the name



//TODO find free spot at partition table:

//TODO write a buffer in the disk
    //find free spot for the FAT32 chain 
    //write the header in the root directory
    //write the buffer using the chain on the table
    
//TODO read a file from the disk
    //find the header file in the root directory
    //read the starting address of the file
    //read the file using the chain in the partition table
