/*
**
** SNDDRV.c (C) Josh 'PH3NOM' Pearson 2011-2013
**
*/
/*
** To anyone looking at this code:
**
** This driver runs in its own thread on the sh4.
** 
** When the AICA driver requests more samples,
** it will signal sndbuf_status=SNDDRV_STATUS_NEEDBUF 
** and assign the number of requested samples to pcm_needed.
**
** The decoders need to check sndbuf_status,
** when more samples are requested by the driver ** the decoders will loop
** decoding into pcm_buffer untill pcm_bytes==pcm_needed
** at that point the decoder signals sndbuf_status=SNDDRV_STATUS_HAVEBUF
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kos/thread.h>

#include "snddrv.h"

static uint32 hz;
static uint16 chn;

static int snddrv_vol = 0xFF;

static snddrv_cb *drv_callback = NULL;

/* Increase the Sound Driver volume */
int snddrv_volume_up()
{    
    if( snddrv_vol <= 245 )
    {
        snddrv_vol += 10;
  	    snd_stream_volume(shnd, snddrv_vol);
    }
    return snddrv_vol;
}

/* Decrease the Sound Driver volume */
int snddrv_volume_down()
{    
    if( snddrv_vol >= 10 )
    {
        snddrv_vol -= 10;
  	    snd_stream_volume(shnd, snddrv_vol);
    }
    return snddrv_vol;
}

/* Exit the Sound Driver */
int snddrv_exit()
{    
    if( snddrv_status != SNDDRV_STATUS_NULL ) {  
        snddrv_status = SNDDRV_STATUS_DONE;
        sndbuf_status = SNDDRV_STATUS_BUFEND;
       
        while( snddrv_status != SNDDRV_STATUS_NULL ) 
          thd_pass();
            
    }
          
    memset( pcm_buffer, 0, 65536+16384);
    pcm_bytes = 0;
    pcm_needed = 0;

    return snddrv_status;
}   

int snddrv_exit_cb()
{
    if(snddrv_status == SNDDRV_STATUS_NULL)
        return 0;
    
    snddrv_status = SNDDRV_STATUS_NULL;    
    snd_stream_destroy(shnd);
	snd_stream_shutdown();
	
    return 1;
}

/* Signal how many samples the AICA needs, then wait for the deocder to produce them */
static void *snddrv_callback(snd_stream_hnd_t hnd, int len, int * actual)
{       
    /* Signal the Decoder thread how many more samples are needed */
    pcm_needed = len;       
    sndbuf_status = SNDDRV_STATUS_NEEDBUF;

    /* Wait for the samples to be ready */
    while( sndbuf_status != SNDDRV_STATUS_HAVEBUF && sndbuf_status != SNDDRV_STATUS_BUFEND )  
           thd_pass();

    pcm_ptr = pcm_buffer;
    *actual = len;
	
	return pcm_ptr;
}

static void snddrv_thread()
{
    //printf("SNDDRV: Rate - %i, Channels - %i\n", hz, chn);
    shnd = snd_stream_alloc(drv_callback, SND_STREAM_BUFFER_MAX/4);
	
    snd_stream_start(shnd, hz, chn-1);
    snddrv_status = SNDDRV_STATUS_STREAMING;

	while( snddrv_status != SNDDRV_STATUS_DONE && snddrv_status != SNDDRV_STATUS_ERROR )
    {       
		snd_stream_poll(shnd);		
		thd_sleep(20);
	}
    snddrv_status = SNDDRV_STATUS_NULL;

    snd_stream_destroy(shnd);
	snd_stream_shutdown();

    //printf("SNDDRV: Finished\n");
}

/* Start the AICA Sound Stream Thread */
int snddrv_start( int rate, int chans )
{    
    hz = rate;
    chn = chans;
    if( chn < 1 || chn > 2)
    {
        printf("SNDDRV: ERROR - Exceeds maximum channels\n");
        return -1;
    }
    
    printf("SNDDRV: Checking Status\n");

    while( snddrv_status != SNDDRV_STATUS_NULL )
          thd_pass(); 
    
    drv_callback = snddrv_callback;
    
    snddrv_status = SNDDRV_STATUS_INITIALIZING;

    snd_stream_init();
        
    kthread_t * snddrv;
    printf("SNDDRV: Creating Driver Thread\n");
    snddrv = thd_create( snddrv_thread, NULL );
    
    return snddrv_status;

}

int snddrv_start_cb( int rate, int chans, snddrv_cb cb )
{       
    hz = rate;
    chn = chans;
    if( chn < 1 || chn > 2)
    {
        printf("SNDDRV: ERROR - Exceeds maximum channels\n");
        return -1;
    }
    
    while( snddrv_status != SNDDRV_STATUS_NULL )
          thd_pass(); 

    drv_callback = cb;

    snddrv_status = SNDDRV_STATUS_INITIALIZING;

    snd_stream_init();
        
    shnd = snd_stream_alloc(drv_callback, SND_STREAM_BUFFER_MAX/4);
	
    snd_stream_start(shnd, hz, chn-1);
    snddrv_status = SNDDRV_STATUS_STREAMING;
  
    return snddrv_status;

}

int snddrv_start_cb_thd( int rate, int chans, snddrv_cb cb )
{       
    hz = rate;
    chn = chans;
    if( chn < 1 || chn > 2)
    {
        printf("SNDDRV: ERROR - Exceeds maximum channels\n");
        return -1;
    }
    
    while( snddrv_status != SNDDRV_STATUS_NULL )
          thd_pass(); 

    drv_callback = cb;

    snddrv_status = SNDDRV_STATUS_INITIALIZING;

    snd_stream_init();
        
    kthread_t * snddrv;
    //printf("SNDDRV: Creating Driver Thread\n");
    snddrv = thd_create( snddrv_thread, NULL );
  
    return snddrv_status;
}

/* This is probably the final version of the driver, based on a handle */

/* Set the Sound Driver volume */
int snddrv_hnd_volume_set( snddrv_hnd * drvhnd, unsigned short vol )
{           
    drvhnd->vol=vol;
    
    snd_stream_volume(drvhnd->shnd, drvhnd->vol);
    
    return drvhnd->vol;
}

/* Increase the Sound Driver volume */
int snddrv_hnd_volume_up( snddrv_hnd * drvhnd )
{    
    if( drvhnd->vol <= 245 )
    {
        drvhnd->vol += 10;
  	    snd_stream_volume(drvhnd->shnd, drvhnd->vol);
    }
    return drvhnd->vol;
}

/* Decrease the Sound Driver volume */
int snddrv_hnd_volume_down( snddrv_hnd * drvhnd )
{    
    if( drvhnd->vol >= 10 )
    {
        drvhnd->vol -= 10;
  	    snd_stream_volume(drvhnd->shnd, drvhnd->vol);
    }
    return drvhnd->vol;
}

/* Exit the Sound Driver */
void snddrv_hnd_exit( snddrv_hnd * drvhnd )
{
    if(drvhnd->stat == SNDDRV_STATUS_NULL)
       return;
       
    drvhnd->stat = SNDDRV_STATUS_NULL;
    thd_sleep(50);
    
    snd_stream_destroy(drvhnd->shnd);
	snd_stream_shutdown();
}

void snddrv_hnd_cb( snddrv_hnd * drvhnd  )
{
    snd_stream_poll(drvhnd->shnd);
}

int snddrv_hnd_start( snddrv_hnd * drvhnd )
{
    if(drvhnd->stat != SNDDRV_STATUS_NULL)
       return 0;
    
    if( drvhnd->chan < 1 || drvhnd->chan > 2 )
    {
        printf("SNDDRV: ERROR - Invalid Number of channels\n");
        return 0;
    }
#ifdef DEBUG    
    printf("SNDDRV: %iHz - %i channel\n", drvhnd->rate, drvhnd->chan );
#endif
    drvhnd->vol = 0xFF;  
      
    snd_stream_init();
        
    drvhnd->shnd = snd_stream_alloc(drvhnd->drv_cb, SND_STREAM_BUFFER_MAX/4);
	
	drvhnd->stat = SNDDRV_STATUS_STREAMING;
	
    snd_stream_start(drvhnd->shnd, drvhnd->rate, drvhnd->chan-1);
    
    return 1;
}
