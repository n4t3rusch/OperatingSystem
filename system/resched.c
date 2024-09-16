/**
 * @file resched.c
 * @provides resched
 * AUTHORS: Josh Wild, Nathan Rusch
 * COSC 3250 Assignment 6
 */
/* Embedded XINU, Copyright (C) 2008,2024.  All rights reserved. */

#include <xinu.h>

extern void ctxsw(void *, void *, ulong);

uint getTotalTickets(void);

int getWinner(uint totalTickets);

/**
 * Reschedule processor to next ready process.
 * Upon entry, currpid gives current process id.  Proctab[currpid].pstate 
 * gives correct NEXT state for current process if other than PRREADY.
 * @return OK when the process is context switched back
 */
syscall resched(void)
{
    pcb *oldproc;               /* pointer to old process entry */
    pcb *newproc;               /* pointer to new process entry */

    oldproc = &proctab[currpid];

    /* place current process at end of ready queue */
    if (PRCURR == oldproc->state)
    {
        oldproc->state = PRREADY;
        enqueue(currpid, readylist);
    }

    /**
     * We recommend you use a helper function for the following:
     * TODO: Get the total number of tickets from all processes that are in
     * current and ready states.
     * Use random() function to pick a random ticket. 
     * Traverse through the process table to identify which proccess has the
     * random ticket value.  Remove process from queue.
     * Set currpid to the new process.
     */

    uint totalTickets;                          // variable to hold the total number to tickets
    totalTickets = getTotalTickets();           // total up the tickets of all active processes

    int winnerpid;                              // variable to hold the winner's pid
    winnerpid = getWinner(totalTickets);        // get the winner pid

    currpid = remove(winnerpid);                // remove the winner from the queue based on its pid

    // currpid = dequeue(readylist);
    newproc = &proctab[currpid];                
    newproc->state = PRCURR;    /* mark it currently running    */

#if PREEMPT
    preempt = QUANTUM;
#endif
    pcb * current = &proctab[currpid];
    ctxsw(&oldproc->stkptr, &newproc->stkptr, MAKE_SATP(currpid, current -> pagetable));


    /* The OLD process returns here when resumed. */
    return OK;
}

uint getTotalTickets(void) {
    int i;                                      // counter for loop
    uint numTickets = 0;                        // counter for total number of tickets
    pcb * currproc = NULL;                      // pointer to the current pcb
    for (i = 0; i < NPROC; i ++) {              // loop through the proctab array of all pcb entries
        currproc = &proctab[i];                 // set the pointer to each proctab entry
        if ((currproc -> state == PRCURR) || (currproc -> state == PRREADY)) {
            numTickets += (currproc -> tickets);    // add the current pcb entry's tickets to the total
        }
    }
    return numTickets;                          // return the total number of tickets
}

int getWinner(uint totalTickets) {
    int counter = 0;                            // counter for amount of ticekts that have been added up so far
    int winner = random(totalTickets);          // get a random "winner" value between [0, totalTickets)
    int i;                                      // counter for loop
    pcb * currproc = NULL;                      // pointer to pcb entry within proctab
    for (i = 0; i < NPROC; i ++) {            // loop through entries in proctab
        currproc = &proctab[i];                 // set pointer to point at the current proctab pcb entry
        if ((currproc -> state == PRCURR) || (currproc -> state == PRREADY)) {
            counter += (currproc -> tickets);       // add the current pcb entry's tickets to the counter
            if (counter > winner) {                 // check if the counter is greater than the "winner" value
                break;                              // if it is, break
            }
        }
    }
    return i;                                   // return whatever when i is when the loop breaks because that is 
}                                                // the proctab entry that "won"

