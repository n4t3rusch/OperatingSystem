/**
 * @file freemem.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>

/**
 * @ingroup memory_mgmt
 *
 * Frees a block of heap-allocated memory.
 *
 * @param memptr
 *      Pointer to memory block allocated with getmem().
 *
 * @param nbytes
 *      Length of memory block, in bytes.  (Same value passed to getmem().)
 *
 * @return
 *      ::OK on success; ::SYSERR on failure.  This function can only fail
 *      because of memory corruption or specifying an invalid memory block.
 */
syscall freemem(void *memptr, ulong nbytes)
{
    register struct memblock *block, *next, *prev, *curr;
    struct memhead *head = NULL;
    ulong top;

    /* make sure block is in heap */
    if ((0 == nbytes)
        || ((ulong)memptr < (ulong)PROCHEAPADDR))
    {
        return SYSERR;
    }

    head = (struct memhead *)PROCHEAPADDR;
    block = (struct memblock *)memptr;
    curr = (struct memblock *) head -> head;
    nbytes = (ulong)roundmb(nbytes);

    /* TODO:
     *      - Find where the memory block should
     *        go back onto the freelist (based on address)
     *      - Find top of previous memblock
     *      - Make sure block is not overlapping on prev or next blocks
     *      - Coalesce with previous block if adjacent
     *      - Coalesce with next block if adjacent
     */

    if (block < curr) {
        block -> next = curr;
        head -> head = block;
    }
    else {
        while ((curr -> next != NULL) && ((curr -> next) < block)) {
            curr = curr -> next;
        } 

        if (curr -> next == NULL) {
            curr -> next = block;
            block -> next = NULL;
            next = NULL;
        }
        else {
            prev = curr;
            curr = curr -> next;

            prev -> next = block;
            block -> next = curr;

            next = block -> next;
        }
    }


    
    if ((next != NULL) && ((block + nbytes) == block -> next)) {
        block -> length = nbytes + next -> length;
        block -> next = next -> next;
    }

    if ((prev != NULL) && (prev + (prev -> length) == block)) {
        prev -> length = prev -> length + nbytes;
        prev -> next = block -> next;
    }

   
    return OK;
}
