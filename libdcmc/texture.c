/*
**
** Texture.c (C) Josh PH3NOM Pearson 2013
**
*/
#include <stdio.h>
#include <malloc.h>

#include "Texture.h"
#include "jpg_core.h"
#include "png_core.h"
#include "timer.h"

#define PVR_HDR_SIZE 0x20

#define abs(n) n<0 ? -n : n

TexStruct * TextureJPG( char * filename )
{
    TexStruct *Texture = malloc( sizeof(TexStruct) );
    
    printf("JPG: %s\n", filename );
        
    FILE * jpgFile = NULL;
    jpgFile = fopen( filename, "rb" );
    if(jpgFile==NULL)
    {
        printf("JPG: Can Not Open %s\n", filename);
        while(1);
    }

    unsigned int st = GetTime();
     
    jpeg_to_image(jpgFile, Texture );
    
    printf("JPG Dec: %i msec\n", GetTime()-st );
    printf("JPG Dimensions: %ix%i\n", Texture->w, Texture->h);
    
    fclose(jpgFile);
    
    Texture->col = PVR_TXRFMT_RGB565; 
    Texture->fmt = PVR_TXRFMT_NONTWIDDLED;  

    return Texture;
}

TexStruct * TexturePNG( char * filename, int mode )
{
    TexStruct *Texture = malloc( sizeof(TexStruct) );
    
    printf("PNG: %s\n", filename );
        
    FILE * pngFile = NULL;
    pngFile = fopen( filename, "rb" );
    if(pngFile==NULL)
    {
        printf("PNG: Can Not Open %s\n", filename);
        while(1);
    }

    unsigned int st = GetTime();
    
    png_to_image(pngFile, mode, Texture );
    
    printf("PNG Dec: %i msec\n", GetTime()-st );
    printf("PNG Dimensions: %ix%i\n", Texture->w, Texture->h);
    
    fclose(pngFile);
        
    switch(mode)
    {
       case 0:
            Texture->col = PVR_TXRFMT_RGB565;
            break;
       case 1:
            Texture->col = PVR_TXRFMT_ARGB1555;
            break;
       case 2:
            Texture->col = PVR_TXRFMT_ARGB4444;
            break;
    }
    Texture->fmt = PVR_TXRFMT_NONTWIDDLED;  

    return Texture;
}

TexStruct * TexturePVR( char * filename )
{
    TexStruct *Texture = malloc( sizeof(TexStruct) );
    
    FILE * f = NULL;
    unsigned char * buffer;
    
    f=fopen( filename, "rb" );
    if(f==NULL) return NULL;
    
    /* Read the file into ram */
    fseek( f, 0, SEEK_END );
    Texture->size = ftell( f );
    fseek( f, 0, SEEK_SET );
    buffer = (unsigned char*)memalign(0x20, Texture->size );
    fread( buffer, Texture->size/16, 16, f );
    fclose( f );
    
    Texture->size-=PVR_HDR_SIZE; // Texture data, not including the file header
    
    Texture->data = (char*)pvr_mem_malloc( Texture->size );
    
    switch( (unsigned int)buffer[PVR_HDR_SIZE-8] ) {
          case 0x00: Texture->col = PVR_TXRFMT_ARGB1555; break; //(bilevel translucent alpha 0,255)
          case 0x01: Texture->col = PVR_TXRFMT_RGB565;   break; //(non translucent RGB565 )
          case 0x02: Texture->col = PVR_TXRFMT_ARGB4444; break; //(translucent alpha 0-255)
          case 0x03: Texture->col = PVR_TXRFMT_YUV422;   break; //(non translucent UYVY )
          case 0x04: Texture->col = PVR_TXRFMT_BUMP;     break; //(special bump-mapping format)
          case 0x05: Texture->col = PVR_TXRFMT_PAL4BPP;  break; //(4-bit palleted texture)
          case 0x06: Texture->col = PVR_TXRFMT_PAL8BPP;  break; //(8-bit palleted texture)
          default:   break;            
    }    

    switch( (unsigned int)buffer[PVR_HDR_SIZE-7] ) {
          case 0x01: Texture->fmt = PVR_TXRFMT_TWIDDLED;                           break;//SQUARE TWIDDLED
          //case 0x02: Texture->fmt = SQUARE TWIDDLED & MIPMAP
          case 0x03: Texture->fmt = PVR_TXRFMT_VQ_ENABLE;                          break;//VQ TWIDDLED
          //case 0x04: Texture->fmt = VQ & MIPMAP
          //case 0X05: Texture->fmt = 8-BIT CLUT TWIDDLED
          //case 0X06: Texture->fmt = 4-BIT CLUT TWIDDLED
          //case 0x07: Texture->fmt = 8-BIT DIRECT TWIDDLED
          //case 0X08: Texture->fmt = 4-BIT DIRECT TWIDDLED
          case 0x09: Texture->fmt = PVR_TXRFMT_NONTWIDDLED;                        break;//RECTANGLE    
          case 0x0B: Texture->fmt = PVR_TXRFMT_STRIDE | PVR_TXRFMT_NONTWIDDLED;    break;//RECTANGULAR STRIDE
          case 0x0D: Texture->fmt = PVR_TXRFMT_TWIDDLED;                           break;//RECTANGULAR TWIDDLED
          case 0x10: Texture->fmt = PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_NONTWIDDLED; break;//SMALL VQ
          //case 0x11: Texture->fmt = SMALL VQ & MIPMAP
          //case 0x12: Texture->fmt = SQUARE TWIDDLED & MIPMAP
          default:   Texture->fmt = PVR_TXRFMT_NONE; break;
    }

    /* Get PVR Texture Dimensions */  
    Texture->w = abs(((buffer[PVR_HDR_SIZE-4])|(buffer[PVR_HDR_SIZE-3] << 8))); 
    Texture->h = abs(((buffer[PVR_HDR_SIZE-2])|(buffer[PVR_HDR_SIZE-1] << 8)));
    
    pvr_txr_load((char*)buffer+PVR_HDR_SIZE, (char*)Texture->data, Texture->size);  
    
    free(buffer);
    
    return Texture;
}
