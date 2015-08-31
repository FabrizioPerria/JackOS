#include <pageTableEntry.h>

inline void pte_add_attribute(pageTableEntry *pte,int attribute)
{
	*pte |= attribute;
}

inline void pte_delete_attribute(pageTableEntry *pte,int attribute)
{
	*pte &= ~(attribute);
}

inline void pte_set_frame(pageTableEntry *pte,int physical_address)
{
	*pte = (*pte & ~(PTE_FRAME)) | physical_address;
}

inline int pte_isPresent(pageTableEntry pte)
{
	return pte & PTE_PRESENT;
}

inline int pte_isWritable(pageTableEntry pte)
{
	return pte & PTE_WRITABLE;
}

inline int pte_physicalAddress(pageTableEntry pte)
{
	return pte & PTE_FRAME;
}

