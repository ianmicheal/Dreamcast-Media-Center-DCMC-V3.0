/*
**
** PNG Core - Decode a PNG texture using libPing
** (C) Josh "PH3NOM" Pearson  2011
** 
*/


#include <malloc.h>

#include "Texture.h"
#include "png_core.h"

extern void _png_copy_texture(uint8 *buffer, uint16 *temp_tex,
                      uint32 channels, uint32 stride,
                      uint32 mask, uint32 w, uint32 h);

short png_to_image(FILE *infile, uint32 mask, TexStruct * texture )
{
  uint8 *buffer;     /* Output row buffer */
  uint32 row_stride; /* physical row width in output buffer */
  uint32 channels;   /* 3 for RGB 4 for RGBA */
  void * strs;		 /* internal structs */

  /* Initialize loader */
  strs = readpng_init(infile);
  if (!strs) return 0;

  /* Decompress file */
  buffer = readpng_get_image( strs, &channels, &row_stride,
                              (uint32*)&texture->w,(uint32*)&texture->h );

  texture->data = memalign(32, 2*texture->w*texture->h);

  _png_copy_texture( buffer, (uint16 *)texture->data , channels, row_stride,
                       mask, texture->w, texture->h );

  /* Rree resources */
  free(buffer);
  readpng_cleanup(strs);

  return 1;
}
