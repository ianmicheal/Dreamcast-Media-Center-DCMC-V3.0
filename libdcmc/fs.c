/*
**
** FS.c (C) Josh PH3NOM Pearson 2013
** FileSystem Navigation Utilities
**
*/

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "gl.h"

#include "fs.h"

#include "cstring.h"

#include "container.h"

#include "thumbnail.h"

#define VERBOSE

unsigned int FsIsFile( char *src )
{
     unsigned int i = 0, last_index=0;
     
     while(!char_is_break(src[i]))
         if(src[i++]=='.')
             last_index = i;
     
     return last_index;
}

unsigned char FsIsTbn( char * fname )
{
    unsigned int len = cstr_len(fname);
    if((fname[len-4]=='t' || fname[len-4]=='T') &&
       (fname[len-3]=='b' || fname[len-3]=='B') &&
       (fname[len-2]=='n' || fname[len-2]=='N') )
       return 1;
    return 0;
}

unsigned int FsDirectoryEntryCount( DIR *dp, direntry * ep )
{
     DIR * d = dp;
     direntry * e = ep;
     unsigned int i = 0;
     
     while ((e = readdir (d))) ++i;     
     
     return i;
}

DirEntry * FsParseDirectory( char * directory, unsigned int *entries )
{
#ifdef VERBOSE
    printf("FileSystem: ParseDirectory() %s\n", directory );
#endif
    DirEntry *dir, *dptr;
    DIR *dp;
    unsigned int i=0, size;
    direntry *ep;        
    
    dp   = opendir (directory);
    size = FsDirectoryEntryCount(dp,ep);
    closedir(dp);
    dir  = malloc(sizeof(DirEntry)*size);
    dptr = dir;
    dp   = opendir (directory);
         
    while ((ep = readdir (dp)))
    {
        dptr->Name = malloc(sizeof(char)*cstr_len(ep->d_name));
        cstr_copy( ep->d_name, dptr->Name );
        dptr->IsFile = FsIsFile(dptr->Name);
        dptr->Index = i++;
        ++dptr;
    }
    
    *entries = size;
 
    closedir(dp);
 
    return dir;
}

void FsFreeDirectory( DirEntry * dir, unsigned int entries )
{
    unsigned int i;
    DirEntry * dptr = dir;
    for(i=0; i<entries;i++)
    {
        free(dptr->Name);
        ++dptr;
    }
    free(dir);
}

unsigned char FsFixDir( String * dir )
{
    if(dir->String[1]=='d')
    {
        if(dir->String[0]=='c')
        {
           StringSet( dir, "/cd/" );
           return 1;
        }
        if(dir->String[0]=='s')
        {
           StringSet( dir, "/sd/" );
           return 1;
        }
    }
    if( dir->String[0]=='v' && dir->String[1]=='m' && dir->String[2]=='u' )
    {
        StringSet( dir, "/vmu/" );
        return 1;
    }
    if( dir->String[0]=='r' && dir->String[1]=='a' && dir->String[2]=='m' )
    {
        StringSet( dir, "/ram/" );
        return 1;
    }
    if( dir->String[0]=='p' && dir->String[1]=='t' && dir->String[2]=='y' )
    {
        StringSet( dir, "/pty/" );
        return 1;
    }
    return 0;
}

void FsFreeDirectoryEntry( DirectoryEntry * dir, unsigned int entries )
{
    unsigned int i;
    DirectoryEntry * dptr = dir;
    for(i=0; i<entries;i++)
    {
        free(dptr->Name);
        if(dptr->TexName!=NULL)
            pvr_mem_free(dptr->Thumbnail.TexAddr);

        ++dptr;
    }
    free(dir);
}

DirectoryEntry * FsLoadDirectory( char * dirname, unsigned int *entries )
{
    DIR *dp;
    direntry *ep; 
    DirectoryEntry *dir, *dptr;
    unsigned int size;
    
    dp   = opendir (dirname);
    size = FsDirectoryEntryCount(dp,ep);
    closedir(dp);
    dir  = malloc(sizeof(DirectoryEntry)*size);
    dptr = dir;
    dp   = opendir (dirname);    
    
    while((ep = readdir(dp)))
    {
        /* Need to filter out files with .tbn - dont list them */
        if(FsIsTbn(ep->d_name))
        {
            --size;
            continue;
        }
        
        /* Copy the file name dynamically */
        dptr->Name = malloc(sizeof(char)*cstr_len(ep->d_name));
        cstr_copy( ep->d_name, dptr->Name );
        
        /* Determine file container - based on file extension */
        dptr->Container = FsMediaContainer( dptr->Name ); 
        
        /* Check if a file exists with the same name, but extension .tbn */
        if(ThumbnailFind(dptr, dirname))
           ThumbnailLoadPVR(dptr); /* For now, only PVR thumbnail implemented */
         
        ++dptr;
    }
    
    *entries = size;
 
    closedir(dp);
 
    return dir;
}

unsigned char HasExt( char * fname, char * ext )
{
    unsigned int len = cstr_len(fname),
                elen = cstr_len(ext);
                
    if((fname[len-4]==ext[elen-4]) &&
       (fname[len-3]==ext[elen-3]) &&
       (fname[len-2]==ext[elen-2]) )
       return 1;
       
    return 0;
}

DirectoryEntry * FsLoadDirectoryFiltered( char * dirname, unsigned int *entries,
                                          unsigned char container )
{
    DIR *dp;
    direntry *ep; 
    DirectoryEntry *dir, *dptr;
    unsigned int size;
    
    dp   = opendir (dirname);
    size = FsDirectoryEntryCount(dp,ep);
    closedir(dp);
    dir  = malloc(sizeof(DirectoryEntry)*size);
    dptr = dir;
    dp   = opendir (dirname);    
    
    while((ep = readdir(dp)))
    {
        /* Determine file container - based on file extension */
        dptr->Container = FsMediaContainer( ep->d_name ); 
        
        /* Apply File Extension Filter */
        if(( (dptr->Container>container+0xF) || (dptr->Container<container) )
            &&((dptr->Container != CONTAINER_NULL) || FsIsTbn( ep->d_name ) ))
            {
                --size;
                continue;
            }
        
        /* Copy the file name dynamically */
        dptr->Name = malloc(sizeof(char)*cstr_len(ep->d_name));
        cstr_copy( ep->d_name, dptr->Name );       
        
        /* Check if a file exists with the same name, but extension .tbn */
        if(ThumbnailFind(dptr, dirname))
           ThumbnailLoadJPG(dptr); /* For now, only PVR thumbnail implemented */
         
        ++dptr;
    }
    
    *entries = size;
 
    closedir(dp);
 
    return dir;
}
