/**
 * @file free.c
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <xinu.h>

/**
 * @ingroup libxc
 *
 * Attempt to free a block of memory based on malloc() accounting information
 * stored in preceding two words.
 *
 * @param ptr
 *      A pointer to the memory block to free.
 */
syscall free(void *ptr)
{
    struct memblock *block;

    /* TODO:
     *      1) set block to point to memblock to be free'd (ptr)
     *      2) find accounting information of the memblock
     *      3) call freemem on the block with its length
     */

    // if ptr points to the address of just the memory chunk to be freed, back that up by the size of a memblock struct
    // so that block points to the struct holding the accounting info just before the chunk of actual memory
    block = ptr - sizeof(struct memblock);


    // now that block points to the start of the struct holding the accounting info and not just the chunk of memory,
    // find the length of the chunk (block -> length)
    ulong chunkLength = block -> length;

    // call freemem() passing in the address of the chunk of memory to be freed, and the length of the chunk, and store
    // the syscall return value
    syscall ret = freemem(ptr - (sizeof(struct memblock)), chunkLength);

    // if the syscall return value from freemem is a SYSERR, return that
    if (ret == SYSERR)
        return SYSERR;
    // otherwise return OK
    return OK;
}
