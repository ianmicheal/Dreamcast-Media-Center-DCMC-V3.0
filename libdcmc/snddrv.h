/*
**
** This File is a part of Dreamcast Media Center
** (C) Josh "PH3NOM" Pearson 2011-2012
**
*/

#ifndef SNDDRV_H
#define SNDDRV_H

#include <kos/mutex.h>
#include <dc/sound/stream.h>

/* Keep track of things from the Driver side */
volatile int snddrv_status;
#define SNDDRV_STATUS_NULL         0x00
#define SNDDRV_STATUS_INITIALIZING 0x01
#define SNDDRV_STATUS_READY        0x02
#define SNDDRV_STATUS_STREAMING    0x03
#define SNDDRV_STATUS_DONE         0x04
#define SNDDRV_STATUS_ERROR        0x05

/* Keep track of things from the Decoder side */
volatile int snddec_status;
#define SNDDEC_STATUS_NULL         0x00
#define SNDDEC_STATUS_INITIALIZING 0x01
#define SNDDEC_STATUS_READY        0x02
#define SNDDEC_STATUS_STREAMING    0x03
#define SNDDEC_STATUS_PAUSING      0x04
#define SNDDEC_STATUS_PAUSED       0x05
#define SNDDEC_STATUS_RESUMING     0x06
#define SNDDEC_STATUS_DONE         0x07
#define SNDDEC_STATUS_ERROR        0x08

/* Keep track of the buffer status from both sides*/
volatile int sndbuf_status;
#define SNDDRV_STATUS_NULL         0x00
#define SNDDRV_STATUS_NEEDBUF      0x01
#define SNDDRV_STATUS_HAVEBUF      0x02
#define SNDDRV_STATUS_BUFEND       0x03

#define SEEK_LEN 16384*48

#define PCM_BUF_SIZE 1024*256*2

int pcm_bytes, pcm_needed;

unsigned int pcm_buffer[65536+16384];
unsigned int *pcm_ptr;

typedef void * (*snddrv_cb)(snd_stream_hnd_t, int, int*); 

static snd_stream_hnd_t shnd;
#define SNDDRV_POLL() snd_stream_poll(shnd);	

volatile float ATS;

mutex_t * snddrv_mut;
#define SNDDRV_create_mutex()  { snddrv_mut = mutex_create(); }
#define SNDDRV_lock_mutex()    { mutex_lock( snddrv_mut );    }
#define SNDDRV_unlock_mutex()  { mutex_unlock( snddrv_mut );  }
#define SNDDRV_destroy_mutex() { mutex_destroy( snddrv_mut ); }

typedef struct
{
     long rate;
     int chan;
     unsigned int samples_done;
     short stat;
     unsigned short vol;
     snd_stream_hnd_t shnd;
     snddrv_cb drv_cb;
     unsigned char drv_buf[65536+16384];
     unsigned char *drv_ptr;
} snddrv_hnd;


/* SNDDRV (C) AICA Audio Driver */
struct snddrv {
       int rate;
       int channels;
       int pcm_bytes;
       int pcm_needed;
       volatile int drv_status;
       volatile int dec_status;
       volatile int buf_status;
       unsigned int pcm_buffer[65536+16384];
       unsigned int *pcm_ptr;
}snddrv;

typedef struct {
     char artist[128];
     char  title[128];
     char  track[128];
     char  album[128];
     char  genre[128];
}ID3_Data;

#define min(a,b) ( (a) < (b) ? (a) : (b) )

#define MAX_CHANNELS 6 /* make this higher to support files with
                          more channels for LibFAAD */

/* MicroSoft channel definitions */
#define SPEAKER_FRONT_LEFT             0x1
#define SPEAKER_FRONT_RIGHT            0x2
#define SPEAKER_FRONT_CENTER           0x4
#define SPEAKER_LOW_FREQUENCY          0x8
#define SPEAKER_BACK_LEFT              0x10
#define SPEAKER_BACK_RIGHT             0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define SPEAKER_BACK_CENTER            0x100
#define SPEAKER_SIDE_LEFT              0x200
#define SPEAKER_SIDE_RIGHT             0x400
#define SPEAKER_TOP_CENTER             0x800
#define SPEAKER_TOP_FRONT_LEFT         0x1000
#define SPEAKER_TOP_FRONT_CENTER       0x2000
#define SPEAKER_TOP_FRONT_RIGHT        0x4000
#define SPEAKER_TOP_BACK_LEFT          0x8000
#define SPEAKER_TOP_BACK_CENTER        0x10000
#define SPEAKER_TOP_BACK_RIGHT         0x20000
#define SPEAKER_RESERVED               0x80000000
        
/* SNDDRV Function Protocols */
int snddrv_start( int rate, int chans );
int snddrv_exit();
int snddrv_volume_up();
int snddrv_volume_down();

int  snddrv_start_cb( int rate, int chans, snddrv_cb cb );
void snddrv_kick();
int  snddrv_exit_cb( );

/* SNDDRV HND OBJECT FUNCTIONS */

int snddrv_hnd_start( snddrv_hnd * drvhnd );

void snddrv_hnd_cb( snddrv_hnd * drvhnd  );

void snddrv_hnd_exit( snddrv_hnd * drvhnd );

int snddrv_hnd_volume_set( snddrv_hnd * drvhnd, unsigned short vol );

int snddrv_hnd_volume_down( snddrv_hnd * drvhnd );

int snddrv_hnd_volume_up( snddrv_hnd * drvhnd );

#endif
