/*
**  GL-Vertex (C) 2013 Josh 'PH3NOM' Pearson
** 
**  Global Vertex Pointer variables and functions
*/

#ifndef GL_VERTEX_H
#define GL_VERTEX_H

static float * GL_VERTEX_POINTER = NULL;
static GLsizei GL_VERTEX_STRIDE = 0; 

static float * GL_TEXCOORD_POINTER = NULL;
static GLsizei GL_TEXCOORD_STRIDE = 0; 

static float * GL_COLOR_POINTER = NULL;
static uint32 * GL_COLOR_POINTER_U32 = NULL;
static GLsizei GL_COLOR_STRIDE = 0; 
static GLsizei GL_COLOR_COMPONENTS = 0;

#define GL_USE_ARRAY         0x0001
#define GL_USE_TEXTURE       0x0011
#define GL_USE_COLOR         0x0101
#define GL_USE_TEXTURE_COLOR 0x0111

static unsigned short int GL_VERTEX_MODE = 0;

void (*Transform)() = NULL;
void (*TransformLast)() = NULL;

register float __x __asm__("fr12"); 
register float __y __asm__("fr13"); 
register float __z __asm__("fr14");
register float __w __asm__("fr15");

#define CLIP_ZNEAR 0.00001
#define CLIPZ 0x0F

/* Triangles Submission */

static inline void glRenderTri()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    VERTEX_BUFFER[VERTICES].argb = 0xFFFFFFFF;  
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;

    VERTEX_BUFFER[VERTICES].flags = flag_iterator->flag;
    flag_iterator = (VertexFlagNode*)flag_iterator->ptr; 
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
    
    GL_VERTEX_POINTER += GL_VERTEX_STRIDE; /* Add Pointer Stride */
}

static inline void glRenderTriT()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;
    VERTEX_BUFFER[VERTICES].u = *GL_TEXCOORD_POINTER++;
    VERTEX_BUFFER[VERTICES].v = *GL_TEXCOORD_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = GL_COLOR;
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    
    VERTEX_BUFFER[VERTICES].flags = flag_iterator->flag;
                                    
    flag_iterator = (VertexFlagNode*)flag_iterator->ptr;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_TEXCOORD_POINTER += GL_TEXCOORD_STRIDE;
}

static inline void glRenderTriC()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    float a,r,g,b;
    r = *GL_COLOR_POINTER++;
    g = *GL_COLOR_POINTER++;
    b = *GL_COLOR_POINTER++;
    a = *GL_COLOR_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( a, r, g, b );
    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    
    VERTEX_BUFFER[VERTICES].flags = flag_iterator->flag;
                                    
    flag_iterator = (VertexFlagNode*)flag_iterator->ptr;
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_COLOR_POINTER += GL_COLOR_STRIDE;
}

static inline void glRenderTriTC()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    

    VERTEX_BUFFER[VERTICES].u = *GL_TEXCOORD_POINTER++;
    VERTEX_BUFFER[VERTICES].v = *GL_TEXCOORD_POINTER++;
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    float a,r,g,b;
    r = *GL_COLOR_POINTER++;
    g = *GL_COLOR_POINTER++;
    b = *GL_COLOR_POINTER++;
    a = *GL_COLOR_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( a, r, g, b );
    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    
    VERTEX_BUFFER[VERTICES].flags = flag_iterator->flag;
                                    
    flag_iterator = (VertexFlagNode*)flag_iterator->ptr; 
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER   += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_TEXCOORD_POINTER += GL_TEXCOORD_STRIDE;
    GL_COLOR_POINTER    += GL_COLOR_STRIDE;
}

/* Triangle Strip Submission */

static inline void glRenderTriStrip()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    VERTEX_BUFFER[VERTICES].argb = 0xFFFFFFFF;  
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER += GL_VERTEX_STRIDE; /* Add Pointer Stride */
}

static inline void glRenderTriStripL()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
 
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    VERTEX_BUFFER[VERTICES].argb = 0xFFFFFFFF;
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX_EOL;
    
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

static inline void glRenderTriStripT()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;
    VERTEX_BUFFER[VERTICES].u = *GL_TEXCOORD_POINTER++;
    VERTEX_BUFFER[VERTICES].v = *GL_TEXCOORD_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = GL_COLOR;
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER   += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_TEXCOORD_POINTER += GL_TEXCOORD_STRIDE;
}

static inline void glRenderTriStripLT()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;
    VERTEX_BUFFER[VERTICES].u = *GL_TEXCOORD_POINTER++;
    VERTEX_BUFFER[VERTICES].v = *GL_TEXCOORD_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = GL_COLOR;
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX_EOL;
    
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

static inline void glRenderTriStripC4f()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    float a,r,g,b;
    r = *GL_COLOR_POINTER++;
    g = *GL_COLOR_POINTER++;
    b = *GL_COLOR_POINTER++;
    a = *GL_COLOR_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( a, r, g, b );
    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_COLOR_POINTER  += GL_COLOR_STRIDE;
}

static inline void glRenderTriStripLC4f()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    float a,r,g,b;
    r = *GL_COLOR_POINTER++;
    g = *GL_COLOR_POINTER++;
    b = *GL_COLOR_POINTER++;
    a = *GL_COLOR_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( a, r, g, b );

    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX_EOL;
    
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

static inline void glRenderTriStripC3f()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    float r,g,b;
    r = *GL_COLOR_POINTER++;
    g = *GL_COLOR_POINTER++;
    b = *GL_COLOR_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( 1.0f, r, g, b );
    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_COLOR_POINTER  += GL_COLOR_STRIDE;
}

static inline void glRenderTriStripLC3f()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    float r,g,b;
    r = *GL_COLOR_POINTER++;
    g = *GL_COLOR_POINTER++;
    b = *GL_COLOR_POINTER++;
    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( 1.0f, r, g, b );

    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX_EOL;
    
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}


static inline void glRenderTriStripC1ui()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    VERTEX_BUFFER[VERTICES].argb = *GL_COLOR_POINTER_U32++;   
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_COLOR_POINTER  += GL_COLOR_STRIDE;
}

static inline void glRenderTriStripLC1ui()
{
    __x = *GL_VERTEX_POINTER++; 
	__y = *GL_VERTEX_POINTER++; 
	__z = *GL_VERTEX_POINTER++;
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    VERTEX_BUFFER[VERTICES].argb = *GL_COLOR_POINTER_U32++;   
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX_EOL;
    
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

/****************************************************************************/

static inline void glRenderTriStripTC4f()
{
    __x = GL_VERTEX_POINTER[0]; 
	__y = GL_VERTEX_POINTER[1]; 
	__z = GL_VERTEX_POINTER[2];
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    

    VERTEX_BUFFER[VERTICES].u = GL_TEXCOORD_POINTER[0];
    VERTEX_BUFFER[VERTICES].v = GL_TEXCOORD_POINTER[1];
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    /* Compute Clip Code */
    if( VERTEX_BUFFER[VERTICES].z < CLIP_ZNEAR )
        VERTEX_BUFFER[VERTICES].flags |= CLIPZ;
    
    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( GL_COLOR_POINTER[3],
                                                   GL_COLOR_POINTER[0],
                                                   GL_COLOR_POINTER[1],
                                                   GL_COLOR_POINTER[2] );
    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER   += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_TEXCOORD_POINTER += GL_TEXCOORD_STRIDE;
    GL_COLOR_POINTER    += GL_COLOR_STRIDE;
}

static inline void glRenderTriStripLTC4f()
{
    __x = GL_VERTEX_POINTER[0]; 
	__y = GL_VERTEX_POINTER[1]; 
	__z = GL_VERTEX_POINTER[2];
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    

    VERTEX_BUFFER[VERTICES].u = GL_TEXCOORD_POINTER[0];
    VERTEX_BUFFER[VERTICES].v = GL_TEXCOORD_POINTER[1];
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    /* Compute Clip Code */
    if( VERTEX_BUFFER[VERTICES].z < CLIP_ZNEAR )
        VERTEX_BUFFER[VERTICES].flags |= CLIPZ;

    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( GL_COLOR_POINTER[3],
                                                   GL_COLOR_POINTER[0],
                                                   GL_COLOR_POINTER[1],
                                                   GL_COLOR_POINTER[2] );
    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX_EOL;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}


static inline void glRenderTriStripTC3f()
{
    __x = GL_VERTEX_POINTER[0]; 
	__y = GL_VERTEX_POINTER[1]; 
	__z = GL_VERTEX_POINTER[2];
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    

    VERTEX_BUFFER[VERTICES].u = GL_TEXCOORD_POINTER[0];
    VERTEX_BUFFER[VERTICES].v = GL_TEXCOORD_POINTER[1];
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    /* Compute Clip Code */
    if( VERTEX_BUFFER[VERTICES].z < CLIP_ZNEAR )
        VERTEX_BUFFER[VERTICES].flags |= CLIPZ;

    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( 1.0f, GL_COLOR_POINTER[0],
                                                         GL_COLOR_POINTER[1],
                                                         GL_COLOR_POINTER[2] );
    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER   += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_TEXCOORD_POINTER += GL_TEXCOORD_STRIDE;
    GL_COLOR_POINTER    += GL_COLOR_STRIDE;
}

static inline void glRenderTriStripLTC3f()
{
    __x = GL_VERTEX_POINTER[0]; 
	__y = GL_VERTEX_POINTER[1]; 
	__z = GL_VERTEX_POINTER[2];
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    

    VERTEX_BUFFER[VERTICES].u = GL_TEXCOORD_POINTER[0];
    VERTEX_BUFFER[VERTICES].v = GL_TEXCOORD_POINTER[1];
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;

    /* Compute Clip Code */
    if( VERTEX_BUFFER[VERTICES].z < CLIP_ZNEAR )
        VERTEX_BUFFER[VERTICES].flags |= CLIPZ;

    VERTEX_BUFFER[VERTICES].argb = PVR_PACK_COLOR( 1.0f, GL_COLOR_POINTER[0],
                                                         GL_COLOR_POINTER[1],
                                                         GL_COLOR_POINTER[2] );
    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX_EOL;  
       
    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

static inline void glRenderTriStripTC1ui()
{
    __x = GL_VERTEX_POINTER[0]; 
	__y = GL_VERTEX_POINTER[1]; 
	__z = GL_VERTEX_POINTER[2];
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    

    VERTEX_BUFFER[VERTICES].u = GL_TEXCOORD_POINTER[0];
    VERTEX_BUFFER[VERTICES].v = GL_TEXCOORD_POINTER[1];
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;
    
    VERTEX_BUFFER[VERTICES].argb = *GL_COLOR_POINTER_U32;    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX;  
    
    /* Compute Clip Code */
    if( VERTEX_BUFFER[VERTICES].z < CLIP_ZNEAR )
        VERTEX_BUFFER[VERTICES].flags |= CLIPZ;

    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;

    GL_VERTEX_POINTER   += GL_VERTEX_STRIDE; /* Add Pointer Stride */
    GL_TEXCOORD_POINTER += GL_TEXCOORD_STRIDE;
    GL_COLOR_POINTER    += GL_COLOR_STRIDE;
}

static inline void glRenderTriStripLTC1ui()
{
    __x = GL_VERTEX_POINTER[0]; 
	__y = GL_VERTEX_POINTER[1]; 
	__z = GL_VERTEX_POINTER[2];
	__w = 1.0f;
	
	__asm__ __volatile__( 
		"ftrv	xmtrx, fv12\n" 
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w)
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) );    

    VERTEX_BUFFER[VERTICES].u = GL_TEXCOORD_POINTER[0];
    VERTEX_BUFFER[VERTICES].v = GL_TEXCOORD_POINTER[1];
    
    VERTEX_BUFFER[VERTICES].z = invw = 1/__w; 
    VERTEX_BUFFER[VERTICES].x = __x*invw; 
    VERTEX_BUFFER[VERTICES].y = __y*invw;
    
    VERTEX_BUFFER[VERTICES].argb = *GL_COLOR_POINTER_U32;    
    VERTEX_BUFFER[VERTICES].oargb = GL_COLOR_OFFSET;
    VERTEX_BUFFER[VERTICES].flags = PVR_CMD_VERTEX_EOL;  
    
    /* Compute Clip Code */
    if( VERTEX_BUFFER[VERTICES].z < CLIP_ZNEAR )
        VERTEX_BUFFER[VERTICES].flags |= CLIPZ;

    ++VERTICES;
    ++VERT_LIST[BLEND_MODE][RSIZE[BLEND_MODE]].vertices;
}

#endif
