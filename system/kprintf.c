/**
 * COSC 3250 - Project 1
 * This program seeks to create the functionality of 
 * putchar, getchar, ungetchar, and a checkc function to check
 * if a character is ready to be collected
 * This file uses a little bit of chat gpt to check the 
 * functions we wrote for bugs
 * @author Josh Wild, Nathan Rusch, chat gpt
 * Instructor Brylow
 * TA-BOT:MAILTO joshua.wild@marquette.edu
 */

#include <xinu.h>

#define UNGETMAX 10             /* Can un-get at most 10 characters. */
static unsigned char ungetArray[UNGETMAX];
unsigned int  bufp = 0;

syscall kgetc() {
	volatile struct ns16550_uart_csreg *regptr;
	regptr = (struct ns16550_uart_csreg *)UART_BASE;

	// TODO: First, check the unget buffer for a character.
	//       Otherwise, check UART line status register, and
	//       once there is data ready, get character c.
	unsigned char c = 0;    // initialize a char that's unsigned to account for all possible chars
	if (bufp > 0) {         // if bufp > 0 there is stuff in ungetArray
        bufp --;            // decrement bufp first since bufp is at the index of the first empty slot
	c = ungetArray[bufp];   // set the ungetArray at index bufp to c
	return c;               // return c
    } 
    // if there is nothing in the ungetArray, wait for the register to have data in it that is ready
    while ((regptr -> lsr & UART_LSR_DR) == 0) {
		;
    }
    // set c to be what's in the register once there is data there
    c = regptr -> rbr;

    return c;   // return c
}

/**
 * kcheckc - check to see if a character is available.
 * @return true if a character is available, false otherwise.
 */
syscall kcheckc(void) {
    volatile struct ns16550_uart_csreg *regptr;
    regptr = (struct ns16550_uart_csreg *)UART_BASE;

    // TODO: Check the unget buffer and the UART for characters.
   
    if (bufp > 0) {     // if bufp is greater than 0 there is something in ungetArray
    	return TRUE;
    }
	else if (regptr -> lsr & UART_LSR_DR) { // if this is true, there is data in the register that is ready
		return TRUE;
    }
    // if neither of the above cases is true, return false
	return FALSE;
	
   
}	

/**
 * kungetc - put a serial character "back" into a local buffer.
 * @param c character to unget.
 * @return c on success, SYSERR on failure.
 */
syscall kungetc(unsigned char c) {
    // TODO: Check for room in unget buffer, put the character in or discard.
    if (bufp < UNGETMAX) {       // if buffer is not full
		ungetArray[bufp] = c;    // set the next index of the ungetArray to c
		bufp ++;                 // increment buffer
		// kprintf("kungetc() status: success\r\n");
		return c;
	}
    else {      // if ungetArray is full return a system error
		// kprintf("kungetc() status: error\r\n");
		return SYSERR;
	}
}

syscall kputc(uchar c) {
    volatile struct ns16550_uart_csreg *regptr;
    regptr = (struct ns16550_uart_csreg *)UART_BASE;

    // TODO: Check UART line status register.
    // Once the Transmitter FIFO is empty, send character c.

    // 
    while ((regptr -> lsr & UART_LSR_THRE) == 0) {
        ;
    }
    // once it is ready, set the output register to be c;
    regptr -> thr = c;

    return c;
}

syscall kprintf(const char *format, ...)
{
    int retval;
    va_list ap;

    va_start(ap, format);
    retval = _doprnt(format, ap, (int (*)(long, long))kputc, 0);
    va_end(ap);
    return retval;
}