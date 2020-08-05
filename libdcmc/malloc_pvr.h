#ifndef MALLOC_PVR_H
#define MALLOC_PVR_H

#define PVR_MEM_MAX 1024*1024*4.5 /* 4.5mb For Textures Data */

unsigned int malloc_pvr( unsigned int size );

void free_pvr( char * addr, unsigned int size );

#endif
