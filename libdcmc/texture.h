/*
**
** Very Simple Input Class for the DC Controller (C) Josh PH3NOM 2013
**
*/

#ifndef Texture_H
#define Texture_H

#include <dc/pvr.h>

typedef struct
{
    unsigned short int w, h;
    unsigned int size;
    unsigned int fmt, col;
    char * data;
}TexStruct;

TexStruct * TexturePVR( char * filename );
TexStruct * TexturePNG( char * filename, int mode );
TexStruct * TextureJPG( char * filename );

#endif
