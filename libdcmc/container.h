/*
** Container.c (C) Josh PH3NOM Pearson 2013
*/

#ifndef CONTAINER_H
#define CONTAINER_H

/* Container Formats */
#define CONTAINER_NULL    0x00
#define CONTAINER_UNKNOWN 0x01

#define CONTAINER_AUDIO   0x10
#define CONTAINER_MP3     0x10
#define CONTAINER_AC3     0x11
#define CONTAINER_ADX     0x12
#define CONTAINER_WAV     0x13
#define CONTAINER_OGG     0x14
#define CONTAINER_MP4     0x15
#define CONTAINER_M4A     0x16
#define CONTAINER_AAC     0x17 
#define CONTAINER_FLAC    0x18
/* 0x19->0x1E Reserved */
#define CONTAINER_M3U     0x1F

#define CONTAINER_VIDEO   0x20
#define CONTAINER_MPG     0x20
#define CONTAINER_MPEG    0x21
#define CONTAINER_AVI     0x22
#define CONTAINER_SFD     0x23

#define CONTAINER_IMAGE   0x30
#define CONTAINER_PVR     0x30
#define CONTAINER_JPG     0x31
#define CONTAINER_PNG     0x32

#define CONTAINER_BIN     0x40

typedef struct
{
    float Fps;
    unsigned short int Width, Height;
    unsigned int Flags;
    void * Texture;
} VideoThumbnail;

unsigned char FsMediaContainer( char * fname );

#endif
