#ifndef GL_RENDER_H
#define GL_RENDER_H

#include <arch/cache.h>

//#define RENDER_DMA /* Use DMA to transfer all PVR Commands */

#ifndef RENDER_DMA  

#define TA_SQ_ADDR (unsigned int *)(void *) \
		(0xe0000000 | (((unsigned long)0x10000000) & 0x03ffffe0))

#define QACRTA ((((unsigned int)0x10000000)>>26)<<2)&0x1c

inline void sq_cpy_ta( void *src, int n)
{
	unsigned int *d = TA_SQ_ADDR;
	unsigned int *s = src;
    
	/* fill/write queues as many times necessary */
	n>>=5;
	while(n--) {
		asm("pref @%0" : : "r" (s + 8)); /* prefetch 32 bytes for next loop */
		d[0] = *(s++);
		d[1] = *(s++);
		d[2] = *(s++);
		d[3] = *(s++);
		d[4] = *(s++);
		d[5] = *(s++);
		d[6] = *(s++);
		d[7] = *(s++);
		asm("pref @%0" : : "r" (d));
		d += 8;
	}
	/* Wait for both store queues to complete */
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;
}

#endif

#endif
