/*
**
** This file is a part of Dreamcast Media Center
** (C) Josh PH3NOM Pearson 2011
**
*/
/* KallistiOS ##version##

   exec.
   (c)2002 Dan Potter
*/

#include <kos.h>

void bin_exec( char * binary )
{
    unsigned int * bin_buffer, fSize;
    FILE * bin_file;

    bin_file = fopen( binary, "rb" );

    fseek( bin_file , 0, SEEK_END );
    fSize = ftell( bin_file );
    fseek( bin_file , 0, SEEK_SET );

    bin_buffer = malloc( fSize );
    fread( bin_buffer, 1, fSize, bin_file );
    fclose( bin_file );

    arch_exec( bin_buffer , fSize );
}
