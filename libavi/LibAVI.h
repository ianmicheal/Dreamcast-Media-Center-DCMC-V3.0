/*
**
** LibAVI.h (C) Josh "PH3NOM" Pearson 2012
**
** This work is distributed free of restriction, with no guarantee of any kind.
**
** Most of the data structures expressed here are posted on MSDN
** http://msdn.microsoft.com/en-us/library/ms899421
**
*/

#ifndef LIBAVI_H
#define LIBAVI_H

#define AVI_OFT_RIFF 0x00
#define AVI_OFT_AVI  0x08
#define AVI_OFT_HDR  0x20
#define AVI_OFT_SRTL 0x60
#define AVI_OFT_STRH 0x64

#define AVI_CHUNK_EOF  0x00
#define AVI_CHUNK_VIDS 0x01
#define AVI_CHUNK_AUDS 0x02
#define AVI_CHUNK_TXTS 0x03
#define AVI_CHUNK_JUNK 0x04

#define ACODEC_MP3 0x0055
#define ACODEC_AC3 0x2000

typedef unsigned int    _DWORD;
typedef long            _LONG;
typedef short           _BYTE;
typedef unsigned short  _WORD;
typedef char           FOURCC[4];

typedef struct
{
  FOURCC fcc;                     // "strh"
   _DWORD  cb;                      // Size - 8byte hdr
  FOURCC fccType;                 // vids ! auds
  FOURCC fccHandler;              // Codec type
   _DWORD  dwFlags;
   _WORD   wPriority;
   _WORD   wLanguage;
   _DWORD  dwInitialFrames;
   _DWORD  dwScale;
   _DWORD  dwRate;
   _DWORD  dwStart;
   _DWORD  dwLength;
   _DWORD  dwSuggestedBufferSize;
   _DWORD  dwQuality;
   _DWORD  dwSampleSize;
  struct {
    short int left;
    short int top;
    short int right;
    short int bottom;
  } rcFrame;
} AVISTREAMHEADER;

typedef struct
{
   _WORD  wFormatTag;
   _WORD  nChannels;
   _DWORD nSamplesPerSec;
   _DWORD nAvgBytesPerSec;
   _WORD  nBlockAlign;
   _WORD  wBitsPerSample;
   _WORD  cbSize;
} WAVEFORMATEX;

typedef struct
{
   _DWORD biSize;
   _LONG  biWidth;
   _LONG  biHeight;
   _WORD  biPlanes;
   _WORD  biBitCount;
   _DWORD biCompression;
   _DWORD biSizeImage;
   _LONG  biXPelsPerMeter;
   _LONG  biYPelsPerMeter;
   _DWORD biClrUsed;
   _DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct
{
  FOURCC fcc;
   _DWORD  dwSize;
} FOURCCHDR;

typedef struct
{
  FOURCC fcc;
} FCC;

typedef struct
{
   _DWORD dwMicroSecPerFrame;    /* time delay between frames in microseconds */
   _DWORD dwMaxBytesPerSec;      /* data rate of AVI data */
   _DWORD dwPaddingGranularity;  /* padding multiple size, typically 2048 */
   _DWORD dwFlags;               /* parameter flags */
   _DWORD dwTotalFrames;         /* number of video frames */
   _DWORD dwInitialFrames;       /* number of preview frames */
   _DWORD dwStreams;             /* number of data streams (1 or 2) */
   _DWORD dwSuggestedBufferSize; /* suggested playback buffer size in bytes */
   _DWORD dwWidth;               /* width of video image in pixels */
   _DWORD dwHeight;              /* height of video image in pixels */ 
   _DWORD dwTimeScale;           /* time scale, typically 30 */
   _DWORD dwDataRate;            /* data rate (frame rate = data rate / time scale)*/
   _DWORD dwTimeStart;           /* starting time, typically 0 */
   _DWORD dwTimeChunk;           /* size of AVI data chunk in time scale units */
  WAVEFORMATEX * dwAudio;
} AVIHeader;

/************************************************************************************/
/* LibAVI Function Protocols */

/* LibAVI_open will take an already opened file descriptor */
/* And return the main AVIHeader */
/* PostCondition: The File will be positioned at the begining chunk offset */
AVIHeader * LibAVI_open( FILE * avi_t );

/* 
** Read a chunk of AVI data 
**
** Return Codes:
** AVI_CHUNK_AUDS - Audio Chunk
** AVI_CHUNK_VIDS - Video Chunk
** AVI_CHUNK_EOF - End of Chunk Data
**
** PostCondition: 
** The dst buffer will contain the AVI chunk
** The FOURCCHDR will contain relevant information
*/
int LibAVI_read_chunk( FILE * avi_t, FOURCCHDR * fcchdr, unsigned int * dst );

/* 
** Read a chunk of Video data, discarding everything else 
**
** Return Codes:
** AVI_CHUNK_VIDS - Video Chunk
** AVI_CHUNK_EOF - End of Chunk Data
**
** PostCondition: 
** The dst buffer will contain the AVI chunk
** The FOURCCHDR will contain relevant information
*/
int LibAVI_read_video( FILE * avi_t, FOURCCHDR * fcchdr, unsigned int * dst );

/* Little helper function to convert microseconds to frames per second */
float micro2fps( unsigned int m );

/************************************************************************************/

#endif
