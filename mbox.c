#include "gpio.h"

/* Mailbox message buffer */
volatile unsigned int __attribute__((aligned(16))) mbox[36];

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000


/* Make a mailbox call. Returns 0 on failure and non-zero on success */
int
mbox_call(unsigned char ch)
{
    unsigned int r;

    /* Await write permission */
    do {
	asm volatile("nop");
    } while(*MBOX_STATUS & MBOX_FULL);

    /* Write address of message to the mailbox with the channel identifier */
    *MBOX_WRITE = (((unsigned int)((unsigned long)&mbox) & ~0xF) | (ch & 0xF));

    /* Await response */
    while(1) {
	/* Await response arrival */
	do {
	    asm volatile("nop");
	} while(*MBOX_STATUS & MBOX_EMPTY);
	r = *MBOX_READ;

	/* Check whether it truly is a response */
	if((unsigned char)(r & 0xF) == ch &&
	   (r & ~0xF) == (unsigned int)((unsigned long)&mbox))
	    /* Check whether the response is valid */
	    return mbox[1] == MBOX_RESPONSE;
    }

    /* Couldn't do it */
    return 0;
}
