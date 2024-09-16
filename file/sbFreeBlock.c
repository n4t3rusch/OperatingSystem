/* sbFreeBlock.c - sbFreeBlock */
/* Copyright (C) 2008, Marquette University.  All rights reserved. */
/*                                                                 */
/* Modified by     Josh Wild                                       */
/*                                                                 */
/* and   Nathan Rusch                                              */
/*                                                                 */
/*                                                                 */

#include <kernel.h>
#include <device.h>
#include <memory.h>
#include <disk.h>
#include <file.h>

/*------------------------------------------------------------------------
 * sbFreeBlock - Add a block back into the free list of disk blocks.
 *------------------------------------------------------------------------
 */

// single case
devcall swizzle(int diskfd, struct freeblock * freeblk);
// global case
devcall superSwizzle(int diskfd, struct freeblock * freeblk, struct superblock * psuper);

devcall sbFreeBlock(struct superblock *psuper, int block)
{
    // TODO: Add the block back into the filesystem's list of
    //  free blocks.  Use the superblock's locks to guarantee
    //  mutually exclusive access to the free list, and write
    //  the changed free list segment(s) back to disk.

    struct freeblock * freeblk;
    struct freeblock * free2;
    struct dentry * phw;
    int disk_fd;

    // initial error checking that the super block struct exists and that the block number passed is a valid block number
    if (psuper == NULL || block <= 0 || block > DISKBLOCKTOTAL) {
        return SYSERR;
    }

    phw = psuper -> sb_disk;
    if (NULL == phw) {
        return SYSERR;
    }
    disk_fd = phw - devtab;

    // wait right here before accessing the free list to make sure no other threads are accessing
    wait(psuper -> sb_freelock);

    // once we have aquired the lock for the suberblock free list, access the free list
    freeblk = psuper -> sb_freelst;


    if (freeblk == NULL) {
        freeblk = (struct freeblock *) malloc(sizeof(struct freeblock));
        freeblk -> fr_blocknum = 0;
        freeblk ->fr_free[freeblk -> fr_count] = block;
        freeblk -> fr_count ++;
        superSwizzle(disk_fd, freeblk, psuper);
    }
    while (freeblk -> fr_next != NULL) {
        freeblk = freeblk -> fr_next;
    }
    if (freeblk -> fr_count == 0 || freeblk -> fr_count == FREEBLOCKMAX) {
        free2 = (struct freeblock *) malloc(sizeof(struct freeblock));
        free2-> fr_blocknum = freeblk -> fr_blocknum ++;
        free2 -> fr_free[free2 -> fr_count] = block;
        free2 -> fr_count ++;
        swizzle(disk_fd, free2);
        freeblk -> fr_next = free2;
    }
    else {
        freeblk -> fr_free[freeblk -> fr_count] = block;
        freeblk -> fr_count ++;
    }

    signal(psuper -> sb_freelock);

    return SYSERR;
}

devcall swizzle(int diskfd, struct freeblock * freeblk) {
    struct freeblock * free2 = freeblk -> fr_next;

    if (freeblk -> fr_next == NULL) {
        freeblk -> fr_next = 0;
    }
    else {
        freeblk -> fr_next = (freeblk -> fr_next) -> fr_blocknum;
    }

    seek(diskfd, freeblk -> fr_blocknum);
    
    write(diskfd, freeblk -> fr_blocknum, sizeof(struct freeblock));

    freeblk -> fr_next = free2;
}

devcall superSwizzle(int diskfd, struct freeblock * freeblk, struct superblock * psuper) {
    struct freeblock * swizzle = psuper -> sb_freelst;
    struct dirblock * swizzle2 = psuper -> sb_dirlst;

    psuper -> sb_freelst = swizzle -> fr_blocknum;
    psuper -> sb_dirlst = swizzle2 -> db_blocknum;

    seek(diskfd, psuper -> sb_blocknum);

    if (SYSERR == write(diskfd, psuper, sizeof(struct superblock))) {
        return SYSERR;
    }

    return OK;
}
