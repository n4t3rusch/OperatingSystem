/**
 * @file     ctxsw.s
 * @provides ctxsw
 * Authors: Josh Wild, Nathan Rusch
 * COSC 3250 / COEN 4820 Assignment 4
 */
/* Embedded XINU, Copyright (C) 2023.  All rights reserved. */

#include <riscv.h>


// THIS SECTION?
.section .ctxswec
	.align 4
	.globl	ctxsw


/**
 * @fn void ctxsw(&oldstack, &newstack)
 *
 * Switch context (values in registers) to another process, saving the
 * current processes information. This function will not return as normally
 * thought as it will load in the program counter for a different process and
 * jump to that location and begin executing code.
 *
 * @param  &oldstack address of outgoing stack save area
 * @param  &newstack address of incoming stack save area
 * @return special case -- see above
 */
	.func ctxsw
ctxsw:

// TODO: Save process state, including at least the 
//	callee-save ("non-volatile") registers.

addi sp, sp, -256

sd a0, CTX_A0 * 8(sp)
sd a1, CTX_A1 * 8(sp)
sd a2, CTX_A2 * 8(sp)
sd a3, CTX_A3 * 8(sp)
sd a4, CTX_A4 * 8(sp)
sd a5, CTX_A5 * 8(sp)
sd a6, CTX_A6 * 8(sp)
sd a7, CTX_A7 * 8(sp)

sd s0, CTX_S0 * 8(sp)
sd s1, CTX_S1 * 8(sp)
sd s2, CTX_S2 * 8(sp)
sd s3, CTX_S3 * 8(sp)
sd s4, CTX_S4 * 8(sp)
sd s5, CTX_S5 * 8(sp)
sd s6, CTX_S6 * 8(sp)
sd s7, CTX_S7 * 8(sp)
sd s8, CTX_S8 * 8(sp)
sd s9, CTX_S9 * 8(sp)
sd s10, CTX_S10 * 8(sp)
sd s11, CTX_S11 * 8(sp)

sd ra, CTX_PC * 8(sp)
sd ra, CTX_RA * 8(sp)
sd sp, CTX_SP * 8(sp)

sd gp, CTX_GP * 8(sp)
sd tp, CTX_TP * 8(sp)

sd t0, CTX_T0 * 8(sp)
sd t1, CTX_T1 * 8(sp)
sd t2, CTX_T2 * 8(sp)
sd t3, CTX_T3 * 8(sp)
sd t4, CTX_T4 * 8(sp)
sd t5, CTX_T5 * 8(sp)
sd t6, CTX_T6 * 8(sp)


// TODO: Restore process state, including at least the
//	callee-save ("non-volatile") registers.
//	These call and restore segments must match the register data
//	layout you choose in create().

sd sp, (a0)
ld sp, (a1)
mv t1, a2

ld a0, CTX_A0 * 8(sp)
ld a1, CTX_A1 * 8(sp)
ld a2, CTX_A2 * 8(sp)
ld a3, CTX_A3 * 8(sp)
ld a4, CTX_A4 * 8(sp)
ld a5, CTX_A5 * 8(sp)
ld a6, CTX_A6 * 8(sp)
ld a7, CTX_A7 * 8(sp)

ld s0, CTX_S0 * 8(sp)
ld s1, CTX_S1 * 8(sp)
ld s2, CTX_S2 * 8(sp)
ld s3, CTX_S3 * 8(sp)
ld s4, CTX_S4 * 8(sp)
ld s5, CTX_S5 * 8(sp)
ld s6, CTX_S6 * 8(sp)
ld s7, CTX_S7 * 8(sp)
ld s8, CTX_S8 * 8(sp)
ld s9, CTX_S9 * 8(sp)
ld s10, CTX_S10 * 8(sp)
ld s11, CTX_S11 * 8(sp)

ld ra, CTX_RA * 8(sp)
//ld sp, CTX_SP * 8(sp)

ld gp, CTX_GP * 8(sp)
ld tp, CTX_TP * 8(sp)

ld t0, CTX_T0 * 8(sp)
// ld t1, CTX_T1 * 8(sp)
ld t2, CTX_T2 * 8(sp)
ld t3, CTX_T3 * 8(sp)
ld t4, CTX_T4 * 8(sp)
ld t5, CTX_T5 * 8(sp)
ld t6, CTX_T6 * 8(sp)

// TODO: Jump to next function.
//	Note: there are two important values at play here --
//	the address of the process's function that we want to end up in
//	once we leave this ctxsw(), and the value we want to have in the
//	return address register.  Those are almost always the same value, except
//	for the very first time we ctxsw() to a new process, when they
//	are different.  See create() for more details.

//sd sp, (t2)

ld t0, CTX_PC * 8(sp)

addi sp, sp, 256

//jalr ra, t1

	//ld t0, CTX_PC*8(t1)
	beq t0, ra, switch

	csrw satp, t1 
	sfence.vma zero, zero 

	li t4, SSTATUS_S_MODE
	csrc sstatus, t4
	csrw sepc, t0

	sret
switch:
	ret

// Note: This ret line immediately returns from whence we came.  This
//	is here as a placeholder so that you don't crash your O/S when
//	you call ctxsw() before it is complete.  But, this line of code
//	is not actually a part of a completed solution.
//	ret



// ASSIGNMENT 5 TODO:
// Note: the following code is for the next assignment.  DO NOT uncomment it until you believe your trap handler is working in assignment
// Determine which mode we privilege mode the process should be in

//	ld t0, CTX_PC*8(**REGISTER THAT HOLDS PROCESS CONTEXT**)
//	beq t0, ra, switch
//	li t4, SSTATUS_S_MODE
//	csrc sstatus, t4
//	csrw sepc, t0
//
//	sret
//switch:
//	ret

	.end ctxsw
