/*
**
** This file is a part of Dreamcast Media Center
** (C) Josh PH3NOM Pearson 2011
**
*/
/*
 * a52dec.c
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

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifdef HAVE_IO_H
#include <fcntl.h>
#include <io.h>
#endif

#include <inttypes.h>

#include "a52.h"
#include "audio_out.h"
#include "mm_accel.h"

#include "snddrv.h"

#define BUFFER_SIZE 4096
static uint8_t buffer[BUFFER_SIZE];
static uint8_t buf[3840];
static int disable_accel = 0;
static int disable_dynrng = 0;
static int disable_adjust = 0;
static float gain = 1.50f;
static ao_open_t * output_open = NULL;
static ao_instance_t * output;
static a52_state_t * state;

#include "LibA52.h"

extern void *a52_drv_callback( snd_stream_hnd_t hnd, int pcm_needed, int * pcm_done );

void LibA52_VolumeUp()
{
   snddrv_volume_up(); 
}

void LibA52_VolumeDown()
{
    snddrv_volume_down();
}

int LibA52_Init( int rate, int channels )
{
    ao_driver_t * drivers;
    drivers = ao_drivers ();    
    output_open = drivers[0].open;

    uint32_t accel;
    accel = disable_accel ? 0 : MM_ACCEL_DJBFFT;

    output = output_open ();
    if (output == NULL)
    {
	    fprintf (stderr, "Can not open output\n");
	    return 0;
    }
    
    state = a52_init (accel);
    if (state == NULL)
    {
	    fprintf (stderr, "A52 init failed\n");
	    return 0;
    }
    
    A52_DEC_STAT = 1;
    
    A52_create_mutex()
    
    snddrv_start_cb_thd( rate, channels, a52_drv_callback );
    
    return 1;
}

int LibA52_Exit()
{
    A52_DEC_STAT = 0; thd_sleep(50);
	snddrv_exit();
        
    a52_free (state);    
    if (output->close)
	output->close (output);
	
	A52_destroy_mutex()

    return 1;
}

void LibA52_decode_chunk (uint8_t * start, uint8_t * end)
{
    //static uint8_t buf[3840];
    static uint8_t * bufptr = buf;
    static uint8_t * bufpos = buf + 7;

    /*
     * sample_rate and flags are static because this routine could
     * exit between the a52_syncinfo() and the ao_setup(), and we want
     * to have the same values when we get back !
     */
     
    //printf("LibA52: Decode chunk\n");

    static int sample_rate;
    static int flags;
    int bit_rate;
    int len;

    while (1) {
          
	len = end - start;
	if (!len)
	    break;
	if (len > bufpos - bufptr)
	    len = bufpos - bufptr;
	memcpy (bufptr, start, len);
	bufptr += len;
	start += len;
	if (bufptr == bufpos)
    {
	    if (bufpos == buf + 7) {
		int length;

		length = a52_syncinfo (buf, &flags, &sample_rate, &bit_rate);
		if (!length) {
		    //fprintf (stderr, "skip\n");
		    for (bufptr = buf; bufptr < buf + 6; bufptr++)
			bufptr[0] = bufptr[1];
		    continue;
		}
		bufpos = buf + length;
	    } else {
		level_t level;
		sample_t bias;
		int i;

		if (output->setup (output, sample_rate, &flags, &level, &bias))
		    goto error;
		if (!disable_adjust)
		    flags |= A52_ADJUST_LEVEL;
		level = (level_t) (level * gain);
		if (a52_frame (state, buf, &flags, &level, bias))
		    goto error;
		if (disable_dynrng)
		    a52_dynrng (state, NULL, NULL);
		for (i = 0; i < 6; i++)
        {
		    if (a52_block (state))
			goto error;

            if (output->play (output, flags, a52_samples (state)))
			goto error;
		}
		bufptr = buf;
		bufpos = buf + 7;

		continue;
	    error:
		//fprintf (stderr, "error\n");
		bufptr = buf;
		bufpos = buf + 7;
	    }
	}
    }
}

int ac3_dec( char * ac3_file, char * ac3_dir ) 
{
    int size;
    
	FILE * ac3_t = fs_open (ac3_file, O_RDONLY);
	if (!ac3_t) {
	    fprintf (stderr, "%s - could not open file %s\n", strerror (errno),
		     ac3_file);
	    exit (1);
	}

    LibA52_Init( 48000, 2 );
       
    do {     

        size = fs_read( ac3_t, buffer, BUFFER_SIZE );  
	    
        LibA52_decode_chunk( buffer, buffer + size ); 
        
    } while (size == BUFFER_SIZE );
     
    fs_close( ac3_t );   

    LibA52_Exit();
    
    return 0;
}

