/*
** DCMC V.2.0 (C) Josh PH3NOM Pearson 2010-2013
*/

#include "dcmc.h"

/* Local Function Prototcols */
void MainMenu( Font * font );

int main()
{
     printf("Dreamcast Media Center v.2.0 (C) Josh PH3NOM Pearson 2010-2013\n");

     pvr_init_defaults();

     glKosInit();  /* Init the GL */

	 glMatrixMode(GL_PROJECTION); /* Set up GL Projection Matrix */
	 glLoadIdentity();
	 gluPerspective(45.0f,vid_mode->width/(float)vid_mode->height,0.01f,500.0f);
	 glMatrixMode(GL_MODELVIEW);
	 glLoadIdentity();
               
     /* Load an ASCII font texture */     
          
    // Font * font = DCMC_LoadFontTex("/cd/tbn/font0.pvr",CONTAINER_PNG,1);
    Font * font = DCMC_LoadFontTex("/cd/tbn/font0.pvr",CONTAINER_PVR,2);
      
     MainMenu( font ); 
        
     return 1;
}

void DirectoryAdd( DirectoryEntry * dir, char * name, GlTexture * thumbnail )
{
    dir->Name = malloc(sizeof(char)*cstr_len(name));
    cstr_copy( name, dir->Name );
    dir->Container = FsMediaContainer( dir->Name ); 
    dir->TexName = NULL;
    ThumbnailCopy( thumbnail, &dir->Thumbnail );
}

void MainMenu( Font * font )
{
    /* Initialize DCMC */
    DCMC_InitTextures();
    DCMC_InitFileDirectory();

    /* Contrive a directory listing to use for the main menu */
    DirectoryEntry * dir = malloc(sizeof(DirectoryEntry)*4);
    DirectoryEntry * dptr = dir;
    DirectoryAdd( dptr++, "PICTURES\0", DCMC_TexID(BgndPicture) );
    DirectoryAdd( dptr++, "MOVIES\0",   DCMC_TexID(BgndVideo) );
    DirectoryAdd( dptr++, "MUSIC\0",    DCMC_TexID(BgndMusic) );
    DirectoryAdd( dptr,   "PROGRAMS\0", DCMC_TexID(BgndProgram) );
    
    Input input; /* User input class */
    unsigned char entries = 4;
    float w = vid_mode->width;
           
    while(1)
    {
        glKosBeginFrame(); /* Begin GL/Kos Render */
        
        dptr = dir+FDIR; /* Current Directory Entry */
        
        DCMC_RenderGlTexture( &dptr->Thumbnail, 0,60,w,360 ); 
  // 320.0f, 420.0f, 20.0f, 20.0f
        /* Handle The Render of the current Directory Listing */
        DCMC_RenderDirectory( dir, font, FDIR, entries,  16.0f ); 
               
	    glKosFinishFrame();  
	    
	    DCE_SetInput( &input, 0 );
	    DCE_GetInput( &input );
	        
        if(input.u || input.l)
	        if(FDIR>0)
	        {
	            int x = 0;
	            for(x=0; x<w; x+=w/20.0f)
	            {
                      glKosBeginFrame(); /* Begin GL/Kos Render */
        
                      dptr = dir+FDIR; /* Current Directory Entry */
                      DCMC_RenderGlTexture( &dptr->Thumbnail, x,60,w,360 ); 
                      dptr = dir+FDIR-1; /* Previous Directory Entry */
                      DCMC_RenderGlTexture( &dptr->Thumbnail, x-w,60,w,360 ); 
        
                      /* Handle The Render of the current Directory Listing */
                      DCMC_RenderDirectory( dir, font, FDIR, entries, 16.0f ); //16f is font size
               
	                  glKosFinishFrame();                       
                }
                FDIR--;
              }	    
        if(input.d || input.r)
	        if(FDIR<entries-1)
	        {
	            int x = 0;
	            for(x=0; x>-w; x-=w/20.0f)
	            {
                      glKosBeginFrame(); /* Begin GL/Kos Render */
        
                      dptr = dir+FDIR; /* Current Directory Entry */
                      DCMC_RenderGlTexture( &dptr->Thumbnail, x,60,w,360 ); 
                      dptr = dir+FDIR+1; /* Next Directory Entry */
                      DCMC_RenderGlTexture( &dptr->Thumbnail, x+w,60,w,360 ); 
        
                      /* Handle The Render of the current Directory Listing */
                      DCMC_RenderDirectory( dir, font, FDIR, entries, 16.0f ); //16f is font size
               
	                  glKosFinishFrame();                       
                }
                FDIR++;
              }	 
	              
        if(input.a)
            DCMC_LoadDirectory( font, &dptr->Thumbnail );

    }
}
