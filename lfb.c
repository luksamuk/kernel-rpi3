#include "uart.h"
#include "mbox.h"
#include "homer.h"

typedef struct {
    unsigned int  magic;
    unsigned int  version;
    unsigned int  headersize;
    unsigned int  flags;
    unsigned int  numglyph;
    unsigned int  bytesperglyph;
    unsigned int  height;
    unsigned int  width;
    unsigned char glyphs;
} __attribute__((packed)) psf_t;

/*
  Font label was found using readelf on .o file:
  $(READELF) -s font.o
*/
extern volatile unsigned char _binary_font_psf_start;

unsigned int width, height, pitch;
unsigned char *lfb;

/* Set screen resolution to 1024x768 */
void
lfb_init()
{
    mbox[0]  = 35 * 4;
    mbox[1]  = MBOX_REQUEST;

    mbox[2]  = 0x48003; /* set physical width/height */
    mbox[3]  = 8;
    mbox[4]  = 8;
    mbox[5]  = 1280;    /* framebuffer width */
    mbox[6]  = 720;     /* framebuffer height */

    mbox[7]  = 0x48004; /* set virtual width/height */
    mbox[8]  = 8;
    mbox[9]  = 8;
    mbox[10] = 1280;    /* framebuffer virt width */
    mbox[11] = 720;     /* framebuffer virt height */

    mbox[12] = 0x48009; /* set virtual offset */
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;       /* framebuffer x offset */
    mbox[16] = 0;       /* framebuffer y offset */

    mbox[17] = 0x48005; /* set depth */
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;      /* framebuffer depth */

    mbox[21] = 0x48006; /* set pixel order */
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;       /* RGB -- preferably, not BGR */

    mbox[25] = 0x40001; /* get framebuffer; gets alignment on request */
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;    /* framebuffer pointer */
    mbox[29] = 0;       /* framebuffer size */

    mbox[30] = 0x40008; /* get pitch */
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;       /* framebuffer pitch */

    mbox[34] = MBOX_TAG_LAST;

    if(mbox_call(MBOX_CH_PROP) && (mbox[20] == 32) && mbox[28] != 0) {
	mbox[28] &= 0x3FFFFFFF;
	width  = mbox[5];
	height = mbox[6];
	pitch  = mbox[33];
	lfb = (void*)((unsigned long)mbox[28]);
    } else {
	uart_puts("Unable to set screen resolution to 1280x720x32\n");
    }
}


/* Show a picture */
void
lfb_showpicture()
{
    int x, y;
    unsigned char *ptr = lfb;
    char *data = homer_data, pixel[4];

    ptr += (height - homer_height) / ((2 * pitch) + ((width - homer_width) * 2));
    for(y = 0; y < homer_height; y++) {
	for(x = 0; x < homer_width; x++) {
	    HEADER_PIXEL(data, pixel);
	    *((unsigned int*)ptr) = *((unsigned int*)&pixel);
	    ptr += 4;
	}
	ptr += pitch - homer_width * 4;
    }
}


/* Display a string */
void
lfb_print(int x, int y, char *s)
{
    /* get the font */
    psf_t *font = (psf_t*)&_binary_font_psf_start;
    /* if it is not zero, draw next character */
    while(*s) {
	/* get glyph offset. Needs adjust to support unicode. */
	unsigned char *glyph = (unsigned char*)&_binary_font_psf_start
	    + font->headersize + ((*((unsigned char*)s) < font->numglyph ? *s : 0)
				  * font->bytesperglyph);
	/* calculate offset on screen */
	int offs = (y * font->height * pitch) + (x * (font->width + 1) * 4);

	int i, j,
	    line, mask,
	    bytesperline = (font->width + 7) / 8;

	/* handle carriage return */
	if(*s == '\r') {
	    x = 0;
	} else if(*s == '\n') { /* handle newline */
	    x = 0; y++;
	} else { /* display a character */
	    for(j = 0; j < font->height; j++) {
		/* display one row */
		line = offs;
		mask = 1 << (font->width - 1);
		for(i = 0; i < font->width; i++) {
		    /* If bit is set, we use white color.
		       Otherwise, use black */
		    *((unsigned int*)(lfb + line)) = ((int)*glyph) & mask ? 0xFFFFFF : 0;
		    mask >>= 1;
		    line += 4;
		}
		/* adjust to next line */
		glyph += bytesperline;
		offs += pitch;
	    }
	    x++;
	}
	/* next character */
	s++;
    }
}
