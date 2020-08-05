/*
 * audio_out_wav.c
 * Copyright (C) 2010-2012 Josh "PH3NOM" Pearson < ph3nom.dcmc@gmail.com >
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of a52dec, a free ATSC A-52 stream decoder.
 * See http://liba52.sourceforge.net/ for updates.
 *
 * a52dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * a52dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <inttypes.h>

#include "a52.h"
#include "audio_out.h"
#include <audio_out_internal.h>

#include "LibA52.h"

#include "snddrv.h"
#include "fifo.h"

#define PCM_BUF_SIZE 1024*256*2

typedef struct wav_instance_s {
    ao_instance_t ao;
    int sample_rate;
    int set_params;
    int flags;
    uint32_t speaker_flags;
    int size;
} wav_instance_t;

static uint8  snddrv_buf[65537];  /* SNDDRV Buffer */
static uint8 *snddrv_ptr;            /* SNDDRV Ptr */
static int16_t int16_samples[256*6];
static FifoBuffer abuf;
static int first_stream = 1;
static int a52chans=0;

static volatile int A52_WAIT_START = 0;

static unsigned int SAMPLES = 0;

void LibA52_WaitStart()
{
    A52_WAIT_START = 1;
}

void LibA52_Start()
{
    A52_WAIT_START = 0;
}

unsigned int LibA52_PcmHave()
{
    return abuf.size;
}

unsigned int LibA52_PcmMax()
{
    return PCM_BUF_SIZE;
}


//#define DEBUG

void *a52_drv_callback( snd_stream_hnd_t hnd, int pcm_needed, int * pcm_done )
{
#ifdef DEBUG 
    printf("MPA DRV CB - REQ %i - HAVE %i\n", pcm_needed, abuf.size ); 
#endif
    while(A52_WAIT_START) thd_pass();
    
    while( abuf.size < pcm_needed )
    {
        if(!A52_DEC_STAT)
        {
            memset( snddrv_buf, 0, pcm_needed );
            snddrv_ptr = snddrv_buf;
            *pcm_done = pcm_needed;
     
            return snddrv_ptr;
        }
        //printf("MPA DRV CB - REQ %i - HAVE %i\n", pcm_needed, abuf.size ); 
        thd_pass();
    }
    
    A52_lock_mutex()
    memcpy( snddrv_buf, (uint8*)abuf.buf, pcm_needed );           
    abuf.size -= pcm_needed;
    memcpy( abuf.buf, abuf.buf+pcm_needed, abuf.size );
    A52_unlock_mutex()
    
    SAMPLES += pcm_needed;
    
    snddrv_ptr = snddrv_buf;
    *pcm_done = pcm_needed;
    
    return snddrv_ptr;
}

static void malloc_fifo()
{
    a52chans=0;     
    abuf.size=abuf.frames=0;
    abuf.buf = malloc(PCM_BUF_SIZE);
    first_stream=0;
}

static int wav_setup (ao_instance_t * _instance, int sample_rate, int * flags,
		      level_t * level, sample_t * bias)
{
    if(first_stream)
        malloc_fifo();
    
    SAMPLES = 0;
    
    wav_instance_t * instance = (wav_instance_t *) _instance;

    if ((instance->set_params == 0) && (instance->sample_rate != sample_rate))
	return 1;
    instance->sample_rate = sample_rate;

    if (instance->flags >= 0)
	*flags = instance->flags;
    *level = CONVERT_LEVEL;
    *bias = CONVERT_BIAS;

    return 0;
}

static int wav_channels (int flags, uint32_t * speaker_flags)
{
    static const uint16_t speaker_tbl[] = {
	3, 4, 3, 7, 0x103, 0x107, 0x33, 0x37, 4, 4, 3
    };
    static const uint8_t nfchans_tbl[] = {
	2, 1, 2, 3, 3, 4, 4, 5, 1, 1, 2
    };
    int chans;

    *speaker_flags = speaker_tbl[flags & A52_CHANNEL_MASK];
    chans = nfchans_tbl[flags & A52_CHANNEL_MASK];

    if (flags & A52_LFE) {
	*speaker_flags |= 8;	/* WAVE_SPEAKER_LOW_FREQUENCY */
	chans++;
    }

    return chans;	    
}


static int wav_play (ao_instance_t * _instance, int flags, convert_t * samples )
{
    uint32_t speaker_flags;

    if(!a52chans) a52chans = wav_channels (flags, &speaker_flags);

    convert2s16_wav (samples, int16_samples, flags);
    
    A52_lock_mutex()
    memcpy( (uint8*)abuf.buf+abuf.size, int16_samples, 256 * sizeof (int16_t) * a52chans );
    abuf.size+=256 * sizeof (int16_t) * a52chans;
    A52_unlock_mutex()

    return 0;
}

static void wav_close (ao_instance_t * _instance)
{   
    free(abuf.buf);
    abuf.size=0;
    first_stream = 1;
    
    wav_instance_t * instance = (wav_instance_t *) _instance;
    free(instance);
}

static ao_instance_t * wav_open (int flags)
{
    wav_instance_t * instance;

    instance = (wav_instance_t *) malloc (sizeof (wav_instance_t));
    if (instance == NULL)
	return NULL;

    instance->ao.setup = wav_setup;
    instance->ao.play = wav_play;
    instance->ao.close = wav_close;

    instance->sample_rate = 0;
    instance->set_params = 1;
    instance->flags = flags;
    instance->size = 0;

    return (ao_instance_t *) instance;
}

ao_instance_t * ao_wav_open (void)
{
    return wav_open (A52_STEREO);
}

ao_instance_t * ao_wavdolby_open (void)
{
    return wav_open (A52_DOLBY);
}

ao_instance_t * ao_wav6_open (void)
{
    return wav_open (-1);
}
