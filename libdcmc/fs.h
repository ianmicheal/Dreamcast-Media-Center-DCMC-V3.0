#ifndef FS_H
#define FS_H

#include <sys/types.h>
#include <dirent.h>

typedef struct dirent direntry;

typedef struct
{
    unsigned int  Index;
    unsigned char IsFile;
    char          *Name;
} DirEntry;

typedef struct
{
    float ImgW, ImgH;
    float TexW, TexH;
    unsigned int Format;
    unsigned int ID;
    void * TexAddr;
} GlTexture;

typedef struct
{
    char * Name;
    char * TexName;
    unsigned char Container;
    GlTexture Thumbnail;
} DirectoryEntry;

#define FS_DIR_UP   0x01
#define FS_DIR_MAIN 0x10

unsigned int FsIsFile( char *src );

DirEntry * FsParseDirectory( char * directory, unsigned int *entries );

unsigned int FsDirectoryEntryCount( DIR *dp, direntry * ep );

void FsFreeDirectory( DirEntry * dir, unsigned int entries );

#endif
