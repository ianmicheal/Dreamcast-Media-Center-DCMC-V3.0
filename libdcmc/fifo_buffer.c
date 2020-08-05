/*
** FIFO_BUFFER.c (C) Josh PH3NOM Pearson 2013
**
** Buffer routines for stream data based on packets
** Uses a circular queue for efficient memory usage
*/

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "fifo_buffer.h"

void FifoBufferInit( FIFOBUFFER * fbuf, unsigned int packet_size )
{
    unsigned int i;
    for( i=0; i<FIFO_SIZE; i++)
    {
        fbuf->PACKET[i] = malloc( packet_size );
        fbuf->PACKET_SIZE[i] = 0;
    }
    fbuf->rpos = fbuf->wpos = fbuf->num = 0;
}

void FifoBufferFree( FIFOBUFFER * fbuf )
{
    unsigned int i;
    for( i=0; i<FIFO_SIZE; i++)
         free( fbuf->PACKET[i] );
}

short FifoBufferHasRoom( FIFOBUFFER * fbuf )
{
    return fbuf->num < FIFO_SIZE;
}

short FifoBufferHasPacket( FIFOBUFFER * fbuf )
{
    return fbuf->num > 0;
}

unsigned int FifoBufferNextPacket( unsigned int i ) 
{
    if( i == FIFO_SIZE-1 )
        return 0;
    return i+1; 
}

short FifoBufferPush( FIFOBUFFER * fbuf, void * packet, unsigned int packet_size )
{
    if(!FifoBufferHasRoom(fbuf)) return 0;
    
    FIFO_MUTEX_LOCK()

    memcpy( fbuf->PACKET[fbuf->wpos], packet, packet_size );
    fbuf->PACKET_SIZE[fbuf->wpos] = packet_size;     
    ++fbuf->num;
    fbuf->wpos = FifoBufferNextPacket( fbuf->wpos );
    
    FIFO_MUTEX_UNLOCK()

    return 1;
}

unsigned int FifoBufferPop( FIFOBUFFER * fbuf, void * packet )
{
    if(!FifoBufferHasPacket(fbuf)) return 0;

    FIFO_MUTEX_LOCK()

    unsigned int packet_size = fbuf->PACKET_SIZE[fbuf->rpos];
    memcpy( packet, fbuf->PACKET[fbuf->rpos], packet_size );
    --fbuf->num;
    fbuf->rpos = FifoBufferNextPacket( fbuf->rpos );

    FIFO_MUTEX_UNLOCK()

    return packet_size;
}

unsigned int * FifoBufferTopPacket( FIFOBUFFER * fbuf )
{
    return fbuf->PACKET[fbuf->rpos];
}

unsigned int FifoBufferTopSize( FIFOBUFFER * fbuf )
{
    return fbuf->PACKET_SIZE[fbuf->rpos];
}

short FifoBufferTopRemove( FIFOBUFFER * fbuf )
{
    if(!FifoBufferHasPacket(fbuf)) return 0;
    
    FIFO_MUTEX_LOCK()    
    
    --fbuf->num;
    fbuf->rpos = FifoBufferNextPacket( fbuf->rpos );   

    FIFO_MUTEX_UNLOCK()

    return 1;
}

