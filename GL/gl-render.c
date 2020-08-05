/*
**
** OpenGL Render API
** (C) Josh 'PH3NOM' Pearson 2013
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gl.h"

/******************************************************************************/

typedef struct
{
    pvr_poly_hdr_t hdr;
    pvr_vertex_t   * vertex;
    DWORD            vertices;
}RenderNode;

typedef struct
{
    uint32 flag;
    void * ptr;
}VertexFlagNode;

typedef struct
{
    GLenum  target;
    GLint  level;
    GLint  internalFormat;
    unsigned short int  width;
    unsigned short int  height;
    GLint  border;
    GLenum  format;
    GLenum  type;
    GLvoid *  data;
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr __attribute__((aligned(32)));
} GlTexCxt;

/******************************************************************************/

#define GL_MAX_VERTICES 1024*32
static pvr_vertex_t VERTEX_BUFFER[GL_MAX_VERTICES] __attribute__((aligned(32)));
static DWORD        VERTICES = 0;

#define OP 0x00
#define TR 0x01
static BYTE BLEND_MODE = OP;

static RenderNode VERT_LIST[2][1024*16];
static DWORD      RSIZE[2] = {0,0};

static int    GL_PRIMITIVE = -1;
static BYTE   GL_BIND_TEX = 0;

static void RenderCallback();
static BYTE RenderInit = 0;

/******************************************************************************/
/* We will use some pre-built structures to expedite determining vertex flags */

static VertexFlagNode TriFlag[3];
static VertexFlagNode QuadFlag[4];
static VertexFlagNode * flag_iterator;

void SetTriFlags()
{
    TriFlag[0].flag = PVR_CMD_VERTEX;
    TriFlag[0].ptr = &TriFlag[1];   
    TriFlag[1].flag = PVR_CMD_VERTEX;
    TriFlag[1].ptr = &TriFlag[2];
    TriFlag[2].flag = PVR_CMD_VERTEX_EOL;
    TriFlag[2].ptr = &TriFlag[0];
}

void SetQuadFlags()
{
    QuadFlag[0].flag = PVR_CMD_VERTEX;
    QuadFlag[0].ptr = &QuadFlag[1];   
    QuadFlag[1].flag = PVR_CMD_VERTEX;
    QuadFlag[1].ptr = &QuadFlag[2];
    QuadFlag[2].flag = PVR_CMD_VERTEX;
    QuadFlag[2].ptr = &QuadFlag[3];
    QuadFlag[3].flag = PVR_CMD_VERTEX_EOL;
    QuadFlag[3].ptr = &QuadFlag[0];
}

void RenderInitVert()
{
    SetTriFlags();
    SetQuadFlags();
}

/* Blending API ***************************************************************/
/*
** A brief of what is going on here:
** When a call is made to change blending modes, an intermediate flag is set.
** We will wait for a call to glBegin to actually compile the flags into the 
** PVR Poly Context. This should make things flow as GL and the PVR expect.
** A big change over the previous KGLX, now we can change blending modes
** freely without having to call glKosFinsihList() to enable transparency.
** This means we can submit transparent vertices as GL expects them to work.
** glKosFinishList() is supported in this API for backwards compatibility,
** but now it simply makes a call to glEnable(GL_BLEND).  The difference is
** glKosFinishList() will only apply to that frame, and then reset back to OP.
*/

#define GL_BLEND_OPAQUE PVR_LIST_OP_POLY
#define GL_BLEND_TRANS  PVR_LIST_TR_POLY

static BYTE GL_BLEND_FUNC = GL_BLEND_OPAQUE;
static BYTE BLEND_SRC     = PVR_BLEND_ONE;
static BYTE BLEND_DST     = PVR_BLEND_ZERO;
static BYTE GL_SHADE_FUNC = PVR_SHADE_GOURAUD;
static BYTE GL_CULL_FUNC  = PVR_CULLING_NONE;
static BYTE DEPTH_FUNC    = PVR_DEPTHCMP_NEVER;
static BYTE DEPTH_ENABLE  = PVR_DEPTHWRITE_ENABLE;
static BYTE GL_TXR_ENV    = PVR_TXRENV_REPLACE;

void glDepthFunc(GLenum func)
{
    if( (func-GL_NEVER) < PVR_DEPTHCMP_NEVER ||
        (func-GL_ALWAYS) > PVR_DEPTHCMP_ALWAYS )
        return;
    DEPTH_FUNC = func-GL_NEVER;
}

void glDepthMask(GLboolean flag)
{
    if(flag)
        DEPTH_ENABLE = PVR_DEPTHWRITE_ENABLE;
    else
        DEPTH_ENABLE = PVR_DEPTHWRITE_DISABLE;
}

void glClearDepthf(GLfloat depth)
{ 
    if(!depth) 
       DEPTH_ENABLE = PVR_DEPTHWRITE_DISABLE;
    else
       DEPTH_ENABLE = PVR_DEPTHWRITE_ENABLE;
}

void glBlendFunc( GLenum sfactor, GLenum dfactor )
{
    switch(sfactor)
    {
         case GL_ONE:
              BLEND_SRC = PVR_BLEND_ONE;
              break;
                   
         case GL_ZERO:
              BLEND_SRC = PVR_BLEND_ZERO;
              break;
                              
         case GL_SRC_COLOR:
              BLEND_SRC = PVR_BLEND_SRCALPHA;
              break;
                    
         case GL_DST_COLOR:
              BLEND_SRC = PVR_BLEND_DESTCOLOR;
              break; 
                   
         case GL_SRC_ALPHA:
              BLEND_SRC = PVR_BLEND_SRCALPHA;
              break;
                   
         case GL_DST_ALPHA:
              BLEND_SRC = PVR_BLEND_DESTALPHA;
              break;  

         case GL_ONE_MINUS_SRC_ALPHA:
              BLEND_SRC = PVR_BLEND_INVSRCALPHA;
              break;  
                   
         case GL_ONE_MINUS_DST_ALPHA:
              BLEND_SRC = PVR_BLEND_INVDESTALPHA;
              break;  
                   
         case GL_ONE_MINUS_DST_COLOR:
              BLEND_SRC = PVR_BLEND_INVDESTCOLOR;
              break;                     
    }
    
    switch(dfactor)
    {
         case GL_ONE:
              BLEND_DST = PVR_BLEND_ONE;
              break;
                   
         case GL_ZERO:
              BLEND_DST = PVR_BLEND_ZERO;
              break;
                              
         case GL_SRC_COLOR:
              BLEND_DST = PVR_BLEND_SRCALPHA;
              break;
                    
         case GL_DST_COLOR:
              BLEND_DST = PVR_BLEND_DESTCOLOR;
              break; 
                   
         case GL_SRC_ALPHA:
              BLEND_DST = PVR_BLEND_SRCALPHA;
              break;
                   
         case GL_DST_ALPHA:
              BLEND_DST = PVR_BLEND_DESTALPHA;
              break;  

         case GL_ONE_MINUS_SRC_ALPHA:
              BLEND_DST = PVR_BLEND_INVSRCALPHA;
              break;  
                   
         case GL_ONE_MINUS_DST_ALPHA:
              BLEND_DST = PVR_BLEND_INVDESTALPHA;
              break;  
                   
         case GL_ONE_MINUS_DST_COLOR:
              BLEND_DST = PVR_BLEND_INVDESTCOLOR;
              break;                     
    }
}

void glShadeModel( GLenum   mode)
{
    switch(mode)
    {
        case GL_FLAT:
             GL_SHADE_FUNC = PVR_SHADE_FLAT;
             break;
         
        case GL_SMOOTH:
             GL_SHADE_FUNC = PVR_SHADE_GOURAUD;
             break;
    }
}

/* Texture API *****************************************************************/
/*
** A brief of what is going on here:
** If you look at the KGL texture api, you will see things are done quite
** differently here.
** KGL is using dynamic memory for each texture, and returns a pointer to the
** allocated space when a call to glGenTextures is called.
** Furthermore, KGL uses memcpy to copy the allocated texture back into its
** local context before compiling and submiting to the PVR.
** Instead of that, here I use a fixed-size internal array of texture contexts.
** Next, I keep two counters:
** -Textures is used to keep track of how many textures have been bound to gl
    ( and therefore filled in the internal array )
** -TexIndex is used to point to the currently bound texture, and therefore all
    operations ( blending, shading, clipping, etc. ) will be applied to the
    texture bound at TexIndex
** Position 0 in the array is used internally for polygons that are not textued
** 
** So, when the client makes a call to glGenTextures, we only need to return
** the position in the internal array.
** And then when the client makes a call to glBindTexture, all we need to do
** is set TexIndex to that value returned by glGenTextures.
**
*/    

static GlTexCxt TexArr[256];
static GLuint   Textures=0;
static GLuint   TexIndex = 1;
static GLint    TextureFilter = PVR_FILTER_BILINEAR;
static GLuint   TextureMemory=0;

void glGenTextures( GLsizei n, GLuint * textures )
{
    while(n--)
        *textures++ = Textures++;
}

void glBindTexture( GLenum  target, GLuint texture )
{
    if(target==GL_TEXTURE_2D)
    {
        TexIndex = texture;
        if(RenderInit) /* Render is open, bind the active texture to the render context */
            GL_BIND_TEX = 1;
    }
}

void glTexImage2D( GLenum target, GLint level, GLint internalFormat,
                     GLsizei width, GLsizei height, GLint border,
                     GLenum format, GLenum type, GLvoid * data )
{	                   
    if( target!=GL_TEXTURE_2D ) return;

    TexArr[TexIndex].target = target;
    TexArr[TexIndex].level = level;
    TexArr[TexIndex].internalFormat = internalFormat;       
    TexArr[TexIndex].width = width;
    TexArr[TexIndex].height = height;
    TexArr[TexIndex].border = border;
    TexArr[TexIndex].format = format;
    TexArr[TexIndex].type = type;
    TexArr[TexIndex].data = pvr_mem_malloc(width*height*2);
    sq_cpy( TexArr[TexIndex].data, data, width*height*2 );
#ifdef debug
    TextureMemory+=width*height*2;
    printf("GL Texture Memory Allocated: %i\n", TextureMemory );
#endif
}

/* KOS shortcut function (if you loaded your own texture) */
void glKosTex2D(GLint internal_fmt, GLsizei width, GLsizei height,
                pvr_ptr_t txr_address) 
{
    TexArr[TexIndex].internalFormat = internal_fmt;       
    TexArr[TexIndex].width = width;
    TexArr[TexIndex].height = height;
    TexArr[TexIndex].border = 0;
    TexArr[TexIndex].format = 0;
    TexArr[TexIndex].type = 0;
    TexArr[TexIndex].data = txr_address;   
}

void glTexParameteri( GLenum target, GLenum pname, GLint param )
{
    if(target==GL_TEXTURE_2D)
    {
        switch(pname)
        {        
              case GL_TEXTURE_MAG_FILTER:
              case GL_TEXTURE_MIN_FILTER:
                  switch(param)
                  {                          
                      case GL_LINEAR:
                           TextureFilter = PVR_FILTER_BILINEAR;
                           break;
                           
                      case GL_NEAREST:
                           TextureFilter = PVR_FILTER_NEAREST;
                           break;
                       
                      case GL_FILTER_NONE: /* Compatabile with deprecated kgl */
                           TextureFilter = PVR_FILTER_NONE;
                           break;
                           
                      case GL_FILTER_BILINEAR: /* Compatabile with deprecated kgl */
                           TextureFilter = PVR_FILTER_BILINEAR;
                           break;

                      default: /* Some mip-map levels maybe */
                           TextureFilter = PVR_FILTER_TRILINEAR1;
                           break;                      
                  }
                  break;
                  
              case GL_TEXTURE_WRAP_S:
              case GL_TEXTURE_WRAP_T: 
                   break;                               
        }
    }
}

void glTexEnvi( GLenum target, GLenum pname, GLint param )
{
    if( target == GL_TEXTURE_ENV
        || target == GL_TEXTURE_2D ) // Not correct, added for kgl compatibility
        if( param>=PVR_TXRENV_REPLACE && param <=PVR_TXRENV_MODULATEALPHA )
            GL_TXR_ENV = param; // PVR and GL are equivalent
}

void glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{ /* GL_TEXTURE_LOD_BIAS Not Implemented */
    glTexEnvi( target, pname, param );
}

/* This is not a public funciton - only called once internally */
void RenderInitCxt()
{      
    pvr_poly_cxt_col( &TexArr[TexIndex].cxt, PVR_LIST_OP_POLY );
  
    TexArr[TexIndex].cxt.gen.alpha = PVR_ALPHA_ENABLE;
    TexArr[TexIndex].cxt.gen.shading = GL_SHADE_FUNC;
    TexArr[TexIndex].cxt.gen.fog_type = PVR_FOG_DISABLE;
    TexArr[TexIndex].cxt.gen.culling = GL_CULL_FUNC;
    TexArr[TexIndex].cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
    TexArr[TexIndex].cxt.gen.clip_mode = PVR_USERCLIP_DISABLE;
    
    TexArr[TexIndex].cxt.blend.src = BLEND_SRC;
    TexArr[TexIndex].cxt.blend.dst = BLEND_DST;
    //TexArr[TexIndex].cxt.blend.src_enable = PVR_BLEND_ENABLE;
   // TexArr[TexIndex].cxt.blend.dst_enable = PVR_BLEND_DISABLE;
    
    //TexArr[TexIndex].cxt.depth.comparison = DEPTH_FUNC;
    TexArr[TexIndex].cxt.depth.write = PVR_DEPTHWRITE_DISABLE;
    
    TexArr[TexIndex].cxt.txr.enable = PVR_TEXTURE_DISABLE;
 
    pvr_poly_compile(&TexArr[TexIndex].hdr, &TexArr[TexIndex].cxt);

    ++Textures;
}

/* Vertex API *****************************************************************/
/*
** A brief of what is going on here:
** This vertex api has been built to handle mixed submission of opaque and 
** transparent polygons.  At the core of this api is a Vertex Buffer used
** to store all of the vertices bound to GL in main RAM.
** To keep things simple, vertex parameters are stored individually until a
** call to glVertex is made.  When glVertex is called, all of the parameters
** will applied to the vertex, and the coordinates be transformed using a macro
** based on mat_transform_single into the internal Vertex Buffer.
** The trick here is to keep some efficient pointers so we know what PVR list
** to submit the vertex to, when glKosFinishFrame() is called and we finally
** flush the Vertex Buffer to the PVR, using DMA or StoreQueues, depending on
** weather or not the flag RENDER_DMA is enabled when compiled.
** By default DMA is disabled, it is mainly there for a future implementation
** using a double-buffered approach, where we can write to one DMA buffer
** while the other is being transferred in a non-blocking method to the PVR.
** Looking at the code, this could be faster than the KOS DMA implementation.
*/

static uint32 GL_COLOR        = 0xFFFFFFFF;
static uint32 GL_BASE_COLOR   = 0xFFFFFFFF;
static uint32 GL_COLOR_OFFSET = 0x00000000;
static float  GL_UV[2] = {0.0f,0.0f};
static BYTE   GL_AUTO_UV = 0; /* kgl backwards compatible */
static BYTE   GL_CLIPZ = 0;
static float  invw;

static float UV[4][2] =
{
      { 0.0f, 0.0f},
      { 1.0f, 0.0f},
      { 1.0f, 1.0f},
      { 0.0f, 1.0f }
};

void glColor1ui( uint32 c )
{
    GL_COLOR = c;
}

void glColor3f( float r, float g, float b )
{
    GL_COLOR = PVR_PACK_COLOR( 1.0f, r, g, b );
}

void glColor3fv( float * rgb )
{
    GL_COLOR = PVR_PACK_COLOR( 1.0f, rgb[0], rgb[1], rgb[2] );
}

void glColor4f( float r, float g, float b, float a )
{
    GL_COLOR = PVR_PACK_COLOR( a, r, g, b );
}

void glColor4fv( float * rgba )
{
    GL_COLOR = PVR_PACK_COLOR( rgba[3], rgba[0], rgba[1], rgba[2] );
}

void glColor4ub( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
    GL_COLOR = 0xFF00FFFF;
    //GL_COLOR = ( (a)<<24 | (r)<<16 | (g)<<8 | (b)<<0 );
}

void glTexCoord2f( float u, float v )
{
    GL_UV[0] = u;
    GL_UV[1] = v;
}

void glTexCoord2fv( float *uv )
{
    GL_UV[0] = uv[0];
    GL_UV[1] = uv[1];
}

void glNormal3f( float x, float y, float z )
{ /* Not Implemented */ }

static DWORD quad_index=0;
void NextQuadIndex() /* Quads: Make the vertex order work with PVR TriStrips */
{
    if(++quad_index>3) quad_index=0;
}

/* 2D Vertex - No need to transform */
void glVertex2f( float x, float y )
{
    DWORD V = VERTICES; /* Quads: Make the vertex order work with PVR TriStrips */
    
    if(GL_PRIMITIVE==GL_QUADS) 
    {
      BYTE index = quad_index;

      switch(quad_index)
      {
        case 0:
             V++;
             index++;
             break;
        case 1:
             V--;
             index--;
             break;

      }            
      NextQuadIndex();
    }     
    
    VERTEX_BUFFER[V].z = 1; 
    VERTEX_BUFFER[V].x = x; 
    VERTEX_BUFFER[V].y = y;
    VERTEX_BUFFER[V].u = GL_UV[0];
    VERTEX_BUFFER[V].v = GL_UV[1];
    VERTEX_BUFFER[V].argb = GL_COLOR;
    VERTEX_BUFFER[V].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[V].flags = flag_iterator->flag;
                                    
    flag_iterator = (VertexFlagNode*)flag_iterator->ptr;
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

// Thanks to Tapamn for advise on small optimization
void glVertex3f( float x, float y, float z )
{
    register float __x __asm__("fr0") = x; 
	register float __y __asm__("fr1") = y; 
	register float __z __asm__("fr2") = z; 
	register float __w __asm__("fr3") = 1;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx,fv0\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );  
    
    DWORD V = VERTICES; /* Quads: Make the vertex order work with PVR TriStrips */
    if(GL_PRIMITIVE==GL_QUADS) 
    {
      BYTE index = quad_index;

      if(GL_AUTO_UV)
      {
         GL_UV[0] = UV[index][0];
         GL_UV[1] = UV[index][1];
      }
      
      switch(quad_index)
      {
        case 0:
             V++;
             index++;
             break;
        case 1:
             V--;
             index--;
             break;

      }            
      NextQuadIndex();
    }     
    
    VERTEX_BUFFER[V].z = invw = 1/__w; 
    VERTEX_BUFFER[V].x = __x*invw; 
    VERTEX_BUFFER[V].y = __y*invw;
    VERTEX_BUFFER[V].u = GL_UV[0];
    VERTEX_BUFFER[V].v = GL_UV[1];
    VERTEX_BUFFER[V].argb = GL_COLOR;
    VERTEX_BUFFER[V].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[V].flags = flag_iterator->flag;
                                    
    flag_iterator = (VertexFlagNode*)flag_iterator->ptr;
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

void glVertex3fv( float *xyz )
{
    register float __x __asm__("fr0") = xyz[0]; 
	register float __y __asm__("fr1") = xyz[1]; 
	register float __z __asm__("fr2") = xyz[2]; 
	register float __w __asm__("fr3") = 1;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx,fv0\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );  
    
    DWORD V = VERTICES; /* Quads: Make the vertex order work with PVR TriStrips */
    if(GL_PRIMITIVE==GL_QUADS) 
    {
      BYTE index = quad_index;

      if(GL_AUTO_UV)
      {
         GL_UV[0] = UV[index][0];
         GL_UV[1] = UV[index][1];
      }
      
      switch(quad_index)
      {
        case 0:
             V++;
             index++;
             break;
        case 1:
             V--;
             index--;
             break;

      }            
      NextQuadIndex();
    }   
        
    VERTEX_BUFFER[V].z = invw = 1/__w; 
    VERTEX_BUFFER[V].x = __x*invw; 
    VERTEX_BUFFER[V].y = __y*invw;
    VERTEX_BUFFER[V].u = GL_UV[0];
    VERTEX_BUFFER[V].v = GL_UV[1];
    VERTEX_BUFFER[V].argb = GL_COLOR;
    VERTEX_BUFFER[V].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[V].flags = flag_iterator->flag;
                                    
    flag_iterator = (VertexFlagNode*)flag_iterator->ptr;
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

void glVertex2fv( float *xy )
{
    DWORD V = VERTICES; /* Quads: Make the vertex order work with PVR TriStrips */
    
    if(GL_PRIMITIVE==GL_QUADS) 
    {
      BYTE index = quad_index;

      switch(quad_index)
      {
        case 0:
             V++;
             index++;
             break;
        case 1:
             V--;
             index--;
             break;

      }            
      NextQuadIndex();
    }     
    
    VERTEX_BUFFER[V].z = 1; 
    VERTEX_BUFFER[V].x = xy[0]; 
    VERTEX_BUFFER[V].y = xy[1];
    VERTEX_BUFFER[V].u = GL_UV[0];
    VERTEX_BUFFER[V].v = GL_UV[1];
    VERTEX_BUFFER[V].argb = GL_COLOR;
    VERTEX_BUFFER[V].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[V].flags = flag_iterator->flag;
                                    
    flag_iterator = (VertexFlagNode*)flag_iterator->ptr;
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

/* Global API *****************************************************************/

void glInitRender()
{
    RenderInitCxt();    /* Initialize our Render Context */
    RenderInitVert();   
}

void glClearColor( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) 
{
    //GL_BASE_COLOR = PVR_PACK_COLOR( alpha, red, green, blue );
}

void glClear( int mode )
{ /* Not Implemented */ }

void glEnable( int mode )
{
    switch(mode)
    {
        case GL_TEXTURE_2D:
             break;

        case GL_BLEND:
             GL_BLEND_FUNC = GL_BLEND_TRANS;
             BLEND_MODE = TR;
             break;
        
        case GL_DEPTH_TEST:
             DEPTH_ENABLE = PVR_DEPTHWRITE_ENABLE;
             break;
        
        case GL_KOS_NEARZ_CLIPPING:
             GL_CLIPZ = 1;
             break;     
    }
}

void glDisable( int mode )
{
    switch(mode)
    {
        case GL_TEXTURE_2D:
             GL_BIND_TEX=0;
             break;

        case GL_BLEND:
             GL_BLEND_FUNC = GL_BLEND_OPAQUE;
             BLEND_MODE = OP;
             break;
             
        case GL_DEPTH_TEST:
             DEPTH_ENABLE = PVR_DEPTHWRITE_DISABLE;
             break;
        
        case GL_KOS_NEARZ_CLIPPING:
             GL_CLIPZ = 0;
             break;
    }
}

extern void glKosMatrixApplyRender();

#include "gl-pvr.h"

inline void glKosCompileHeaderTx()
{
   /*
    pvr_poly_cxt_col( &TexArr[TexIndex].cxt, GL_BLEND_FUNC );
 
    TexArr[TexIndex].cxt.gen.alpha = BLEND_MODE;
    TexArr[TexIndex].cxt.gen.shading = GL_SHADE_FUNC;
    TexArr[TexIndex].cxt.gen.fog_type = PVR_FOG_DISABLE;
    TexArr[TexIndex].cxt.gen.culling = GL_CULL_FUNC;
    TexArr[TexIndex].cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
    TexArr[TexIndex].cxt.gen.clip_mode = PVR_USERCLIP_DISABLE;
    //TexArr[TexIndex].cxt.gen.clip_mode = PVR_USERCLIP_INSIDE;
    
    if(BLEND_MODE)
    {
    TexArr[TexIndex].cxt.blend.src = BLEND_SRC;
    TexArr[TexIndex].cxt.blend.dst = BLEND_DST;
    }
    else
    {
    TexArr[TexIndex].cxt.blend.src = PVR_BLEND_ONE;
    TexArr[TexIndex].cxt.blend.dst = PVR_BLEND_ZERO;
    }    
    //TexArr[TexIndex].cxt.blend.src_enable = PVR_BLEND_ENABLE;
   // TexArr[TexIndex].cxt.blend.dst_enable = PVR_BLEND_DISABLE;
    
    //TexArr[TexIndex].cxt.depth.comparison = DEPTH_FUNC;
    TexArr[TexIndex].cxt.depth.write = DEPTH_ENABLE;
    
    TexArr[TexIndex].cxt.txr.enable = GL_BIND_TEX;
    TexArr[TexIndex].cxt.txr.filter = TextureFilter;
    TexArr[TexIndex].cxt.txr.uv_flip = PVR_UVFLIP_NONE;
    TexArr[TexIndex].cxt.txr.uv_clamp = PVR_UVCLAMP_NONE;
    TexArr[TexIndex].cxt.txr.alpha = !BLEND_MODE;
    TexArr[TexIndex].cxt.txr.env = GL_TXR_ENV;  

    TexArr[TexIndex].cxt.txr.enable = PVR_TEXTURE_ENABLE;
    TexArr[TexIndex].cxt.txr.filter = TextureFilter;
    TexArr[TexIndex].cxt.txr.mipmap = 0;
    TexArr[TexIndex].cxt.txr.mipmap_bias = 0;
    TexArr[TexIndex].cxt.txr.uv_flip = PVR_UVFLIP_NONE;
    TexArr[TexIndex].cxt.txr.uv_clamp = PVR_UVCLAMP_NONE;
    TexArr[TexIndex].cxt.txr.alpha = PVR_TXRALPHA_ENABLE;
    TexArr[TexIndex].cxt.txr.env = GL_TXR_ENV;
    
    TexArr[TexIndex].cxt.txr.width = TexArr[TexIndex].width;
    TexArr[TexIndex].cxt.txr.height = TexArr[TexIndex].height;
    TexArr[TexIndex].cxt.txr.format = TexArr[TexIndex].type | TexArr[TexIndex].internalFormat,
    TexArr[TexIndex].cxt.txr.base = TexArr[TexIndex].data;
*/
    //printf("GL: Compile Textured Poly\n");

    //pvr_poly_cxt_col( &TexArr[TexIndex].cxt, GL_BLEND_FUNC );

	gl_poly_cxt_txr(&TexArr[TexIndex].cxt, TexArr[TexIndex].type,
                    TexArr[TexIndex].internalFormat,
 		            TexArr[TexIndex].width, TexArr[TexIndex].height,
                    TexArr[TexIndex].data );

    TexArr[TexIndex].cxt.gen.alpha = BLEND_MODE;
    TexArr[TexIndex].cxt.gen.shading = GL_SHADE_FUNC;
    TexArr[TexIndex].cxt.gen.fog_type = PVR_FOG_DISABLE;
    TexArr[TexIndex].cxt.gen.culling = GL_CULL_FUNC;
    TexArr[TexIndex].cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
    TexArr[TexIndex].cxt.gen.clip_mode = PVR_USERCLIP_DISABLE;

    if(BLEND_MODE)
    {
    TexArr[TexIndex].cxt.blend.src = BLEND_SRC;
    TexArr[TexIndex].cxt.blend.dst = BLEND_DST;
    TexArr[TexIndex].cxt.txr.alpha = PVR_TXRALPHA_ENABLE;
    }
    else
    {
    TexArr[TexIndex].cxt.blend.src = PVR_BLEND_ONE;
    TexArr[TexIndex].cxt.blend.dst = PVR_BLEND_ZERO;
    TexArr[TexIndex].cxt.txr.alpha = PVR_TXRALPHA_DISABLE;
    }    
    
    TexArr[TexIndex].cxt.depth.write = DEPTH_ENABLE;
    
    gl_poly_compile(&TexArr[TexIndex].hdr, &TexArr[TexIndex].cxt);     
    
    memcpy( &VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].hdr,
         &TexArr[TexIndex].hdr, sizeof(TexArr[TexIndex].hdr) );
    
}

inline void glKosCompileHeader()
{
    //printf("GL: Compile Non-Textured Poly\n");
    pvr_poly_cxt_col( &TexArr[0].cxt, GL_BLEND_FUNC );

    TexArr[0].cxt.gen.alpha = BLEND_MODE;
    TexArr[0].cxt.gen.shading = GL_SHADE_FUNC;
    TexArr[0].cxt.gen.fog_type = PVR_FOG_DISABLE;
    TexArr[0].cxt.gen.culling = GL_CULL_FUNC;
    TexArr[0].cxt.gen.color_clamp = PVR_CLRCLAMP_DISABLE;
    TexArr[0].cxt.gen.clip_mode = PVR_USERCLIP_DISABLE;

    if(BLEND_MODE)
    {
    TexArr[0].cxt.blend.src = BLEND_SRC;
    TexArr[0].cxt.blend.dst = BLEND_DST;
    }
    else
    {
    TexArr[0].cxt.blend.src = PVR_BLEND_ONE;
    TexArr[0].cxt.blend.dst = PVR_BLEND_ZERO;
    }    
    
    TexArr[0].cxt.depth.write = DEPTH_ENABLE;

    gl_poly_compile( &TexArr[0].hdr, &TexArr[0].cxt );

    memcpy( &VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].hdr,
         &TexArr[0].hdr, sizeof(TexArr[0].hdr) );
}

void glBegin( int mode )
{
    GL_COLOR = GL_BASE_COLOR;
    GL_COLOR_OFFSET = 0xFFFFFFFF;
    GL_PRIMITIVE = mode;

    glKosMatrixApplyRender(); /* Apply the Render Matrix Stack */
    
    /* Set the pointers into the Vertex Buffer */
    VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertex = &VERTEX_BUFFER[VERTICES];
    VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices = 0;
    //VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].hdr = &TexArr[TexIndex].hdr;
    
    /* Compile the PVR polygon context with the currently enabled flags */
    GL_BIND_TEX ? glKosCompileHeaderTx() : glKosCompileHeader();
    
    switch(GL_PRIMITIVE) /* Determine vertex flags for the vertex buffer */
    {
        case GL_TRIANGLES:
             flag_iterator = &TriFlag[0];
             break;
             
        case GL_QUADS:
             flag_iterator = &QuadFlag[0];
             break;
             
        case GL_POINTS:
             // ToDo
             break;
             
        case GL_TRIANGLE_STRIP:
             // ToDo
             break;
    }
}

static inline void glPreStack()
{
    GL_COLOR = GL_BASE_COLOR;
    GL_COLOR_OFFSET = 0xFFFFFFFF;
    
    glKosMatrixApplyRender(); /* Apply the Render Matrix Stack */
    
    /* Set the pointers into the Vertex Buffer */
    VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertex = &VERTEX_BUFFER[VERTICES];
    VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices = 0;
    //VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].hdr = &TexArr[TexIndex].hdr;
    
    /* Compile the PVR polygon context with the currently enabled flags */
    GL_BIND_TEX ? glKosCompileHeaderTx() : glKosCompileHeader();
}

#include "gl-clip.h"
#include "gl-vertex.h"

void glEnd()
{
    if(GL_CLIPZ)
    {
       int v = VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
    
       ClipZNearTriStrip( VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertex,
                       VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertex+
                       VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices,
                       &v );
    
       VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertex = 
                       VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertex+
                       VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
                   
       VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices = v;
    }    
    
    ++RSIZE[BLEND_MODE];
    
    GL_BIND_TEX=0;
}


static BYTE kglxDirtyDirty = 0;

void glKosBeginFrame()
{
    VERTICES = 0;
    RenderInit = 1;
    if(kglxDirtyDirty)
    {
        glDisable(GL_BLEND);
        kglxDirtyDirty=0;
    }
}

void glKosFinishFrame()
{
    RenderCallback();
    RenderInit = 0;
}

void glKosFinishList() /* This func is only here for backwards compatibility */
{
    glEnable(GL_BLEND);
    kglxDirtyDirty=1; 
}


/* Vertex Pointer API *********************************************************/

//#include "gl-vertex.h"

void glVertexPointer( GLint size, GLenum type,
                      GLsizei stride, const GLvoid * pointer )
{
    if(size!=3) return; /* Expect 3D X,Y,Z vertex... could do 2d X,Y later */
    
    if(type!=GL_FLOAT) return; /* Expect Floating point vertices */

    (stride) ? (GL_VERTEX_STRIDE = stride) : (GL_VERTEX_STRIDE = 3); 
    
    GL_VERTEX_POINTER = (float*)pointer;
    
    GL_VERTEX_MODE |= GL_USE_ARRAY;
}

void glTexCoordPointer( GLint size, GLenum type,
                        GLsizei stride, const GLvoid * pointer )
{
    if(size!=2) return; /* Expect u and v */
    
    if(type!=GL_FLOAT) return; /* Expect Floating point vertices */

    (stride) ? (GL_TEXCOORD_STRIDE = stride) : (GL_TEXCOORD_STRIDE = 2);    
    
    GL_TEXCOORD_POINTER = (float*)pointer;
    
    GL_VERTEX_MODE |= GL_USE_TEXTURE;
}

void glColorPointer( GLint size, GLenum type,
                     GLsizei stride, const GLvoid * pointer )
{ 
    if( (type==GL_UNSIGNED_INT) && (size==1) )
    {
        GL_COLOR_COMPONENTS = 1;
        GL_COLOR_POINTER_U32 = (uint32*)pointer;    
    }
    else if( (type==GL_FLOAT) && (size==3) )
    {
        GL_COLOR_COMPONENTS = 3;
        GL_COLOR_POINTER = (float*)pointer;
    }
    else if( (type==GL_FLOAT) && (size==4) )
    {
        GL_COLOR_COMPONENTS = 4;
        GL_COLOR_POINTER = (float*)pointer;
    }
    else
        return;

    (stride) ? (GL_COLOR_STRIDE = stride) : (GL_COLOR_STRIDE = size);    
      
    GL_VERTEX_MODE |= GL_USE_COLOR;
}

void glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
    /* Make sure user has set a vertex pointer */
    if(!(GL_VERTEX_MODE & GL_USE_ARRAY))
    {
        GL_VERTEX_MODE = 0;
        return;
    }
    
    /* Determine transform function pointer */
    switch( mode )
    {
      case GL_TRIANGLE_STRIP:
         
        switch( GL_VERTEX_MODE ) 
        {
          case GL_USE_TEXTURE:
               Transform = &glRenderTriStripT;
               TransformLast = &glRenderTriStripLT;
               break;
        
          case GL_USE_COLOR:
               switch(GL_COLOR_COMPONENTS)
               {
                   case 1:
                       Transform = &glRenderTriStripC1ui;
                       TransformLast = &glRenderTriStripLC1ui;
                       break;
                       
                   case 3:
                       Transform = &glRenderTriStripC3f;
                       TransformLast = &glRenderTriStripLC3f;
                       break;
                       
                   case 4:
                       Transform = &glRenderTriStripC4f;
                       TransformLast = &glRenderTriStripLC4f;
                       break;
               }
               break;
        
          case GL_USE_TEXTURE_COLOR:
               switch(GL_COLOR_COMPONENTS)
               {
                   case 1:
                       Transform = &glRenderTriStripTC1ui;
                       TransformLast = &glRenderTriStripLTC1ui;
                       break;      

                   case 3:
                       Transform = &glRenderTriStripTC3f;
                       TransformLast = &glRenderTriStripLTC3f;
                       break;  
                              
                   case 4:
                       Transform = &glRenderTriStripTC4f;
                       TransformLast = &glRenderTriStripLTC4f;
                       break;                      
               }
               break;
               
          default:
               Transform = &glRenderTriStrip;
               TransformLast = &glRenderTriStripL;
               break;            
        }
        
        break;
      
      case GL_TRIANGLES:

        switch( GL_VERTEX_MODE )
        {
          case GL_USE_TEXTURE:
               Transform = &glRenderTriT;
               break;
        
          case GL_USE_COLOR:
               Transform = &glRenderTriC;
               break;
        
          case GL_USE_TEXTURE_COLOR:
               Transform = &glRenderTriTC;
               break;             
        
          default:
               Transform = &glRenderTri;
               break;            
        }
        
        break;
    
      default: /* Only Triangles and Triangle Strips supported by the PVR */
        GL_VERTEX_MODE = 0; /* Other modes could be added in future builds */
        return;
            
    }

    GL_VERTEX_POINTER   += first;       /* Pointer Offset */  
    GL_TEXCOORD_POINTER += first;
    GL_COLOR_POINTER    += first;
  
    register float __x __asm__("fr12");  /* Vertex Registers */
	register float __y __asm__("fr13"); 
	register float __z __asm__("fr14");
	register float __w __asm__("fr15");
    
    if(mode==GL_TRIANGLE_STRIP)
    {
       glPreStack();
       
       while(--count)
           Transform();
       TransformLast();
       
       glEnd();
    }
    else
    {
       glBegin(mode);
       
       while(count--)
           Transform();
       
       glEnd();
    }
    
    GL_VERTEX_MODE = 0;
}

/* Render API *****************************************************************/

#include "gl-render.h"

static void RenderCallback()
{
    DWORD i;
    pvr_wait_ready();
    pvr_scene_begin(); 
           
    pvr_list_begin(PVR_LIST_OP_POLY);
    
    //pvr_set_clip(0, 64, 640, 480-128);
      
#ifndef RENDER_DMA  
	QACR0 = QACR1 = QACRTA;
#endif    
    //printf("GL: OP: %i\n", RSIZE[OP] );
    for(i=0;i<RSIZE[OP];i++)
    {
#ifdef RENDER_DMA
        dcache_flush_range((uint32)VERT_LIST[OP][i].hdr, 0x20);
        pvr_dma_load_ta((pvr_poly_hdr_t*)VERT_LIST[OP][i].hdr, 0x20, 1, NULL, 0 );
        dcache_flush_range((uint32)VERT_LIST[OP][i].vertex, 0x20*VERT_LIST[OP][i].vertices);
        pvr_dma_load_ta((pvr_vertex_t*)VERT_LIST[OP][i].vertex, 0x20*VERT_LIST[OP][i].vertices, 1, NULL, 0);
#else
        sq_cpy_ta(&VERT_LIST[OP][i].hdr,  0x20 );
        sq_cpy_ta((pvr_vertex_t*)VERT_LIST[OP][i].vertex, 0x20*VERT_LIST[OP][i].vertices );
#endif        
    }
    RSIZE[OP] = 0;
    
    pvr_list_finish(); 
    pvr_list_begin(PVR_LIST_TR_POLY); 
    
    //printf("GL: TR: %i\n", RSIZE[TR] );
    for(i=0;i<RSIZE[TR];i++)
    {
#ifdef RENDER_DMA
        dcache_flush_range((uint32)VERT_LIST[TR][i].hdr, 0x20);
        pvr_dma_load_ta((pvr_poly_hdr_t*)VERT_LIST[TR][i].hdr, 0x20, 1, NULL, 0);
        dcache_flush_range((uint32)VERT_LIST[TR][i].vertex, 0x20*VERT_LIST[TR][i].vertices);
        pvr_dma_load_ta((pvr_vertex_t*)VERT_LIST[TR][i].vertex, 0x20*VERT_LIST[TR][i].vertices, 1, NULL, 0);
#else
        sq_cpy_ta(&VERT_LIST[TR][i].hdr,  0x20 );
        sq_cpy_ta((pvr_vertex_t*)VERT_LIST[TR][i].vertex, 0x20*VERT_LIST[TR][i].vertices );
#endif
    }
    RSIZE[TR] = 0;
    
    pvr_list_finish(); 
    pvr_scene_finish();    
}


