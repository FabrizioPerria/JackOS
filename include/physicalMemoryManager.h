#ifndef __PHYSICALMEMORYMANAGER_H
#define __PHYSICALMEMORYMANAGER_H

#include <multiboot.h>

#define PHY_MANAGER_BLOCKS_PER_BYTE 8
#define PHY_MANAGER_BLOCK_SIZE 4096
#define PHY_MANAGER_BLOCK_ALIGNMENT PHY_MANAGER_BLOCK_SIZE

void mmap_set(int bit);
void mmap_unset(int bit);

unsigned long getMemorySize(struct multiboot_info *mbootPtr);
int phy_manager_init(int memorysize, int *bitmap_Addr);
int phy_get_free_block(int numblocks);
void phy_manager_init_region(int *base, int size);
void phy_manager_deinit_region(int *base, int size);
void *phy_manager_alloc_blocks(int numblocks);
void phy_manager_dealloc_blocks(void *blockPtr, int numblocks);
void PHYinit(struct multiboot_info *mbootPtr, int kernelSize);

#endif
