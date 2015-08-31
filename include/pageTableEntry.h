#ifndef __PAGETABLEENTRY_H
#define __PAGETABLEENTRY_H

enum pageTableEntryFlags
{
	PTE_PRESENT       = 0x1,
	PTE_WRITABLE      = 0x2,
	PTE_USERMODE      = 0x4,
	PTE_WRITETHROUGH  = 0x8,
	PTE_NOT_CACHEABLE = 0x10,
	PTE_ACCESSED      = 0x20,
	PTE_DIRTY         = 0x40,
	PTE_PAT           = 0x80,
	PTE_CPU_GLOBAL    = 0x100,
	PTE_LV4_GLOBAL    = 0x200,
	PTE_FRAME     = 0x7FFF000
};

typedef int pageTableEntry;

void pte_add_attribute(pageTableEntry *pte,int attribute);
void pte_delete_attribute(pageTableEntry *pte,int attribute);
void pte_set_frame(pageTableEntry *pte,int physical_address);
int pte_isPresent(pageTableEntry pte);
int pte_isWritable(pageTableEntry pte);
int pte_physicalAddress(pageTableEntry pte);

#endif
