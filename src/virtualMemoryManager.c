#include <virtualMemoryManager.h>
#include <pageTableEntry.h>
#include <pageDirectoryEntry.h>
#include <physicalMemoryManager.h>
#include <system.h>

/* VIRTUAL MEMORY MANAGER IMPLEMENTATION */

static pageDirectory *currentDirectory = 0;

/* Map a physical address into a virtual address */
void mapPhytoVirtual(void *physicalAddress,void *virtualAddress)
{
    pageTable *pt;
    pageTableEntry *pte;
    pageDirectoryEntry *pde= &currentDirectory->table[PAGE_DIRECTORY_INDEX((int)virtualAddress)];
    if(!(*pde & PTE_PRESENT)){
        pt = (pageTable*)phy_manager_alloc_blocks(1);
        if(!pt) return;

        memset((unsigned char *)pt,0,sizeof(pageTable));
        /*Now p points to the page of the virtual address */
        pde= &currentDirectory->table[PAGE_DIRECTORY_INDEX((int)virtualAddress)];
        pde_add_attribute(pde,PDE_PRESENT | PDE_WRITABLE);
        pde_set_frame(pde,(int)pt);
    }

    pt = (pageTable*) (PAGE_GET_PHYSICAL_ADDRESS(pde));
    pte = &pt->table[PAGE_TABLE_INDEX((int)virtualAddress)];
    pte_set_frame(pte,(int)physicalAddress);
    pte_add_attribute(pte,PTE_PRESENT);
}

/*Initialize virtual memory manager */
void VMMinit()
{
    pageTable *table_first_4mb,*table_kernel;
    pageTableEntry *pte=0;
    pageDirectory *dir;
    pageDirectoryEntry *pde=0;
    int i,frame,virt;

	/*DEFAULT PAGE TABLE */
    /* Default Page Table; it will be identity mapped */
    table_first_4mb = (pageTable*)phy_manager_alloc_blocks(1);
    if(!table_first_4mb) return;

	/* Reset the default page table */
    memset((unsigned char*)table_first_4mb,0,sizeof(pageTable));

    for(i=0,frame=0x0,virt=0x0; i<1024 ;i++,frame+=4096,virt+=4096){
        pte_add_attribute(pte,PTE_PRESENT);
        pte_set_frame(pte,frame);
        table_first_4mb->table[PAGE_TABLE_INDEX(virt)]= (int)pte;
    }

	/* KERNEL'S PAGE TABLE */

    /*3GB kernel's page table; map 1MB physicalmemory into
      3GB virtual memory */
    table_kernel = (pageTable*)phy_manager_alloc_blocks(1);
    if(!table_kernel) return;
  
    memset((unsigned char *)table_kernel,0,sizeof(pageTable));

    for(i=0,frame=0x100000,virt=0xc0000000;i<1024;i++,frame+=4096,virt+=4096){
        pte_add_attribute(pte,PTE_PRESENT);
        pte_set_frame(pte,frame);
        table_kernel->table[PAGE_TABLE_INDEX(virt)]= (int)pte;
    }

    dir=(pageDirectory*) phy_manager_alloc_blocks(3);
    if(!dir) return;
    memset((unsigned char *)dir,0,sizeof(pageDirectory));

    pde = &dir -> table[PAGE_DIRECTORY_INDEX(0x00000000)];
    pde_add_attribute(pde,PDE_PRESENT|PDE_WRITABLE);
    pde_set_frame(pde,(int)table_first_4mb);
    
    pde = &dir -> table[PAGE_DIRECTORY_INDEX(0xc0000000)];
    pde_add_attribute(pde,PDE_PRESENT|PDE_WRITABLE);
    pde_set_frame(pde,(int)table_kernel);

    VMM_switch_directory(dir);

    enable_paging();
}

/*Allocate a new Page in the pageTable */
void VMM_alloc(pageTableEntry *pte)
{
    void *ptr=phy_manager_alloc_blocks(1);
    if(ptr){ 
        pte_set_frame(pte,(int)ptr);
        pte_add_attribute(pte,PTE_PRESENT);
    }
}

/*Deallocate a page from the pageTable*/
void VMM_free(pageTableEntry *pte)
{
    void *ptr=(void *)pte_physicalAddress(*pte);
    if(ptr){
        phy_manager_dealloc_blocks(ptr,1);
    }
    pte_delete_attribute(pte,PTE_PRESENT);
}

/*Find an entry in the page table given the virtual Address*/
inline pageTableEntry *VMM_PTE_lookup_entry(pageTable *tab,virtualAddress address)
{
    if(tab){
        return &tab->table[PAGE_TABLE_INDEX(address)];
    }else return 0;
}

/*Change the current directory in the CPU's register*/
void VMM_switch_directory(pageDirectory *pd)
{
    if(pd){
        currentDirectory = pd; 

        /* load the Page Directory Base Register (cr3) */

        __asm__ __volatile__("mov %0,%%eax\n"
                             "mov %%eax,%%cr3\n"
                             ::"m"(currentDirectory));
    }
}

/*Get the current directory */
pageDirectory *VMM_getDirectory()
{
    return currentDirectory;
}

/*Find the directory in the table*/
inline pageDirectoryEntry *VMM_PDE_lookup_entry(pageDirectory *tab,virtualAddress address)
{
    if(tab){
        return &tab->table[PAGE_TABLE_INDEX(address)];
    }else return 0;

}

/*invalidate the TLB; the INVLPG instruction must be used
  only in supervisor mode */
void VMM_flush_TLB_entry(virtualAddress address)
{
    __asm__ __volatile__ ("cli\n"
                          "invlpg %0\n"
                          "sti\n"::"m"(address));
}

void enable_paging()
{
    __asm__ __volatile__ ("mov %cr0,%eax\n"
                          "or $0x8000000,%eax\n"
                          "mov %eax,%cr0\n");
}
