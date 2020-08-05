/*
** FIFO_BUFFER.h (C) Josh PH3NOM Pearson 2013
**
** Buffer routines for stream data based on packets
** Uses a circular queue for efficient memory usage
*/

#ifndef FIFO_BUFFER
#define FIFO_BUFFER

#include <kos/mutex.h>

#define FIFO_SIZE 30

typedef struct
{
    unsigned int * PACKET[FIFO_SIZE];
    unsigned int PACKET_SIZE[FIFO_SIZE];
    unsigned int rpos, wpos, num;
} FIFOBUFFER;

mutex_t *FIFO_MUTEX;
#define FIFO_MUTEX_CREATE()  FIFO_MUTEX = mutex_create();
#define FIFO_MUTEX_DESTROY() mutex_destroy(FIFO_MUTEX);
#define FIFO_MUTEX_LOCK()    mutex_lock(FIFO_MUTEX);
#define FIFO_MUTEX_UNLOCK()  mutex_unlock(FIFO_MUTEX);

/* Initialize A Fifo Buffer, Allocating Packets of size packet_size */
void FifoBufferInit( FIFOBUFFER * fbuf, unsigned int packet_size );

/* Free Memory Allocated for Fifo Buffer */
void FifoBufferFree( FIFOBUFFER * fbuf );

/* Check if Fifo Buffer has room for another packet - 0 = False, 1 = True */
short FifoBufferHasRoom( FIFOBUFFER * fbuf );

/* Check if Fifo Buffer has a packet available - 0 = False, 1 = True */
short FifoBufferHasPacket( FIFOBUFFER * fbuf );

/* Push a Packet into the Fifo Buffer - 0 = Failure, 1 = Success */
short FifoBufferPush( FIFOBUFFER * fbuf, void * packet, unsigned int packet_size );

/* Pop a Packet from the Fifo Buffer - 0 = Failure, >0 = Size of Packet Returned */
unsigned int FifoBufferPop( FIFOBUFFER * fbuf, void * packet );

/* Return a pointer to the Packet at the top of the Fifo Buffer */
unsigned int * FifoBufferTopPacket( FIFOBUFFER * fbuf );

/* Return the size of the Packet at the top of the Fifo Buffer */
unsigned int FifoBufferTopSize( FIFOBUFFER * fbuf );

/* Remove the Packet top of the Fifo Buffer */
short FifoBufferTopRemove( FIFOBUFFER * fbuf );

#endif
