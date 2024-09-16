/**
 * @file malloc.c
 * This file is deprecated in favor of mem/malloc.c and the user heap
 * allocator.  However, it remains here for backup purposes.
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <xinu.h>

/**
 * @ingroup libxc
 *
 * Request heap storage, record accounting information, returning pointer
 * to assigned memory region.
 *
 * @param size
 *      number of bytes requested
 *
 * @return
 *      pointer to region on success, or @c NULL on failure
 */
void *malloc(ulong size)
{
    struct memblock *pmem;

    /* we don't allocate 0 bytes. */
    if (0 == size)
    {
        return NULL;
    }

      /** TODO:
      *      1) Make room for accounting info
      *      2) Acquire memory with getmem
      *         - handle possible error (SYSERR) from getmem...
      *      3) Set accounting info in pmem
      */

    // make space for accounting block
    size += sizeof(struct memblock);

    // use getmem to get a pointer to a chunk of memory
    void * addr = getmem(size);

    // if getmem returns an error
    if (addr == (void *) SYSERR) 
      return NULL;
    // if getmem does not return an error
    else {
      // the address of the struct is right before the chunk of memory so since addr points to the memory chunk itself, go back
      // by the size of the memblock struct 
      pmem = addr - sizeof(struct memblock);
      // since the length parameter includes the struct, just set it to the total size since the total size includes the size of 
      // the struct
      pmem -> length = size;
      // since the next chunk of memory would be after the chunk that was just malloc'ed, add to addr the size of just the memory 
      // chunk (AKA the size minus the size of a memblock struct)
      pmem -> next = (void *) addr + (size - sizeof(struct memblock));

      //return the address of the memory chunk
      return addr;
    }

    // return (void *)SYSERR;
}
