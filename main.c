#include "uart.h"
#include "mbox.h"

void
main()
{
    /* Setup serial console */
    uart_init();

    /* Get board's unique serial number with a mailbox call */
    mbox[0] = 8 * 4;              /* Message length */
    mbox[1] = MBOX_REQUEST;       /* Flag as request */

    mbox[2] = MBOX_TAG_GETSERIAL; /* Request serial number */
    mbox[3] = 8;                  /* Buffer size */
    mbox[4] = 8;
    mbox[5] = 0;                  /* Clear output buffer */
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    /* Send message to GPU; Receive answer */
    if(mbox_call(MBOX_CH_PROP)) {
	uart_puts("My serial number is: ");
	uart_hex(mbox[6]);
	uart_hex(mbox[5]);
	uart_puts("\n");
    } else {
	uart_puts("Unable to query board serial number\n");
    }

    /* Echo everything back */
    while(1) {
	uart_send(uart_getc());
    }
}
