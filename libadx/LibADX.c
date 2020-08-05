/*
   This File is a part of Dreamcast Media Center
   ADXCORE (c)2012-2013 Josh "PH3NOM" Pearson
   ph3nom.dcmc@gmail.com
   decoder algorithm optimized for stereo streams (C) PH3NOM
   decoder algorithm: adv2wav(c)2001 BERO
	http://www.geocities.co.jp/Playtown/2004/
	bero@geocities.co.jp
	adx info from: http://ku-www.ss.titech.ac.jp/~yatsushi/adx.html
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kos/thread.h>

#include "LibADX.h"
#include "snddrv.h"
#include "fifo.h"

/* A few global vars, should move this into a struct/handle */
static ADX_INFO ADX_Info;
static unsigned char *sfa_buf;
static unsigned char *sfa_ptr;
static unsigned int  sfa_bytes=0;
static PREV prev[2];
static snddrv_hnd *sndhnd;
static FifoBuffer * pcm_buf;

static volatile int ADX_DEC_STAT=0;
static volatile int ADX_WAIT_START = 0;

static void *adx_drv_callback( snd_stream_hnd_t hnd, int pcm_needed, int * pcm_done );
static void adx_drv_thd();

int read_be16(unsigned char *buf)     /* ADX File Format is Big Endian */
{
	return (buf[0]<<8)|buf[1];
}

long read_be32(unsigned char *buf)
{
	return (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
}

void LibADX_WaitStart()
{
    ADX_WAIT_START = 1;
}

void LibADX_Start()
{
    ADX_WAIT_START = 0;
}

unsigned int LibADX_Channels()
{
    return ADX_Info.channels;
}

unsigned int LibADX_Rate()
{
    return ADX_Info.rate;
}

int LibADX_volume_up()
{
    return snddrv_hnd_volume_up(sndhnd);
}    

int LibADX_volume_down()
{
    return snddrv_hnd_volume_down(sndhnd);
}

unsigned int LibADX_PcmHave()
{
    return pcm_buf->size;
}         

//#define DEBUG

/* This callback will handle the AICA Driver */
static void *adx_drv_callback( snd_stream_hnd_t hnd, int pcm_needed, int * pcm_done )
{   
#ifdef DEBUG 
    printf("MPA DRV CB - REQ %i - HAVE %i\n", pcm_needed, pcm_buf->size ); 
#endif
    while(ADX_WAIT_START) thd_pass();
 
    while( pcm_buf->size < pcm_needed )
    {
        if(!ADX_DEC_STAT)
        {
            memset( sndhnd->drv_buf, 0, pcm_needed );
            sndhnd->drv_ptr = sndhnd->drv_buf;
            *pcm_done = pcm_needed;
            return sndhnd->drv_ptr;
        }
        thd_pass();
    }       
    
    ADX_lock_mutex()
    memcpy( sndhnd->drv_buf, pcm_buf->buf, pcm_needed );
    pcm_buf->size -= pcm_needed;
    memmove( pcm_buf->buf, pcm_buf->buf+pcm_needed, pcm_buf->size );
    ADX_unlock_mutex()
        
    sndhnd->drv_ptr = sndhnd->drv_buf;
    *pcm_done = pcm_needed;   
        
    return sndhnd->drv_ptr;
}

static void adx_drv_thd()
{
    snddrv_hnd_start( sndhnd );   
    while(sndhnd->stat != SNDDRV_STATUS_NULL)
    {
        snddrv_hnd_cb( sndhnd );
        thd_sleep(50);    
    }
}

/* Convert ADX samples to PCM samples */
static void adx_to_pcm(short *out,unsigned char *in,PREV *prev)
{
	int scale = ((in[0]<<8)|(in[1]));
	int i;
	int s0,s1,s2,d;
    int sf = BASEVOL*scale;// precalculate to save 16*2 muls per chunk (C) PH3NOM
     
	in+=2;
	s1 = prev->s1;
	s2 = prev->s2;
	for(i=0;i<16;i++) {
		d = in[i]>>4;
		if (d&8) d-=16;
		s0 = (sf*d + 0x7298*s1 - 0x3350*s2)>>14;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		s2 = s1;
		s1 = s0;

		d = in[i]&15;
		if (d&8) d-=16;
		s0 = (sf*d + 0x7298*s1 - 0x3350*s2)>>14;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		s2 = s1;
		s1 = s0;
	}
	prev->s1 = s1;
	prev->s2 = s2;

}

static void adx_to_pcm_stride(short *out,unsigned char *in,PREV *prev)
{
	int scale = ((in[0]<<8)|(in[1]));
	int i;
	int s0,s1,s2,d;
    int sf = BASEVOL*scale; // precalculate to save 16*2 muls per chunk (C) PH3NOM
     
	in+=2;
	s1 = prev->s1;
	s2 = prev->s2;
	for(i=0;i<16;i++) {
		d = in[i]>>4;
		if (d&8) d-=16;
		s0 = (sf*d + 0x7298*s1 - 0x3350*s2)>>14;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		++out; // Add pointer stride to allow stereo interleave (C) PH3NOM
		s2 = s1;
		s1 = s0;

		d = in[i]&15;
		if (d&8) d-=16;
		s0 = (sf*d + 0x7298*s1 - 0x3350*s2)>>14;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		++out; // Add pointer stride to allow stereo interleave (C) PH3NOM
		s2 = s1;
		s1 = s0;
	}
	prev->s1 = s1;
	prev->s2 = s2;

}

/* Read and parse the SFA Stream header and return begining sample offset */
static int LibADX_Parse( unsigned char *buf )
{
	if(buf[0]!=ADX_HDR_SIG ) return -1;           /* Check ADX File Signature */
	
    /* Parse the ADX File header */
	ADX_Info.sample_offset = read_be16(buf+ADX_ADDR_START)-2;
	ADX_Info.chunk_size    = buf[ADX_ADDR_CHUNK];
    ADX_Info.channels      = buf[ADX_ADDR_CHAN];
	ADX_Info.rate          = read_be32(buf+ADX_ADDR_RATE);
	ADX_Info.samples       = read_be32(buf+ADX_ADDR_SAMP);
	ADX_Info.loop_type     = buf[ADX_ADDR_TYPE]; 
    
    ADX_Info.chunk_size *= ADX_Info.channels;
    
	return ADX_Info.sample_offset;
}

void LibADX_Exit()
{
    ADX_DEC_STAT=0; thd_sleep(50); 
    ADX_WAIT_START=0;
    
    ADX_destroy_mutex()
    
    free( pcm_buf->buf );
    free( pcm_buf );

    snddrv_hnd_exit( sndhnd );                       /* Exit the Sound Driver */
    free(sndhnd);
    
    free(sfa_buf);

    printf("LibADX: EXIT()\n");
}

int LibADX_Init( unsigned char * adx_stream, unsigned int bytes )
{
    int hdr_size;
    sfa_ptr=adx_stream;
    sfa_buf=malloc(ADX_BUF_SIZE);
    sfa_bytes = 0;
    
    prev[0].s1 = prev[0].s2 = 0;
    prev[1].s1 = prev[1].s2 = 0;
    
    pcm_buf = malloc( sizeof(FifoBuffer) );
    memset( pcm_buf, 0, sizeof(FifoBuffer) );
    pcm_buf->buf = malloc(PCM_BUF_SIZE);
    
    printf("LibADX: Checking Status\n");
    if( snddec_status!=SNDDEC_STATUS_NULL )
    {
        printf("LibADX: Already Running in another process!\n");
        return 0; 
    } 
	
	hdr_size = LibADX_Parse( sfa_ptr ); 
    if( hdr_size < 1) /* Make sure we are working with an ADX stream */
    {
        printf("LibADX: Invalid File Header\n");
        return 0;
    }

	if ( memcmp(sfa_ptr+hdr_size,"(c)CRI",6) )
    {
		printf("Invalid ADX header!\n");
		return -1;
	}
	
	if(ADX_DEC_STAT)
	    return -1;
    
    ADX_DEC_STAT=1;

    printf("LibADX: %iHz, %ich\n",ADX_Info.rate,ADX_Info.channels);
    
    sndhnd = malloc( sizeof( snddrv_hnd ) );
    memset( sndhnd, 0 , sizeof(snddrv_hnd) );
    
    sndhnd->rate = ADX_Info.rate;
	sndhnd->chan = ADX_Info.channels;
	sndhnd->drv_cb = adx_drv_callback;
    
    ADX_create_mutex()

    LibADX_DecodeChunk( adx_stream+hdr_size+6, bytes-(hdr_size+6) );
    
    thd_create( adx_drv_thd, NULL );
    
    return 1;    
}

/* Decode the ADX chunk into internal PCM buffer */
int LibADX_DecodeChunk( unsigned char * adx_stream, unsigned int bytes )
{
    unsigned int adx_bytes = sfa_bytes+bytes;  /* End of bitstream buffer */
    short * ptr;
	
    memcpy( sfa_buf+sfa_bytes, adx_stream, bytes );
    sfa_bytes += bytes;
    sfa_ptr = sfa_buf;
    
    switch( ADX_Info.channels )
    {
        case 1:
           ADX_lock_mutex()
           while( sfa_bytes >= ADX_Info.chunk_size )
           {
                adx_to_pcm(pcm_buf->buf+pcm_buf->size,sfa_ptr,prev);

                pcm_buf->size+=64;
                
                sfa_ptr+=ADX_Info.chunk_size;
                sfa_bytes-=ADX_Info.chunk_size;           
           }
           ADX_unlock_mutex()
           break;
    
        case 2:
           ADX_lock_mutex()  
           while( sfa_bytes >= ADX_Info.chunk_size )
           {
	           ptr = pcm_buf->buf+pcm_buf->size;
	           adx_to_pcm_stride(ptr,sfa_ptr,prev);
	           adx_to_pcm_stride(ptr+1,sfa_ptr+ADX_Info.chunk_size,prev+1);
               
               pcm_buf->size+=128;
	           
  		       sfa_ptr+=ADX_Info.chunk_size;
  		       sfa_bytes-=ADX_Info.chunk_size;
           }
           ADX_unlock_mutex()	
           break;
    }
    
    memcpy( sfa_buf, sfa_buf+(adx_bytes-sfa_bytes), sfa_bytes );
        
    return 1;
}
