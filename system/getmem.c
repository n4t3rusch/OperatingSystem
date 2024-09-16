/**
 * @file getmem.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>
#include <memory.h>

/**
 * @ingroup memory_mgmt
 *
 * Allocate heap memory.
 *
 *
 * @param nbytes
 *      Number of bytes requested.
 *
 * @return
 *      ::SYSERR if @p nbytes was 0 or there is no memory to satisfy the
 *      request; otherwise returns a pointer to the allocated memory region.
 *      The returned pointer is guaranteed to be 8-byte aligned.  Free the block
 *      with memfree() when done with it.
 */
void *getmem(ulong nbytes)
{
    register memblk *prev, *curr, *leftover;

    if (0 == nbytes)
    {
        return (void *)SYSERR;
    }

    /* round to multiple of memblock size   */
    nbytes = (ulong)roundmb(nbytes);
    struct memhead *head = (memhead *)PROCHEAPADDR;

    /* TODO:
     *      - Traverse through the freelist
     *        to find a block that's suitable 
     *        (Use First Fit with remainder splitting)
     *      - if there is no suitable block, call user_incheap
     *        with the request to add more pages to our stack
     *      - return memory address if successful
     */

    // have the free list of free memory chunks as the variable 'head'
    // to get the actual linked list of free memory chunks, we need the free list's 'head' field (AKA head -> head) 
    // because this takes the free list struct itself and goes to the actual linked list of memory chunks
    // these memory chunks are stored as (similar but separate) memblock structs which store their lengths and
    // the next free chunk in the linked list
    // therefore the current chunk of memory will be a memblock struct and more specifically will be the 'head' field of
    // the memhead struct stored in the variable 'head' (head -> head)
    curr = (struct memblock *) head -> head;
   
   // iterate through the linked list of free memory chunks (while curr is not NULL)
    while (curr) {
        // if the current chunk is a size that is just big enough or more than big enough to support the requested bytes
        if ((curr -> length) >= nbytes) {
            if (curr -> length > nbytes) {
                leftover = curr + nbytes;
                leftover -> length = (curr -> length) - nbytes;

                if (prev == NULL) {
                    head -> head = leftover;
                    leftover -> next = curr -> next;
                }
                else {
                    prev -> next = leftover;
                    leftover -> next = curr -> next;
                }
            }
            else {
                // if prev is NULL, this means that the head of the linked list of free memory chunks was big enough to support the 
                // request and therefore the head of the linked list is being removed so we set head to be the next element of the list
                if (prev == NULL)
                    head -> head = curr -> next;
                // if prev is not null (meaning this is not the first element/head of linked list), set the prev's next to be the 
                // current's next, IE remove the current chunk from the free list
                else
                    prev -> next = curr -> next;

                // since removing the current chunk from the free list, undo its link by setting its next field to NULL
                curr -> next == NULL;

                // store a void * to the address of the memory chunk (this would be the location of the chunk pointed to by 'curr' AKA
                // the value of 'curr' not its memory location itself, plus the size of the memblock struct that holds this chunk's 
                // accounting info)
                void * addr = (void *) (curr + sizeof(struct memblock));
                // return the pointer to the correct address in memory
                return addr;
            }
        }
        // if the current chunk is not large enough to support the memory request

        // set previous to be current
        prev = curr;
        // set current to be the next chunk in the linked list
        curr = curr -> next;
        // set prev's next to be current
        prev -> next = curr;
    }

    // if there are no chunks of free memory large enough to support the memory request, return the new chunk of heap memory
    // allocated by user_incheap()
    return (void *) user_incheap(nbytes);

    // return (void *)SYSERR;
}
