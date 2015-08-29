#include <pageDirectoryEntry.h>

inline void pde_add_attribute(pageDirectoryEntry *pde,int attribute)
{
    *pde |= attribute;
}

inline void pde_delete_attribute(pageDirectoryEntry *pde,int attribute)
{
    *pde &= ~(attribute);
}

inline void pde_set_frame(pageDirectoryEntry *pde,int physical_address)
{
    *pde = (*pde & ~(PDE_FRAME)) | physical_address;
}

inline int pde_isPresent(pageDirectoryEntry pde)
{
    return pde & PDE_PRESENT;
}

inline int pde_isWritable(pageDirectoryEntry pde)
{
    return pde & PDE_WRITABLE;
}

inline int pde_isUser(pageDirectoryEntry pde)
{
    return pde & PDE_USERMODE;
}

inline int pde_is4MB(pageDirectoryEntry pde)
{
    return pde & PDE_4MB;
}

inline int pde_physicalAddress(pageDirectoryEntry pde)
{
    return pde & PDE_FRAME;
}

