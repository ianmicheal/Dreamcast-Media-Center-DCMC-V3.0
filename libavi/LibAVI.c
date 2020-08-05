/*
**
** LibAVI.c (C) Josh "PH3NOM" Pearson 2012
**
** This work is distributed free of restriction, with no guarantee of any kind.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavi.h"

//#define VERBOSE

float micro2fps( unsigned int m )
{
    return 1000.0/(m*0.001);
}

int LibAVI_parse_riff( FILE * avi_t )
{
    char buf[4];
    
    fseek( avi_t, AVI_OFT_RIFF, SEEK_SET );
    fread( buf, 4, 1, avi_t );
    
    if(!memcmp("RIFF", buf, 4) )
        return 1;
    return 0;
}

int LibAVI_parse_avi( FILE * avi_t )
{
    char buf[4];
    
    fseek( avi_t, AVI_OFT_AVI, SEEK_SET );
    fread( buf, 4, 1, avi_t );
    
    if(!memcmp("AVI ", buf, 4) )
        return 1;
    return 0;
}

AVIHeader * LibAVI_parse_hdr( FILE * avi_t )
{
    AVIHeader * avihdr = malloc( sizeof(AVIHeader) );
       
    fseek( avi_t, AVI_OFT_HDR, SEEK_SET );
    fread( avihdr, 1, sizeof(AVIHeader)-4, avi_t );

    return avihdr;
}

/* Seek to the offset of the first Data Chunk */
int LibAVI_seek_data_offset( FILE * avi_t )
{
    FCC * fcc = malloc( sizeof(FCC) );
    while(1)                    
    {
        if( fread( fcc, 1, sizeof(FCC), avi_t ) == EOF )/* Read DWORD FOURCC */
            return 0;             

        if( !memcmp("LIST", fcc->fcc, 4) )               /* Found FOURCC LIST */
        {
            fseek( avi_t, sizeof(FCC), SEEK_CUR );    /* Skip un-needed DWORD */
            fread( fcc, 1, sizeof(FCC), avi_t );         /* Read DWORD FOURCC */
            if( !memcmp("movi", fcc->fcc, 4) )           /* Found FOURCC movi */
                break;
        }         
    }
    free(fcc);
    return 1;
}

int LibAVI_parse_stream_hdr( FILE * avi_t, AVIHeader *avihdr )
{
    AVISTREAMHEADER  * sthdr  = malloc( sizeof(AVISTREAMHEADER) );
    BITMAPINFOHEADER * bmphdr = malloc( sizeof(BITMAPINFOHEADER) );
    WAVEFORMATEX     * wavhdr = malloc( sizeof(WAVEFORMATEX) );
    FOURCCHDR        * fcchdr = malloc( sizeof(FOURCCHDR) );
        
    fseek( avi_t, AVI_OFT_STRH, SEEK_SET );    /* Seek to AVISTREAMHDR Offset */
    
    fread( sthdr, 1, sizeof(AVISTREAMHEADER), avi_t );   /* Read AVISTREAMHDR */
    
    if( memcmp("strh", sthdr->fcc, 4) )              /* Ensure Correct Header */
        return 0;

    /* Video Stream Header -> BITMAPINFOHEADER */
    if( !memcmp("vids", sthdr->fccType, 4) )
    {       
        fread( fcchdr, 1, sizeof(FOURCCHDR), avi_t );

        if( memcmp("strf", fcchdr->fcc, 4) )
            return 0;
        
        fread( bmphdr, 1, sizeof(BITMAPINFOHEADER), avi_t );
#ifdef VERBOSE       
        printf("BIMAPINFOHDR - Width: %i\n",  bmphdr->biWidth );
        printf("BIMAPINFOHDR - Height: %i\n", bmphdr->biHeight );        
#endif
    } 
    
    /* Seek to WAVEFORMATEX chunk */
    fread( fcchdr, 1, sizeof(FOURCCHDR), avi_t );
    if( memcmp("LIST", fcchdr->fcc, 4) )
    {
       fseek( avi_t, fcchdr->dwSize, SEEK_CUR );
       fread( fcchdr, 1, sizeof(FOURCCHDR), avi_t );
    }
    fread( fcchdr->fcc, 1, 4, avi_t );
    
    /* Audio Stream Header -> WAVEFORMATEX*/
    if(!memcmp("strl", fcchdr->fcc, 4) )
    {
         fread( fcchdr, 1, sizeof(FOURCCHDR), avi_t );
         fseek( avi_t, fcchdr->dwSize, SEEK_CUR );
         fread( fcchdr, 1, sizeof(FOURCCHDR), avi_t );

         if(!memcmp("strf", fcchdr->fcc, 4) )
         {
             fread( wavhdr, 1, sizeof(WAVEFORMATEX), avi_t );
             if( fcchdr->dwSize > sizeof(WAVEFORMATEX) )
                 fseek( avi_t, fcchdr->dwSize - sizeof(WAVEFORMATEX), SEEK_CUR );
         }
         
         avihdr->dwAudio = wavhdr;
    } 
    
    /* Seek to the offset of the data chunks */
    if( !LibAVI_seek_data_offset( avi_t ) )
    {
        printf("LibAVI ERROR: Data Chunk Offset\n");
        return 0;
    }
    
    free( sthdr );
    free( bmphdr );
    free( fcchdr );
        
    return 1;
    
}

int LibAVI_read_chunk( FILE * avi_t, FOURCCHDR * fcchdr, unsigned int * dst )
{
    if( fread( fcchdr, 1, sizeof(FOURCCHDR), avi_t ) == EOF )
        return AVI_CHUNK_EOF;
            
    if( !memcmp("01wb", fcchdr->fcc, 4) )
    {
        if( fread( dst, 1, fcchdr->dwSize, avi_t ) == EOF )
            return AVI_CHUNK_EOF;

        if(fcchdr->dwSize%2!=0)
           fseek( avi_t, 1, SEEK_CUR );
#ifdef DEBUG         
        printf("LibAVI: Audio Chunk %i bytes\n", fcchdr->dwSize );
#endif           
        return AVI_CHUNK_AUDS;
    }
    else if( !memcmp("00dc", fcchdr->fcc, 4) )
    {
        if( fread( dst, 1, fcchdr->dwSize, avi_t ) == EOF )
            return AVI_CHUNK_EOF;

        if(fcchdr->dwSize%2!=0)
           fseek( avi_t, 1, SEEK_CUR );
#ifdef DEBUG          
        printf("LibAVI: Video Chunk %i bytes\n", fcchdr->dwSize );
#endif           
        return AVI_CHUNK_VIDS;
    }
    else
#ifdef VERBOSE
       printf("LibAVI: Invalid Chunk\n");
#endif 
    return AVI_CHUNK_JUNK;
}

int LibAVI_read_video( FILE * avi_t, FOURCCHDR * fcchdr, unsigned int * dst )
{
    int stat;
    do {
        stat = LibAVI_read_chunk( avi_t, fcchdr, dst );
        if( fcchdr->dwSize < 1 )
            return AVI_CHUNK_EOF; 
    } while( stat != AVI_CHUNK_VIDS );
    return stat;
}

/* Open the AVI File, Parse the Main Header, then seek to data chunk offset */
AVIHeader * LibAVI_open( FILE * avi_t )
{
    AVIHeader * avihdr = NULL;     
    
    if( !LibAVI_parse_riff(avi_t) && !LibAVI_parse_avi(avi_t) )
    {  
        printf("RIFF AVI Format NOT Found\n");
        return avihdr;
    }  

    avihdr = LibAVI_parse_hdr(avi_t);
    
    if( !LibAVI_parse_stream_hdr(avi_t, avihdr) )
    {  
        printf("AVI I/O ERROR\n");
        return avihdr;
    }     

#ifdef VERBOSE
    printf("AVIHDR FPS: %.3f\n",  micro2fps(avihdr->dwMicroSecPerFrame) );    
    printf("AVIHDR Width: %i\n",  avihdr->dwWidth );    
    printf("AVIHDR Height: %i\n", avihdr->dwHeight ); 
    switch( avihdr->dwAudio->wFormatTag )
    {
        case ACODEC_MP3:
             printf("AVIHDR Audio Codec: MP3\n");
             break;
             
        case ACODEC_AC3:
             printf("AVIHDR Audio Codec: AC3\n");
             break;
    }
    printf("AVIHDR Audio Rate: %i\n", avihdr->dwAudio->nSamplesPerSec );
    printf("AVIHDR Audio Channels: %i\n", avihdr->dwAudio->nChannels );           
#endif

    return avihdr;
}
