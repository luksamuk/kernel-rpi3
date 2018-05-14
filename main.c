#include "uart.h"
#include "mbox.h"
#include "rand.h"
#include "delays.h"

void test_mbox();
void test_rng();
void test_delay();



void
main()
{
    /* Setup serial console */
    uart_init();
    rand_init();

    test_mbox();

    test_rng();

    test_delay();

    /* Echo everything back */
    while(1) {
	uart_send(uart_getc());
    }
}


/* --- */

void
test_mbox()
{
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
}

void
test_rng()
{
    /* test rng */
    uart_puts("Here, have a random number: ");
    uart_hex(rand(0, 4294967295));
    uart_puts("\n");
}

void
test_delay()
{
    uart_puts("Waiting 10000000 CPU cycles (ARM CPU): ");
    wait_cycles(10000000);
    uart_puts("OK\n");

    uart_puts("Waiting 10000000 CPU microseconds (ARM CPU): ");
    wait_msec(10000000);
    uart_puts("OK\n");

    uart_puts("Waiting 10000000 CPU microseconds (BCM System Timer): ");
    if(get_system_timer() == 0) {
	uart_puts("Not available\n");
    } else {
	wait_msec_st(1000000);
	uart_puts("OK\n");
    }
}
