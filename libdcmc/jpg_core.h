/* 
** 
** JPG-Core - A JPG Texture Loader
** (C) Josh "PH3NOM" Pearson 2011 
**
*/


#ifndef JPG_CORE_H
#define JPG_CORE_H

#include "jpeg/jpeglib.h"

#include "Texture.h"

/* Load a JPEG into a TexStruct - Not loaded into VRAM */
int jpeg_to_image( FILE * infile, TexStruct * texture );

#endif

