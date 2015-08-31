#ifndef __VIRTUALMEMORYMANAGER_H
#define __VIRTUALMEMORYMANAGER_H

#include <pageTableEntry.h>
#include <pageDirectoryEntry.h>

/* VIRTUAL MEMORY MANAGER HEADER */

typedef int virtualAddress;

#define PAGES_PER_TABLE               1024     /* 4MB for each table */
#define PAGETABLES_PER_DIRECTORY      1024     /* 4GB of virtual memory */

#define PAGE_DIRECTORY_INDEX(x)       (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x)           (((x) >> 12) & 0X3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x)  (*x & (~0xfff))

typedef struct pageTable{
	pageTableEntry table[PAGES_PER_TABLE];
}pageTable;

typedef struct pageDirectory{
	pageDirectoryEntry table[PAGETABLES_PER_DIRECTORY];
}pageDirectory;

void mapPhytoVirtual(void *physicalAddress,void *virtualAddress);
void VMMinit();

void VMM_alloc(pageTableEntry *pte);
void VMM_free(pageTableEntry *pte);
void VMM_clear_pageTable(pageTable *tab);
int VMM_PTE_virtual_to_index(virtualAddress address);
pageTableEntry *VMM_PTE_lookup_entry(pageTable *tab,virtualAddress address);

void VMM_switch_directory(pageDirectory *pd);
pageDirectory *VMM_getDirectory();

void VMM_clear_pageDirectory(pageDirectory *tab);
int VMM_PDE_virtual_to_index(virtualAddress address);
pageDirectoryEntry *VMM_PDE_lookup_entry(pageDirectory *tab,virtualAddress address);

void VMM_flush_TLB_entry(virtualAddress address);

void enable_paging();
#endif
