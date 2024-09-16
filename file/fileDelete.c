/* fileDelete.c - fileDelete */
/* Copyright (C) 2008, Marquette University.  All rights reserved. */
/*                                                                 */
/* Modified by           Josh Wild                                 */
/*                                                                 */
/* and  Nathan Rusch                                               */
/*                                                                 */
/*                                                                 */

#include <kernel.h>
#include <memory.h>
#include <file.h>

/*------------------------------------------------------------------------
 * fileDelete - Delete a file.
 *------------------------------------------------------------------------
 */
devcall fileDelete(int fd)
{
    // TODO: Unlink this file from the master directory index,
    //  and return its space to the free disk block list.
    //  Use the superblock's locks to guarantee mutually exclusive
    //  access to the directory index.

    // error check fd and superblock
    if (isbadfd(fd) || supertab == NULL) 
        return SYSERR;

    struct dentry * phw = supertab -> sb_disk;
    int diskfd = phw - devtab;
    
    // wait on the directory lock
    wait(supertab -> sb_dirlock);

    // once lock is aquired, get the superblock's directory list
    struct dirblock * dirblk = supertab -> sb_dirlst;

    // error check that the directory list is not NULL
    if (dirblk == NULL)
        return SYSERR;

    // INSTEAD OF FILETAB, we may have to use dirblk -> db_fnodes[fd]

    // error check the file's state (not sure what to check for I checked if it's already free)
    if (filetab[fd].fn_state == FILE_DIRTY || filetab[fd].fn_state == FILE_FREE) {
        signal(supertab -> sb_dirlock);
        return SYSERR;
    }

    // store file's length
    int length = filetab[fd].fn_length;
    
    // clear values from file
    filetab[fd].fn_length = 0;
    filetab[fd].fn_cursor = 0;
    filetab[fd].fn_blocknum = 0;
    filetab[fd].fn_data = '\0';
    strncpy(filetab[fd].fn_name, '\0', FNAMLEN);

    // free the file's block from disk
    sbFreeBlock(filetab[fd].fn_data, fd);

    // set the file's state to FREE
    filetab[fd].fn_state = FILE_FREE;

    seek(diskfd, (supertab -> sb_dirlst) -> db_blocknum);

    write(diskfd, supertab -> sb_dirlst, sizeof(struct dirblock));
    
    signal(supertab -> sb_dirlock);

    return OK;
}
