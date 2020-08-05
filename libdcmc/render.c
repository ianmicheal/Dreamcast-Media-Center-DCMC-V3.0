/*
** DC Video Render Routine using GL and KOS (C) Josh PH3NOM Pearson 2013
**
** This handles the presentation of frame output during video playback.
** First, you need to call RenderInit() to set the state of the Render Routine.
** When you are ready to start stream playback, call RenderStart() to create
** the Render Thread, and begin audio playback with video playback.
**
** The core of this routine is based around a circular "FIFO Buffer", where
** frames are decoded into a large buffer in main ram before being sent to
** vram for presentation.
** While the Render Thread is running, the decoder can continue to run,
** pushing video frames into the Render FIFO by calling RenderPush(), only
** stalling the decoder when the FIFO Buffer is full.
**
** Only thing left to do is implement a double-buffer from sh4->pvr to prevent
** screen-tearing that occurs ocasionally...
*/

#include "malloc.h"
#include "malloc_pvr.h"
#include "stdio.h"
#include "string.h"

#include "Render.h"

#include "Timer.h"

#include "Fifo_Buffer.h"

#include "LibMPG123.h"

#include "LibA52.h"

#include "LibADX.h"

#include "SndDrv.h"

static unsigned int IMGW, IMGH, TEXW, TEXH, TEXWSQ, DW, DH;
static unsigned char GlTexGen = 0, ACODEC;
static volatile unsigned char RENDER_ACTIVE = 0, RENDER_INFO = 0;
static float FRAME_TS;
static FIFOBUFFER vidbuf;
static pvr_ptr_t VRAM_BUFFER[2] = { NULL, NULL };
static GLuint gltexture[2];

static unsigned int FRAME = 0;

static Font * font;

static volatile float DELAY_FACTOR = 0.085f;
static unsigned int CUR_BUF = 0;

/* Vertex Data for Display Rectangle */
static vector3f disp_rect[4] = { { 0.0f,   480.0f, 0.0f },
                                 { 0.0f,     0.0f, 0.0f },
                                 { 640.0f, 480.0f, 0.0f },
                                 { 640.0f,   0.0f, 0.0f } };  

/* UV Data for Display Rectangle */
static float uv[4][2] = {
           { 0.0f, 1.0f },
           { 0.0f, 0.0f }, 
           { 1.0f, 1.0f },
           { 1.0f, 0.0f }, 
     }; 

static void InitSQ();

static inline void FrameLoadPVRSQ( unsigned int * buf );

static void InitDisplay();

/******************************************************************************/

void RenderDelayIncrease()
{
    DELAY_FACTOR += .001f;
}

void RenderDelayDecrease()
{
    DELAY_FACTOR -= .001f;
}


void RenderInit( unsigned int w, unsigned int h, unsigned int vidfmt,
                 unsigned int audfmt, float FTS, Font * f )
{
    IMGW = w; IMGH = h;
    TEXW = TEXH = 0x08;
    while(TEXW<IMGW)(TEXW<<=1);
    while(TEXH<IMGH)(TEXH<<=1);
    
    if(VRAM_BUFFER[CUR_BUF]!=NULL) pvr_mem_free((unsigned int*)VRAM_BUFFER[CUR_BUF]);
    VRAM_BUFFER[CUR_BUF] = pvr_mem_malloc(TEXW*TEXH*2);
    
    /* Bind the Texture Address to GL */
    if(!GlTexGen)/* Use original texture index bound to GL */
    {
        glGenTextures(2, gltexture);
        GlTexGen=!GlTexGen; 
    }
    glBindTexture(GL_TEXTURE_2D, gltexture[0]);
    glKosTex2D( vidfmt, TEXW, TEXH, VRAM_BUFFER[CUR_BUF] );   
    
    ACODEC = audfmt;
    
    FRAME_TS = FTS;
    
    font = f;
                
    InitSQ();
    InitDisplay();
    
    /* Initialze the FIFO Buffer */
    FifoBufferInit(&vidbuf, IMGW*IMGH*2);
    FIFO_MUTEX_CREATE()
}

void RenderDestroy()
{
    FifoBufferFree(&vidbuf);
    FIFO_MUTEX_DESTROY()
    
    if(VRAM_BUFFER[CUR_BUF])  pvr_mem_free((unsigned int*)VRAM_BUFFER[CUR_BUF]);
    VRAM_BUFFER[CUR_BUF]=NULL;
    
    RENDER_ACTIVE = 0;
} 

unsigned char RenderInfo()
{
    return RENDER_INFO;
}

void RenderShowInfo()
{
    RENDER_INFO = 1;
}

void RenderHideInfo()
{
    RENDER_INFO = 0;
}

/* SH4->PVR SQ Transfers ******************************************************/

static uint32 c;
static unsigned int *d;

static void InitSQ() /* Avoid doing this every frame */
{
    TEXWSQ = TEXW*2;
    c = IMGW*2;
	if (c % 4)
		c = (c & 0xfffffffc) + 4;
	
    d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)VRAM_BUFFER[CUR_BUF]) & 0x03ffffe0));
}

/* Textures sent to PVR using SQ transfers */
static inline void FrameLoadPVRSQ( unsigned int * src )
{
    int i, n;
    unsigned int *s = (unsigned int *)src;
    uint32 count = c; 
   
	/* Set store queue memory area as desired */
	QACR0 = ((((unsigned int)VRAM_BUFFER[CUR_BUF])>>26)<<2)&0x1c;
	QACR1 = ((((unsigned int)VRAM_BUFFER[CUR_BUF])>>26)<<2)&0x1c;

    for(i=0;i<IMGW;i++)
    {                        
	  d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)VRAM_BUFFER[CUR_BUF]+(i*(TEXWSQ))) & 0x03ffffe0));
	  
	  n = count>>5;
	  
	  while(n--) {
		asm("pref @%0" : : "r" (s + 8)); // prefetch 32 bytes for next loop 
		d[0] = *(s++);
		d[1] = *(s++);
		d[2] = *(s++);
		d[3] = *(s++);
		d[4] = *(s++);
		d[5] = *(s++);
		d[6] = *(s++);
		d[7] = *(s++);
		asm("pref @%0" : : "r" (d));
		d += 8;
	  }
	}
	/* Wait for both store queues to complete */
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;    
}

/* Vertex Data for GL submission **********************************************/

void SetDisplay_16_9()
{
    float h = (vid_mode->width/(16.0f/9.0f));
    disp_rect[0][0] = disp_rect[1][0] = 0.0f;
    disp_rect[2][0] = disp_rect[3][0] = vid_mode->width;
    disp_rect[0][1] = disp_rect[2][1] = h+((vid_mode->height-h)/2); 
    disp_rect[1][1] = disp_rect[3][1] = ((vid_mode->height-h)/2);   
}

void SetDisplay_235_1()
{
    float h = (vid_mode->width/2.35f);
    disp_rect[0][0] = disp_rect[1][0] = 0.0f;
    disp_rect[2][0] = disp_rect[3][0] = vid_mode->width;
    disp_rect[0][1] = disp_rect[2][1] = h+((vid_mode->height-h)/2); 
    disp_rect[1][1] = disp_rect[3][1] = ((vid_mode->height-h)/2);   
}

void SetDisplay_4_3()
{
    disp_rect[0][0] = disp_rect[1][0] = 0.0f;
    disp_rect[2][0] = disp_rect[3][0] = vid_mode->width;
    disp_rect[0][1] = disp_rect[2][1] = vid_mode->height;
    disp_rect[1][1] = disp_rect[3][1] = 0.0f;    
}

void SetDisplay( DWORD w, DWORD h )
{
    disp_rect[0][0] = disp_rect[1][0] = (vid_mode->width-w)/2;
    disp_rect[2][0] = disp_rect[3][0] = w+disp_rect[0][0];
    disp_rect[1][1] = disp_rect[3][1] = (vid_mode->height-h)/2; 
    disp_rect[0][1] = disp_rect[2][1] = h+disp_rect[1][1];
}

void SetDisplayDefault()
{
     SetDisplay( DW, DH );
}

void InitDisplay()
{
    uv[0][0] = uv[1][0] = 1.0f/TEXW;
    uv[1][1] = uv[3][1] = 1.0f/TEXH;
    uv[2][0] = uv[3][0] = ((float)IMGW/TEXW)-(1.0f/TEXW);
    uv[0][1] = uv[2][1] = ((float)IMGH/TEXH)-(1.0f/TEXH);

    DH = vid_mode->width/(((float)IMGW)/IMGH);

    if(IMGW>IMGH)
       DW = vid_mode->width;
    else if (IMGW==IMGH)
       DW = DH = vid_mode->height;
    else // Height > Width
       DW = (DH*IMGW)/IMGH; //untested...
    
    SetDisplay( DW, DH );
}

static inline void FrameRender()
{			
    glKosBeginFrame();

    glBindTexture( GL_TEXTURE_2D, gltexture[0] );    

    glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2fv( (float*)&uv[0] );
        glVertex2fv( (float*)&disp_rect[0] );
        glTexCoord2fv( (float*)&uv[1] );
        glVertex2fv( (float*)&disp_rect[1] );
        glTexCoord2fv( (float*)&uv[2] );
        glVertex2fv( (float*)&disp_rect[2] );    
        glTexCoord2fv( (float*)&uv[3] );
        glVertex2fv( (float*)&disp_rect[3] );
    glEnd();
        
    if(RENDER_INFO)
    {  
        char timestr[128];
         
        glBindTexture( GL_TEXTURE_2D, font->TexId ); 
  
        glBegin(GL_QUADS);
        
            sprintf(timestr, "VIDEO: %.2f \n", (float)FRAME/(1000.0f/FRAME_TS) );
            FontPrintString( font, timestr, 24.0f, 36.0f, 20.0f, 20.0f );

            sprintf(timestr, "AUDIO: %.2f\n", ATS );
            FontPrintString( font, timestr, 24.0f, 56.0f, 20.0f, 20.0f );
    
            sprintf(timestr, "VBUF FRAMES: %i\n", vidbuf.num);
            FontPrintString( font, timestr, 320.0f, 420.0f, 20.0f, 20.0f );
                         
            sprintf(timestr, "DF: %.3f\n", DELAY_FACTOR );
            FontPrintString( font, timestr, 16.0f, 420.0f, 20.0f, 20.0f );
        
        glEnd();
    }
       
	glKosFinishFrame();       	
}

void RenderPush(unsigned char * src)
{
    /* If Video FIFO is full, wait for some room */ 
    while(!FifoBufferHasRoom(&vidbuf)) thd_pass(); 
    
    /* Push Frame into Frame Stack For Later Render */
    FifoBufferPush(&vidbuf, src, IMGW*IMGH*2);
}

//#define DEBUG

void RenderThread()
{
    unsigned int START;
    float NTS;
    FRAME = 0;
    
    RENDER_ACTIVE = 1;
        
    while(FifoBufferHasRoom(&vidbuf)) thd_pass(); /* Pre-Fill the Fifo Buffer */

    switch(ACODEC)                    /* Now, actually start the audio stream */
    {
        case AUDIO_CODEC_AC3:
             LibA52_Start();
             break;
             
        case AUDIO_CODEC_MPEG:
             LibMPG123_Start();
             break;

        case AUDIO_CODEC_ADX:
             LibADX_Start();
             break;
    }
    
    START = GetTime();                             /* Set Starting Frame Time */
    FifoBufferTopRemove(&vidbuf);        /* First xvid frame is not an image! */
    FrameLoadPVRSQ( FifoBufferTopPacket(&vidbuf) );     /* Load Frame To VRAM */
    FrameRender();                                        /* Render the Frame */
    FifoBufferTopRemove(&vidbuf);                   /* Remove Frame from FIFO */

    while(RENDER_ACTIVE)
    {        
        NTS = (FRAME * FRAME_TS)-(DELAY_FACTOR*FRAME++);   /* Set Next Frame Time Stamp */

        while(!FifoBufferHasPacket(&vidbuf)) thd_pass();    /* Wait for Frame */

        FrameLoadPVRSQ( FifoBufferTopPacket(&vidbuf) );    
        FifoBufferTopRemove(&vidbuf);
                     
        while(GetTime()-START < NTS) thd_pass(); /* Wait for Frame Time Stamp */

        FrameRender();                                 
    }
}

void RenderStart()
{
    thd_create( RenderThread, NULL );
}
