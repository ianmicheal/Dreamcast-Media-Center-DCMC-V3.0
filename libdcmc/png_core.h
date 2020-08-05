/*
**
** PNG Core - Decode a PNG texture using libPing
** (C) Josh "PH3NOM" Pearson  2011-2013
** 
*/

#ifndef PNG_CORE_H
#define PNG_CORE_H

#include <png/png.h>
#include <zlib/zlib.h>
#include "readpng.h"

#include "Texture.h"

/* Decode a PNG file into a TexStruct */
/* Based on png_to_texture, but does not copy texture to VRAM */
short png_to_image(FILE *infile, uint32 mask, TexStruct * texture );

#endif
