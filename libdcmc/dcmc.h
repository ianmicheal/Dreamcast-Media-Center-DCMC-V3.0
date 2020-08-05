/*
** Dreamcast Media Center (C) Josh PH3NOM Pearson 2010-2013
*/

#ifndef DCMC_H
#define DCMC_H

#include <stdio.h>

#include "gl.h"

#include "LibXVID.h"

#include "container.h"
#include "cstring.h"
#include "font.h"
#include "fs.h"
#include "input.h"
#include "texture.h"

#define DIR_IMG 0x0
#define DIR_VID 0x1
#define DIR_AUD 0x2
#define DIR_BIN 0x3

/* One Global Variable ...Ugh should make this local and pass it around... */
unsigned char FDIR;

enum {
   BgndVideo = 0,
   BgndMusic,
   BgndPicture,
   BgndProgram,
   TbnFolder,
   TbnProgram,  
   TbnAudio,  
   TbnVideo, 
   TbnPicture 
}  DCMC_TextureID;

/* For now, this will decode a static list of textures */
void DCMC_InitTextures();

/* Initialize internal directory listing array */
void DCMC_InitFileDirectory();

/* Load a font (ascii) texture
** container may be CONTAINER_PVR, CONTAINER_PNG, or CONTAINER_JPG
** mode is for alpha channel of PNG textures 0=RGB565,1=ARGB1555,2=ARGB4444
*/
Font * DCMC_LoadFontTex(char *filename, int container, int mode );

/* Return an pointer to the texture
** param should be one of the DCMC_TextureID enumeration listings */
GlTexture * DCMC_TexID( int param );

/* Load a file according to its container type ( file extension ) */
void DCMC_LoadFile( Font * font, char * file, DirectoryEntry * dir_entry );

/* Render the bgnd, delay 1000ms, and then load directory */
void DCMC_LoadDirectory( Font *font, GlTexture * bgnd );

/* Render a non-textured quad of given dimensions and color */
void DCMC_RenderQuad( float x, float y, float w, float h, uint32 color );

/* Render a textured quad of given dimensions */
void DCMC_RenderGlTexture( GlTexture * tbn, float x, float y, float w, float h );

/* Render a textured quad in 3D space
** x, y, z is an offset from center used to Translate the coordinates
** DAR shoud be your ImageWidth/ImageHeight */
void DCMC_RenderGlTexture3D(float x, float y, float z, float DAR, GlTexture * tbn);

/* Render the thumbnail for a directory entry */
void DCMC_RenderTbn( DirectoryEntry * fs_dir, float x, float y, float w, float h );

/* Load the current file directory, render the thumbnails,
** and allow user to navigate directory including file selection */
void DCMC_ParseDirectory( Font *font, GlTexture * bgnd );

/* */
void DCMC_RenderDirectory( DirectoryEntry * dir, Font * font, int entry,
                           int entries, float size );
/* */
void DCMC_RenderDirectoryLS( DirectoryEntry * dir, Font * font, int entry,
                             int entries, float size );


#endif
