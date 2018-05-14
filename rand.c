#include "gpio.h"
#include "rand.h"

#define RNG_CTRL      ((volatile unsigned int*)(MMIO_BASE+0x00104000))
#define RNG_STATUS    ((volatile unsigned int*)(MMIO_BASE+0x00104004))
#define RNG_DATA      ((volatile unsigned int*)(MMIO_BASE+0x00104008))
#define RNG_INT_MASK  ((volatile unsigned int*)(MMIO_BASE+0x00104010))

/* Initialize the RNG */
void
rand_init()
{
    *RNG_STATUS   =  0x40000;
    *RNG_INT_MASK |= 1; /* mask interrupt */
    *RNG_CTRL     |= 1; /* enable */
    /* await entropy */
    while(!((*RNG_STATUS) >> 24))
	asm volatile("nop");
}

/* Return a random number [min..max] */
unsigned int
rand(unsigned int min, unsigned int max)
{
    return (*RNG_DATA) % (max - min) + min;
}