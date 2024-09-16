/**
 * @file create.c
 * @provides create, newpid, userret
 * Authors: Josh Wild, Nathan Rusch
 * COSC 3250 Assignment 4
 */
/* Embedded XINU, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>

static pid_typ newpid(void);
void userret(void);
void *getstk(ulong);

/**
 * Create a new process to start running a function.
 * @param funcaddr address of function that will begin in new process
 * @param ssize    stack size in bytes
 * @param name     name of the process, used for debugging
 * @param nargs    number of arguments that follow
 * @return the new process id
 */
syscall create(void *funcaddr, ulong ssize, uint priority, char *name, ulong nargs, ...)
{
    ulong *saddr;               /* stack address                */
    ulong *saddr2;               /* stack address                */
    ulong pid;                  /* stores new process id        */
    pcb *ppcb;                  /* pointer to proc control blk  */
    ulong i;
    va_list ap;                 /* points to list of var args   */
    ulong pads = 0;             /* padding entries in record.   */

    if (ssize < MINSTK)
        ssize = MINSTK;

    ssize = (ulong)((((ulong)(ssize + 3)) >> 2) << 2);
    /* round up to even boundary    */

    //COMMENTED OUT GETSTK() BECAUSE WE USE PAGING NOW
    // ALSO MUST CREATE A PAGE AFTER GETTING THE PID SINCE THE PID IS 
    // REQUIRED FOR THE PAGE
    // saddr = (ulong *)getstk(ssize);     /* allocate new stack and pid   */
    pid = newpid();
    /* a little error checking      */
    
    // allocate new page
    page page = pgalloc();

    // create new page table for process with newly created page
    pgtbl pageTable = vm_userinit(pid, page);

    // move saddr to point to the top of the new page/pagetable
    saddr = (page + PAGE_SIZE) - sizeof(ulong);
    saddr2 = saddr;
    

    // error checking
    if ((((ulong *)SYSERR) == saddr) || (SYSERR == pid))
    {
        return SYSERR;
    }

    numproc++;
    ppcb = &proctab[pid];
	
	// TODO: Setup PCB entry for new process.
    // state stack base stack length 
    strncpy(ppcb -> name, name, PNMLEN - 2);
    ppcb -> name[PNMLEN - 1] = '\0';
    ppcb -> stklen = ssize;
    ppcb -> stkbase = saddr;
    ppcb -> state = PRSUSP;    // whatever process state should start out as
    ppcb -> tickets = priority;
    ppcb -> pagetable = pageTable;

    /* Initialize stack with accounting block. */
    *saddr = STACKMAGIC;
    *--saddr = pid;
    *--saddr = ppcb->stklen;
    *--saddr = (ulong)ppcb->stkbase;

    /* Handle variable number of arguments passed to starting function   */
    if (nargs)
    {
        pads = ((nargs - 1) / 8) * 8;
    }
    /* If more than 4 args, pad record size to multiple of native memory */
    /*  transfer size.  Reserve space for extra args                     */
    for (i = 0; i < pads; i++)
    {
        *--saddr = 0;
    }

	// TODO: Initialize process context.
	//
	// TODO:  Place arguments into activation record.
	//        See K&R 7.3 for example using va_start, va_arg and
	//        va_end macros for variable argument functions.
    for (i = 0; i < 32; i ++) {
        *--saddr = 0;
    }

    
    va_start(ap, nargs);
    for (i = 0; i < nargs; i ++) {
        if (i < 8) {
            // first 8 registers
            saddr[i] = va_arg(ap, ulong);
        }
        else {
            // stack
            saddr[(i - 8) + 32] = va_arg(ap, ulong);
        }
        
    }
    
    va_end(ap);

    *(saddr + CTX_PC) = (ulong) funcaddr;
    *(saddr + CTX_RA) = (ulong) userret;
    // *(saddr + CTX_SP) = (ulong) saddr;

    // ppcb -> stkptr = saddr;

    saddr[CTX_SP] = (ulong) (PROCSTACKADDR + PAGE_SIZE - (ulong) (saddr2 - (ulong) (saddr)));

    // saddr[CTX_KERNSATP] = MAKE_SATP(currpid, pageTable);
    // saddr[CTX_KERNSP] = (ulong) (PROCSTACKADDR + PAGE_SIZE - (saddr - (ulong) ppcb -> swaparea));

    return pid;
}

/**
 * Obtain a new (free) process id.
 * @return a free process id, SYSERR if all ids are used
 */
static pid_typ newpid(void)
{
    pid_typ pid;                /* process id to return     */
    static pid_typ nextpid = 0;

    for (pid = 0; pid < NPROC; pid++)
    {                           /* check all NPROC slots    */
        nextpid = (nextpid + 1) % NPROC;
        if (PRFREE == proctab[nextpid].state)
        {
            return nextpid;
        }
    }
    return SYSERR;
}

/**
 * Entered when a process exits by return.
 */
void userret(void)
{
    // ASSIGNMENT 5 TODO: Replace the call to kill(); with user_kill();
    // when you believe your trap handler is working in Assignment 5
    user_kill();
    // kill(currpid); 
}
