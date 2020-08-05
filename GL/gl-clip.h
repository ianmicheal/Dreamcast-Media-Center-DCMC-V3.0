/*
** Gl-Clip.h (C) Josh PH3NOM Pearson 2013
** *Currently not working or implemented into the library
*/

#ifndef GL_CLIP_H
#define GL_CLIP_H

#define NONE   0x0000 /* Clip Codes */
#define FIRST  0x0001
#define SECOND 0x0010
#define THIRD  0x0100
#define ALL    0x0111
#define FIRST_TWO_OUT      0x0011
#define FIRST_AND_LAST_OUT 0x0101
#define LAST_TWO_OUT       0x0110

#define ALPHA 0xFF000000 /* Color Components using PVR's Packed 32bit int */
#define RED   0x00FF0000
#define GREEN 0x0000FF00
#define BLUE  0x000000FF

#define CLIP_ZNEAR 0.00001
#define CLIPZ 0x0F

inline void VertexCopy( pvr_vertex_t *src, pvr_vertex_t *dst )
{
    *dst = *src;
}

inline void VertexClipZNear( pvr_vertex_t *v1, pvr_vertex_t *v2 )
{  
     float MAG = (CLIP_ZNEAR - v1->z)/(v2->z-v1->z); /* Magnitude */
     
     /* Extract Color Components, Apply Linear Interpolation, then Pack it up */
     BYTE a1 = (v1->argb & ALPHA)>>24;
     BYTE r1 = (v1->argb & RED)>>16;
     BYTE g1 = (v1->argb & GREEN)>>8;
     BYTE b1 = (v1->argb & BLUE)>>0;
     BYTE a2 = (v2->argb & ALPHA)>>24;
     BYTE r2 = (v2->argb & RED)>>16;
     BYTE g2 = (v2->argb & GREEN)>>8;
     BYTE b2 = (v2->argb & BLUE)>>0;
     BYTE a  = ((a2-a1)*MAG)+ a1;
     BYTE r  = ((r2-r1)*MAG)+ r1;
     BYTE g  = ((g2-g1)*MAG)+ g1;
     BYTE b  = ((b2-b1)*MAG)+ b1;
     v1->argb = ( (a<<24) | (r<<16) | (g<<8) | (b<<0) );
     
     /* Linear interpolate the U/V data */
     v1->u = ((v2->u-v1->u)*MAG)+v1->u;
     v1->v = ((v2->v-v1->v)*MAG)+v1->v;
     
     /* Linear interpolate the Vertex data */
     v1->x = ((v2->x-v1->x)*MAG) + v1->x;
     v1->y = ((v2->y-v1->y)*MAG) + v1->y;
     v1->z = ((v2->z-v1->z)*MAG) + v1->z;
}

inline void ClipZNearTriStrip( pvr_vertex_t *in, pvr_vertex_t *out, unsigned int *n )
{
    unsigned short int clip = 0, vertices_inside = 0;
    unsigned int v = 0, i;
    
    for( i=0; i<(*n-2); i++ )
    {
        /* Clip codes have been pre-computed and OR'd into the Vertex Flag */ 
        ( in[i+0].flags & CLIPZ ) ? clip |= FIRST  : ++vertices_inside;
        ( in[i+1].flags & CLIPZ ) ? clip |= SECOND : ++vertices_inside;
        ( in[i+2].flags & CLIPZ ) ? clip |= THIRD  : ++vertices_inside;
        
        switch(vertices_inside)
        {
            case 0: // All Vertices are outside of clip window
                 break;
                 
            case 3: // All Vertices are inside of clip window
                 VertexCopy( &in[i+0], &out[v+0] );
                 VertexCopy( &in[i+1], &out[v+1] );
                 VertexCopy( &in[i+2], &out[v+2] );
                 v+=3;
                 
                 break;
                 
            case 1: // 1 Vertex is inside of clip window, 2 are out
                 VertexCopy( &in[i+0], &out[v+0] );
                 VertexCopy( &in[i+1], &out[v+1] );
                 VertexCopy( &in[i+2], &out[v+2] );
                 
                 switch(clip)
                 {
                     case FIRST_TWO_OUT:
                          VertexClipZNear( &out[v], &out[v+2] );
                          VertexClipZNear( &out[v+1], &out[v+2] );
                          
                          break;
                          
                     case FIRST_AND_LAST_OUT:
                          VertexClipZNear( &out[v], &out[v+1] );
                          VertexClipZNear( &out[v+2], &out[v+1] );
                          
                          break;
                     
                     case LAST_TWO_OUT:    
                          VertexClipZNear( &out[v+1], &out[v] );
                          VertexClipZNear( &out[v+2], &out[v] );
                          
                          break;       
                 }
                 v+=3;
                                  
                 break;   
                              
            case 2: // 2 Vertices are inside of clip window, 1 is out
                 switch(clip)
                 {
                     case FIRST:
                          VertexCopy( &in[i+0], &out[v+0] );
                          VertexCopy( &in[i+1], &out[v+1] );
                          VertexClipZNear( &out[v+0], &out[v+1] );
                          v+=2;
                          
                          VertexCopy( &in[i+0], &out[v+0] );
                          VertexCopy( &in[i+2], &out[v+1] );
                          VertexClipZNear( &out[v+0], &out[v+1] );
                          ++v; 
                                                   
                          VertexCopy( &out[v-2], &out[v+0] );
                          VertexCopy( &out[v-1], &out[v+1] );                          
                          VertexCopy( &in[i+2] , &out[v+2] );                      
                          v+=3;
                          
                          break;
                     
                     case SECOND:
                          VertexCopy( &in[i+0], &out[v+0] );
                          VertexCopy( &in[i+1], &out[v+1] );
                          VertexClipZNear( &out[v+1], &out[v+0] );
                          v+=2;

                          VertexCopy( &in[i+2], &out[v+0] );
                          ++v; 

                          VertexCopy( &out[v-2], &out[v+0] );
                          VertexCopy( &out[v-1], &out[v+1] );                          
                          VertexCopy( &in[i+1] , &out[v+2] );                      
                          VertexClipZNear( &out[v+2], &out[v+1] );
                          v+=3;
                                                    
                          break;
                          
                     case THIRD:
                          VertexCopy( &in[i+0], &out[v+0] );
                          VertexCopy( &in[i+1], &out[v+1] );
                          VertexCopy( &in[i+2], &out[v+1] );
                          VertexClipZNear( &out[v+2], &out[v+1] );
                          v+=3;
                          
                          VertexCopy( &out[v-3], &out[v+0] );
                          VertexCopy( &out[v-1], &out[v+1] );
                          v+=2; 

                          VertexCopy( &in[i+2], &out[v] );
                          VertexClipZNear( &out[v+0], &out[v-2] );
                          ++v;
                                                    
                          break;
                 }                 
                 break;                            
        }
        vertices_inside = clip = 0;       
    }
    *n = v;
}

#endif
