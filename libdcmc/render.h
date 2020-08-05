#ifndef RENDER_H
#define RENDER_H

#include "gl.h"

#include "font.h"

/* Audio Types that can begin synchronized streaming with Render Routine */
/* Note that the audio CODEC must be in WaitStart() mode for this to apply */
#define AUDIO_CODEC_MPEG 0x01
#define AUDIO_CODEC_AC3  0x02
#define AUDIO_CODEC_ADX  0x03

/* Initialize the Render Routine State */
/*
** w = width of video frame ( in pixels )
** h = height of video frame ( in pixels )
** vidfmt = Texture Color eg; ( PVR_TXRFMT_YUV422 | PVR_TXRFMT_NONTWIDDLED )
** audfmt = one of the above Audio Type constants to begin synchronized stream
** FTS = Frame Time Stamp ( time, in miliseconds, in between frames )
** f = Font to be used for OSD if enabled
*/
void RenderInit( unsigned int w, unsigned int h, unsigned int vidfmt,
                 unsigned int audfmt, float FTS, Font * f );

/* After calling RenderInit, we can begin Render Thread, with audio stream */
void RenderStart();

/* Push a video frame into the Render FIFO */
void RenderPush(unsigned char * src);

/* Stop the Render Routine and destroy the Render Thread */
void RenderDestroy();

/* We can set the Display Aspect Ratio during playback */
/* This should work for any video mode, 640x480 or 640x576 etc. */
void SetDisplay_16_9();

void SetDisplay_235_1();

void SetDisplay_4_3();

void SetDisplayDefault(); /* Default 1:1 Pixel Aspect Ratio */

void SetDisplay( unsigned int w, unsigned int h );

void RenderShowInfo();

void RenderHideInfo();

unsigned char RenderInfo();

void RenderDelayIncrease();
void RenderDelayDecrease();

#endif
