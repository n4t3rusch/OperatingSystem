// modified by Josh Wild, Nathan Rusch with help from chat gpt

#include <xinu.h>

static ulong *pgTraverseAndCreate(pgtbl pagetable, ulong virtualaddr, int attr, ulong physicaladdr);

/**
 * Maps a page to a specific virtual address
 * @param pagetable     the base pagetable
 * @param page          the page that will be stored at the virtual address
 * @param virtualaddr   the virtual address at which the page will reside
 * @param attr          any attributes to set on the page table entry
 * @return              OK if successful, otherwise a SYSERR
 */
syscall mapPage(pgtbl pagetable, page pg, ulong virtualaddr, int attr)
{
    ulong addr;

    addr = (ulong)truncpage(virtualaddr);

    if (pgTraverseAndCreate(pagetable, addr, attr, (ulong) pg) == (ulong *)SYSERR)
    {
        return SYSERR;
    }

    return OK;
}

/**
 * Maps a given virtual address range to a corresponding physical address range.
 * @param pagetable    the base pagetable
 * @param virtualaddr  the start of the virtual address range. This will be truncated to the nearest page boundry.
 * @param physicaladdr the start of the physical address range
 * @param length       the length of the range to map
 * @param attr         any attributes to set on the page table entry
 * @return             OK if successful, otherwise a SYSERR
 */
syscall mapAddress(pgtbl pagetable, ulong virtualaddr, ulong physicaladdr,
               ulong length, int attr)
{
    ulong addr, end;
    ulong nlength;


    if (length == 0)
    {
        return SYSERR;
    }

    // Round the length to the nearest page size
    nlength = roundpage(length);
    addr = (ulong)truncpage(virtualaddr);
    end = addr + nlength;

    // Loop over the entire range
    for (; addr < end; addr += PAGE_SIZE, physicaladdr += PAGE_SIZE)
    {
        // Create a page table entry if one doesn't exist. Otherwise, get the existing page table entry.
        if (pgTraverseAndCreate(pagetable, addr, attr, physicaladdr) == (ulong *)SYSERR)
        {
            return SYSERR;
        }
    }

    return OK;
}

/**
 * Starting at the base pagetable, tranverse the hierarchical page table structure for the virtual address.  Create pages along the way if they don't exist.
 * @param pagetable    the base pagetable
 * @param virtualaddr  the virtual address to find the it's corresponding page table entry.
 * @param attr	       the attributes to set on the leaf page
 * @return             OK
 */

static ulong *pgTraverseAndCreate(pgtbl pagetable, ulong virtualaddr, int attr, ulong physicaladdr) {

    /**
    * TODO:
    * For each level in the page table, get the page table entry by masking and shifting the bits in the virtualaddr depending on the level
    * If the valid bit is set, use that pagetable for the next level
    * Otherwise create the page by calling pgalloc().  Make sure to setup the page table entry accordingly. Call sfence_vma once finished to flush TLB
    * Once you've tranversed all three levels, set the attributes (attr) for the leaf page (don't forget to set the valid bit!)
    */

   

    // LEVEL 0
    // 0000000000000000000000000   000000000   000000000   111111111   000000000000   
    ulong lvl0mask = 0x1FF << 12;
    
    // LEVEL 1
    // 0000000000000000000000000   000000000   111111111   000000000   000000000000
    ulong lvl1mask = 0x1FF << 21;

    // LEVEL 2
    // 0000000000000000000000000   111111111   000000000   000000000   000000000000   
    ulong lvl2mask = 0x1FF << 30;

    
    ulong pageLevel[3];
    ulong * entry;
    int i;

    // sets up array to hold different masks that mask off each level
    pageLevel[0] = (virtualaddr & lvl2mask) >> 30; 
    pageLevel[1] = (virtualaddr & lvl1mask) >> 21;
    pageLevel[2] = (virtualaddr & lvl0mask) >> 12; 


    // loop through each level of the page tables
    for (i = 0; i <= 2; i++) {
        // store the page table for the current level
        entry = &pagetable[pageLevel[i]];

        // if the valid bit is not set 
        if ((*entry & PTE_V) == 0) { 
            // and this is not the lowest page table level
            if (i < 2) { 
                // create a new page
                page newPage = (page)pgalloc();
                // error checking the the new page is not null meaning there is memory to allocate
                if (newPage == NULL) {
                    kprintf("out of memory\r\n");
                    return (ulong*)SYSERR;
                }
                // enter in newly allocated page to where there was none and set the valid bit to one
                *entry = PA2PTE(newPage) | PTE_V; 
                // store the physical address of the newly created page in the page table/go to next level of table
                pagetable = (pgtbl)PTE2PA(*entry);
            } 
        } 
        // if the valid bit is set and there is already a page there
        else {
            // if this is not the lowest page table level
            if (i < 2)
                // traverse to next level of table
                pagetable = (pgtbl) PTE2PA(*entry);
        }
    }

    // once it is the lowest page table level
    entry = (ulong *) pagetable[pageLevel[2]]; 
    // set the page table entry for the lowest level with the valid bit and other attributes
    entry = (ulong *) (PA2PTE(physicaladdr) | PTE_V | attr); 

    // restore page table entry
    pagetable[pageLevel[2]] = (ulong) entry;

    sfence_vma(); 

    return (ulong*)OK;
}



   

