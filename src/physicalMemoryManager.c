#include <physicalMemoryManager.h>
#include <screen.h>
#include <system.h> 

/* Physical memory manager implementation */

static int memorySize=0;
static int usedBlocks=0;
static int maxBlocks=0;
static int *bitmapPtr=0;

/* Initialize the entire memory and set it as used memory by default;
   the virtual memory manager will set free the available regions */
int phy_manager_init(int memory_size,int *bitmapAddr)
{
	if(memorySize < 0)
		return -1;

	memorySize= memory_size;
	bitmapPtr = bitmapAddr;
	maxBlocks= (memorySize*1024)/PHY_MANAGER_BLOCK_SIZE;
	usedBlocks=maxBlocks;

/* set the whole memory to 0xFF means the every block of the memory is used */
	memset((unsigned char*)bitmapPtr,0xff,(maxBlocks/PHY_MANAGER_BLOCKS_PER_BYTE));

	return 0;
}

/* Set a block as used in the bitmap */
inline void mmap_set(int bit)
{
	if(bit > 0)
		bitmapPtr[bit/32] |= (1<<(bit%32));
}

/* Set a block as free in the bitmap */
inline void mmap_unset(int bit)
{
	if(bit > 0)
		bitmapPtr[bit/32] &= ~(1<<(bit%32));
}

inline int mmap_test(int bit)
{
	if (bit > 0)
		return (bitmapPtr[bit/32] & (1<<(bit%32)));
	else
		return -1;
}

/* Get the length of the memory seen by the BIOS */
unsigned long getMemorySize(struct multiboot_info *ptr)
{
	if(ptr == NULL)
		return 0;
	else
		return ptr->memoryLow + ptr->memoryHigh;
}

/*Get the pointer to a countiguous set of <blocks> free blocks */
int phy_get_free_block(int blocks)
{
	int i=0,j=0,k=0;
	if(blocks < 0)
		return -1;
	for(i=0;i<maxBlocks/32;i++){
		if(bitmapPtr[i] != (int)(0xffffffff)){
			for(j=0;j<32;j++){
				k=0;
				while(k<blocks){
					if((!(bitmapPtr[i] & (1<<(j+k))))){
						k++;
					}else break;
				}
				if(k==blocks)
					return (i*32)+j;
			}
		}
	}
	return -1;
}

/* Initialize a region described by its base address and its length (info passed by the 
BIOS through multiboot structure) */
void phy_manager_init_region(int *base,int size)
{
	int align = (int)(base)/PHY_MANAGER_BLOCK_SIZE;
	int blocks = (size / PHY_MANAGER_BLOCK_SIZE)+1;
	if(size < 0) return;
	while(blocks >=0){
		mmap_unset(align++);
		usedBlocks--;
		blocks--;
	}
	mmap_set(0);
}

/* Deinitialize a region of memory */
void phy_manager_deinit_region(int *base,int size)
{
	int align = (int)(base)/PHY_MANAGER_BLOCK_SIZE;
	int blocks = size / PHY_MANAGER_BLOCK_SIZE;
	if(size < 0) return;
	while(blocks >= 0){
		mmap_set(align++);
		usedBlocks++;
		blocks--;
	}
}

/* Allocate contiguous blocks of memory */
void *phy_manager_alloc_blocks(int numblocks)
{
	int freeBlock=0,i=0;
	if((maxBlocks - usedBlocks <=0) || (numblocks <0))
		return NULL;
	freeBlock = phy_get_free_block(numblocks);

	if(freeBlock < 0)
		return NULL;
	for(i=0;i<numblocks;i++){
		mmap_set(freeBlock+i);
		usedBlocks++;
	}
	return (void*)(freeBlock * PHY_MANAGER_BLOCK_SIZE);
}

/* Deallocate contiguous blocks of memory */
void phy_manager_dealloc_blocks(void *blockPtr,int numBlocks)
{
	int block=(int)blockPtr/PHY_MANAGER_BLOCK_SIZE;
	int i=0;
	if(numBlocks < 0) return;
	for(i=0;i<numBlocks;i++){
		mmap_unset(block+i);
		usedBlocks--;
	}
}

void PHYinit(struct multiboot_info *mbootPtr,int kernelSize)
{
	int i=0;
	struct multiboot_mmap_entry *region=(struct multiboot_mmap_entry*)mbootPtr->mmapAddress;
	/* Initialize the entire memory 
		the memory size is given by the multiboot structure (so is provided by the BIOS at startup)
		|   BIOS   |
		----------- 0x100000
		|  KERNEL  |
		|          |
		----------- 0x100000 + kernelSize (multiplied by 512 because is given in number of clusters)
		|          |
		|   ...    | MEMORY AVAILABLE
		|          |
	*/

	phy_manager_init(getMemorySize(mbootPtr),(int*)(0x100000 + (kernelSize*512)));

	/* Print the characteristics of the regions of the memory (Testing purposes) */
    /* TODO: Let's assume 10 possible regions of physical memory for now.....) */
	for(i=0;i<10;i++){
		if(region[i].type > 4)
			continue;
		if(i > 0 && region[i].addressLow == 0)
			continue;
		print("region %d: start: 0x%x length: %x end: %x type: %d\r\n",
			i, (region[i].addressLow),
			(region[i].lengthLow ),
			(region[i].addressLow)+(region[i].lengthLow)-1,
			region[i].type);
		if(region[i].type==1){
			phy_manager_init_region((int*)region[i].addressLow,region[i].lengthLow);
		}
	}
	asm("jmp .");
}
