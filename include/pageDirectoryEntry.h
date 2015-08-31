#ifndef __PAGETABLEDIRECTORY_H
#define __PAGETABLEDIRECTORY_H

enum pageDirectoryEntryFlags
{
	PDE_PRESENT       = 0x1,
	PDE_WRITABLE      = 0x2,
	PDE_USERMODE      = 0x4,
	PDE_WRITETHROUGH  = 0x8,
	PDE_NOT_CACHEABLE = 0x10,
	PDE_ACCESSED      = 0x20,
	PDE_DIRTY         = 0x40,
	PDE_4MB           = 0x80,
	PDE_CPU_GLOBAL    = 0x100,
	PDE_LV4_GLOBAL    = 0x200,
	PDE_FRAME         = 0x7FFF000
};

typedef int pageDirectoryEntry;

void pde_add_attribute(pageDirectoryEntry *pde,int attribute);
void pde_delete_attribute(pageDirectoryEntry *pde,int attribute);
void pde_set_frame(pageDirectoryEntry *pde,int phisical_address);
int pde_isPresent(pageDirectoryEntry pde);
int pde_isWritable(pageDirectoryEntry pde);
int pde_isUser(pageDirectoryEntry pde);
int pde_is4MB(pageDirectoryEntry pde);
int pde_physicalAddress(pageDirectoryEntry pde);

#endif
