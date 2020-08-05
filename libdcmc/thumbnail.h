#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include "fs.h"

unsigned char ThumbnailFind( DirectoryEntry * dir, char * dirname );
void ThumbnailLoadPVR( DirectoryEntry * dir );
void ThumbnailLoadPNG( DirectoryEntry * dir ); /* ToDo */
void ThumbnailLoadJPG( DirectoryEntry * dir ); /* ToDo */

void ThumbnailCopy( GlTexture *src, GlTexture *dst );

#endif
