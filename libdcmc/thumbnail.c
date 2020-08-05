
#include <malloc.h>
#include <stdio.h>

#include "gl.h"

#include "cstring.h"
#include "fs.h"
#include "texture.h"
#include "thumbnail.h"

inline void TextureLoadPVRSQ( void * src, GlTexture * tex );
inline void TextureLoadPVRDMA( unsigned char *src, GlTexture * tex );

void ThumbnailCopy( GlTexture *src, GlTexture *dst )
{
    *dst = *src;
}

unsigned char ThumbnailFind( DirectoryEntry * dir, char * dirname )
{
    FILE * tbnf = NULL;
    
    String * temp = NewString();
    StringSet( temp, dirname );
    StringConcat( temp, dir->Name );

    dir->TexName = malloc(sizeof(char)*temp->Size);
    cstr_copy( temp->String, dir->TexName );
         
    dir->TexName[temp->Size-4] = 't';
    dir->TexName[temp->Size-3] = 'b';
    dir->TexName[temp->Size-2] = 'n';

    StringDeconstruct(temp);
    
    tbnf = fopen( dir->TexName, "rb" );
    
    if(tbnf!=NULL)
    {
        fclose(tbnf);
        return 1;
    }
    else
    {
        free(dir->TexName);
        dir->TexName = NULL;
        return 0;
    }
}

void ThumbnailLoadPVR( DirectoryEntry * dir )
{
    printf("File Name: %s\n", dir->Name );
    printf("TBN Name: %s\n", dir->TexName );   
    
    TexStruct *texture = TexturePVR( dir->TexName );
    
    dir->Thumbnail.ImgW = dir->Thumbnail.TexW = texture->w;
    dir->Thumbnail.ImgH = dir->Thumbnail.TexH = texture->h;
    dir->Thumbnail.TexAddr = texture->data;
    dir->Thumbnail.Format = texture->fmt | texture->col;
    
    printf("Thumbnail wxh: %.0fx%.0f\n", dir->Thumbnail.TexW, dir->Thumbnail.TexH );
   
    glGenTextures(1, &dir->Thumbnail.ID);
    glBindTexture(GL_TEXTURE_2D, dir->Thumbnail.ID );
    glKosTex2D( dir->Thumbnail.Format,
                dir->Thumbnail.TexW, dir->Thumbnail.TexH,
                dir->Thumbnail.TexAddr );

    free(texture);
}

void ThumbnailLoadJPG( DirectoryEntry * dir )
{
    TexStruct *texture = TextureJPG( dir->TexName );
    
    dir->Thumbnail.ImgW = texture->w;
    dir->Thumbnail.ImgH = texture->h;
    
    unsigned int w=0x8,h=0x8;
    while(w<dir->Thumbnail.ImgW)(w<<=1);
    while(h<dir->Thumbnail.ImgH)(h<<=1);
    dir->Thumbnail.TexW = w;
    dir->Thumbnail.TexH = h;
    dir->Thumbnail.Format = texture->fmt | texture->col;
    dir->Thumbnail.TexAddr = pvr_mem_malloc(w*h*2);
    
    /* Things get a little ugly to ensure resolution works with the PVR */
    unsigned short int * buf = memalign(32,w*h*2);
    unsigned short int * txr = texture->data;
    sq_clr( buf, w*h*2 ); // This will make a black texture
    unsigned int r;
    for(r = 0; r < dir->Thumbnail.ImgH; r++)
        memcpy( buf+(r*w), txr+(r*(int)dir->Thumbnail.ImgW), (int)dir->Thumbnail.ImgW*2 );

    pvr_txr_load( buf, dir->Thumbnail.TexAddr, w*h*2);  
    
    glGenTextures(1, &dir->Thumbnail.ID);
    glBindTexture(GL_TEXTURE_2D, dir->Thumbnail.ID );
    glKosTex2D( dir->Thumbnail.Format,
                dir->Thumbnail.TexW, dir->Thumbnail.TexH,
                dir->Thumbnail.TexAddr );
    
    free(texture->data);
    free(texture);
    free(buf);
}

void ThumbnailDecPVR( char * PvrTexName, GlTexture * Thumbnail )
{    
    TexStruct *texture = TexturePVR( PvrTexName );
    
    Thumbnail->ImgW = Thumbnail->TexW = texture->w;
    Thumbnail->ImgH = Thumbnail->TexH = texture->h;
    Thumbnail->TexAddr = texture->data;
    Thumbnail->Format = texture->fmt | texture->col;

    glGenTextures(1, &Thumbnail->ID);
    glBindTexture(GL_TEXTURE_2D, Thumbnail->ID );
    glKosTex2D( Thumbnail->Format,
                Thumbnail->TexW, Thumbnail->TexH,
                Thumbnail->TexAddr );

    free(texture);    
}

void ThumbnailDecPNG( char * PngTexName, GlTexture * Thumbnail, int mode )
{    
    TexStruct *texture = TexturePNG( PngTexName, mode );
    
    Thumbnail->ImgW = texture->w;
    Thumbnail->ImgH = texture->h;
    
    uint32 w=0x8,h=0x8;
    while(w<Thumbnail->ImgW)(w<<=1);
    while(h<Thumbnail->ImgH)(h<<=1);
    Thumbnail->TexW = w;
    Thumbnail->TexH = h;
        
    printf("TexW/H: %ix%i\n", (int)Thumbnail->TexW, (int)Thumbnail->TexH);
    
    Thumbnail->Format = texture->fmt | texture->col;
    
    Thumbnail->TexAddr = pvr_mem_malloc(texture->w*texture->h*2);
    
    TextureLoadPVRSQ( texture->data, Thumbnail );
    
    glGenTextures(1, &Thumbnail->ID);
    glBindTexture(GL_TEXTURE_2D, Thumbnail->ID );
    glKosTex2D( Thumbnail->Format,
                Thumbnail->TexW, Thumbnail->TexH,
                Thumbnail->TexAddr );
    
    free(texture->data);
    free(texture);    
}

void ThumbnailDecJPG( char * PngTexName, GlTexture * Thumbnail )
{    
    TexStruct *texture = TextureJPG( PngTexName );
    
    Thumbnail->ImgW = texture->w;
    Thumbnail->ImgH = texture->h;
    
    uint32 w=0x8,h=0x8;
    while(w<Thumbnail->ImgW)(w<<=1);
    while(h<Thumbnail->ImgH)(h<<=1);
    Thumbnail->TexW = w;
    Thumbnail->TexH = h;
    
    printf("ImgW/H: %ix%i\n", (int)Thumbnail->ImgW, (int)Thumbnail->ImgH);
    printf("TexW/H: %.0fx%.0f\n", Thumbnail->TexW, Thumbnail->TexH);
    Thumbnail->Format = texture->fmt | texture->col;
    
    Thumbnail->TexAddr = pvr_mem_malloc(texture->w*texture->h*2);
        
    TextureLoadPVRSQ( texture->data, Thumbnail );

    glGenTextures(1, &Thumbnail->ID);
    glBindTexture(GL_TEXTURE_2D, Thumbnail->ID );
    glKosTex2D( Thumbnail->Format,
                Thumbnail->TexW, Thumbnail->TexH,
                Thumbnail->TexAddr );
       
    free(texture->data);
    free(texture);    
}

/* Textures sent to PVR using SQ transfers */
inline void TextureLoadPVRSQ( void * src, GlTexture * tex )
{
    int i, n;
    
    printf("PVRSQ: %.2fx%.2f\n", tex->TexW, tex->TexH );
    
    unsigned int *s = (unsigned int *)src;
    unsigned int * d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)tex->TexAddr) & 0x03ffffe0));

    uint32 count =  tex->ImgW*2;
	if (count % 4)
		count = (count & 0xfffffffc) + 4;   

	/* Set store queue memory area as desired */
	QACR0 = ((((unsigned int)tex->TexAddr)>>26)<<2)&0x1c;
	QACR1 = ((((unsigned int)tex->TexAddr)>>26)<<2)&0x1c;

    for(i=0;i<tex->ImgW;i++)
    {                        
	  d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)tex->TexAddr+(i*((int)tex->TexW*2))) & 0x03ffffe0));
	  
	  n = count>>5;
	  
	  while(n--) {
		asm("pref @%0" : : "r" (s + 8)); // prefetch 32 bytes for next loop 
		d[0] = *(s++);
		d[1] = *(s++);
		d[2] = *(s++);
		d[3] = *(s++);
		d[4] = *(s++);
		d[5] = *(s++);
		d[6] = *(s++);
		d[7] = *(s++);
		asm("pref @%0" : : "r" (d));
		d += 8;
	  }
	}
	/* Wait for both store queues to complete */
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;    
}
