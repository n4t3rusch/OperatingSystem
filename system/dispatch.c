/**
 * @file dispatch.c
 * @provides create, newpid, userret
 * Authors: Josh Wild, Nathan Rusch
 */
/* Embedded XINU, Copyright (C) 2008.  All rights reserved. */


#include <xinu.h>
#include <interrupt.h>

/**
 * 
 * @ingroup process
 * Dispatch the trap or exception handler, called via interrupt.S
 * @param cause  The value of the scause register 
 * @param stval  The value of the stval register  
 * @param frame  The stack pointer of the process that caused the interupt 
 * @param program_counter  The value of the sepc register 
 */

       /**
		* TODO:
		* Check to ensure the trap is an environment call from U-Mode
		* Find the system call number that's triggered
		* Pass the system call number and any arguments into syscall_dispatch. Make sure to set the return value in the appropriate spot.
		* Update the program counter appropriately with set_sepc
		*
		* If the trap is not an environment call from U-Mode call xtrap
		*/

ulong dispatch(ulong cause, ulong val, ulong * frame, ulong *program_counter) {
    ulong swi_opcode;
	pcb * current = &proctab[currpid];
	ulong satp_value = MAKE_SATP(currpid, (current -> pagetable));
    
    if((long)cause > 0) {
        cause = cause << 1;
        cause = cause >> 1;

		// if the cause of the exception is from an environment call
		if (cause == E_ENVCALL_FROM_UMODE) {
			
			// get the syscall code number from the stack
			swi_opcode = current -> swaparea[CTX_A7];	

			// call syscall_dispatch passing in the syscall code number, and the frame pointer where
			// the arguments are stored
			current -> swaparea[CTX_A0] = syscall_dispatch(swi_opcode, &(current -> swaparea[CTX_A0]));  

			// set the program counter to where it should go
			set_sepc(((ulong) program_counter) + 4);		
		
		}
		// if the cause of the exception is not an envrionment call, call xtrap()
		else {
			// frame is the stack pointer to the process that raised the exception
			// cause is the code for what the cause of the exception is
			// val holds the contents of the register which stores the virtual address of the 
			//		instruction that raised the exception
			xtrap((current -> swaparea), cause, val, program_counter);
		}
    } 
	else {
		cause = cause << 1;
		cause = cause >> 1;
		uint irq_num;

		volatile uint *int_sclaim = (volatile uint *)(PLIC_BASE + 0x201004);
		irq_num = *int_sclaim;

		if (cause == I_SUPERVISOR_EXTERNAL) {
			interrupt_handler_t handler = interruptVector[irq_num];

			*int_sclaim = irq_num;
			if (handler) {
				(*handler) ();
			}
			else {
				kprintf("ERROR: No handler registered for interrupt %u\r\n", irq_num);
				while(1)
					;
			}
		}
	}

	// CHANGE THIS TO BE WHATEVER MAKE_SATP RETURNS
		// need to pass into MAKE_SATP a pid and the PPN of the root page table 
	return satp_value;
}