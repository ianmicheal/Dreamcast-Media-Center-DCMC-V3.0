/*
** LibADX (C) Josh PH3NOM Pearson 2012-2013
** Thanks to BERO (C) 2001 for ADX decode algorithm
** 
** In this API, all the user needs to worry about is reading the bitstream into
** a buffer, then calling one of the LibADX decode functions on that buffer.
** The library will handle streaming the PCM samples to the sound hardware,
** in a seperate thread.
**
** The first bitstream chunk read by user must call LibADX_InitSFA_()
** Every chunk after that must call LibADX_DecodeSFA()
** When all of the bitstream has been read, simply call LibADX_ExitSFA()
**
*/ 

#ifndef ADX_H
#define ADX_H

#define ADX_CRI_SIZE 0x06
#define ADX_PAD_SIZE 0x0e
#define ADX_HDR_SIZE 0x2c
#define ADX_HDR_SIG  0x80
#define ADX_EXIT_SIG 0x8001

#define ADX_ADDR_START      0x02
#define ADX_ADDR_CHUNK      0x05
#define ADX_ADDR_CHAN       0x07
#define ADX_ADDR_RATE       0x08
#define ADX_ADDR_SAMP       0x0c
#define ADX_ADDR_TYPE       0x12
#define ADX_ADDR_LOOP       0x18
#define ADX_ADDR_SAMP_START 0x1c
#define ADX_ADDR_BYTE_START 0x20
#define ADX_ADDR_SAMP_END   0x24
#define ADX_ADDR_BYTE_END   0x28

#define	BASEVOL	0x4000

#define ADX_BUF_SIZE 1024*16
#define ADX_CHUNK_SIZE 18*56

typedef struct
{
    int sample_offset;              
    int chunk_size;
    int channels;
    int rate;
    int samples;
    int loop_type;
    int loop;
    int loop_start;
    int loop_end;
    int loop_samp_start;
    int loop_samp_end;
    int loop_samples;
}ADX_INFO;

typedef struct {
	int s1,s2;
} PREV;

#include <kos/mutex.h>

mutex_t * adx_mut;
#define ADX_create_mutex()  { adx_mut = mutex_create(); }
#define ADX_lock_mutex()    { mutex_lock( adx_mut );    }
#define ADX_unlock_mutex()  { mutex_unlock( adx_mut );  }
#define ADX_destroy_mutex() { mutex_destroy( adx_mut ); }

/* LibADX Public Function Definitions */
/* Return 1 on success, 0 on failure */

/* Parse the first chunk of SFA bitstream data and decode the chunk */
int LibADX_Init( unsigned char * adx_stream, unsigned int bytes );

/* Decode a chunk of SFA bitstream */
int LibADX_DecodeChunk( unsigned char * adx_stream, unsigned int bytes );

/* Exit the SFA decoder thread */
void LibADX_Exit();

/* Command the decoder to wait to start sound output */
void LibADX_WaitStart();

/* Command the decoder to start sound output ( i.e. video playback starts) */
void LibADX_Start();

/* Mutator:Volume */
int LibADX_volume_up();
int LibADX_volume_down();

/* Accessor:General */
unsigned int LibADX_Channels();
unsigned int LibADX_Rate();

#endif
