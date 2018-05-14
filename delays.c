#include "gpio.h"
#include "delays.h"

#define SYSTMR_LO  ((volatile unsigned int*)(MMIO_BASE+0x00003004))
#define SYSTMR_HI  ((volatile unsigned int*)(MMIO_BASE+0x00003008))

/* Wait n CPU cycles (ARM CPU only) */
void
wait_cycles(unsigned int n)
{
    if(n) {
	while(n--)
	    asm volatile("nop");
    }
}

/* Wait for n microseconds (ARM CPU only) */
void
wait_msec(unsigned int n)
{
    register unsigned long f, t, r;
    /* get current counter frequency */
    asm volatile("mrs %0, cntfrq_el0" : "=r"(f));
    /* read the current counter */
    asm volatile("mrs %0, cntpct_el0" : "=r"(t));
    /* calculate expire value for counter */
    t += ((f / 1000) * n) / 1000;
    do {
	asm volatile("mrs %0, cntpct_el0" : "=r"(r));
    } while(r < t);
}

/* Get system time counter */
unsigned long
get_system_timer()
{
    unsigned int h = -1, l;
    /* read MMIO are as two separate 32-bit reads */
    h = *SYSTMR_HI;
    l = *SYSTMR_LO;
    /* repeat if high word changed during read */
    if(h != *SYSTMR_HI) {
	h = *SYSTMR_HI;
	l = *SYSTMR_LO;
    }
    /* compose long int value */
    return ((unsigned long) h << 32) | l;
}

/* Wait for n microseconds (with BCM system timer) */
void
wait_msec_st(unsigned int n)
{
    unsigned long t = get_system_timer();
    /* we must check if it's non zero, since qemu doesn't
     * emulate system timer. Returning constant zero would
     * mean an infinite loop */
    if(t)
	while(get_system_timer() < t + n);
}
