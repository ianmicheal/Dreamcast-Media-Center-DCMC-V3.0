/*
** Dreamcast Media Center (C) Josh PH3NOM Pearson 2010-2013
*/

#include "dcmc.h"


static GlTexture DCMC_Thumbnail[16];
static String FILE_DIR[4];

/** DCMC Initialization Routines **********************************************/

void DCMC_InitFileDirectory()
{
    StringSet( &FILE_DIR[DIR_IMG], "/\0" );
    StringSet( &FILE_DIR[DIR_VID], "/\0" );
    StringSet( &FILE_DIR[DIR_AUD], "/\0" );
    StringSet( &FILE_DIR[DIR_BIN], "/\0" );
    FDIR = DIR_IMG;
}

void DCMC_InitTextures()
{
     ThumbnailDecPVR( "/cd/tbn/dcmc/videos.pvr", &DCMC_Thumbnail[0] );
     ThumbnailDecPVR( "/cd/tbn/dcmc/music.pvr", &DCMC_Thumbnail[1] );
     ThumbnailDecPVR( "/cd/tbn/dcmc/pictures.pvr", &DCMC_Thumbnail[2] );
     ThumbnailDecPVR( "/cd/tbn/dcmc/programs.pvr", &DCMC_Thumbnail[3] );
     ThumbnailDecPVR( "/cd/tbn/dcmc/system.pvr", &DCMC_Thumbnail[4] );
     ThumbnailDecPVR( "/cd/tbn/dcmc/settings.pvr", &DCMC_Thumbnail[5] );
     ThumbnailDecPVR( "/cd/tbn/dcmc/DefaultAlbumCover.pvr", &DCMC_Thumbnail[6] );
     ThumbnailDecPVR( "/cd/tbn/dcmc/DefaultVideoCover.pvr", &DCMC_Thumbnail[7] );
     ThumbnailDecPVR( "/cd/tbn/dcmc/DefaultPicture.pvr", &DCMC_Thumbnail[8] );
}

/** DCMC Texture Routines *****************************************************/

GlTexture * DCMC_TexID( int param )
{
    return &DCMC_Thumbnail[param];
}

Font * DCMC_LoadFontTex(char *filename, int container, int mode )
{
    TexStruct *texture;
    
    switch(container)
    {
       case CONTAINER_PVR:
            texture = TexturePVR( filename );
            break;

       case CONTAINER_JPG:
            texture = TextureJPG( filename );
            break;

       case CONTAINER_PNG:
            texture = TexturePNG( filename, mode );
            break;
    } 
    
    Font *font = FontInit( texture->data, texture->fmt | texture->col,
                           texture->w, texture->h, 10, 10,
                           PVR_PACK_COLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

    free(texture);

    return font;
}

void DCMC_HandleTextureInput( Input *inp, float *x, float *y, float *z )
{
    DCE_SetInput(inp,0);
    DCE_GetInput(inp);
    
    if(inp->lt>0.f) *z+=.01f;
    if(inp->rt>0.f) *z-=.01f;
    if(inp->l)      *x-=.01f;
    if(inp->r)      *x+=.01f;
    if(inp->u)      *y+=.01f;
    if(inp->d)      *y-=.01f; 
}

void DCMC_HandleTexture( GlTexture * glTex )
{
    Input cont;
    DCE_SetInput(&cont,0);
    
    float z = -1.78f;
    float x = 0.0f;
    float y = 0.0f;
    
    while(!cont.st)
    {
        glKosBeginFrame();

        DCMC_RenderGlTexture3D( x, y, z, glTex->ImgW/glTex->ImgH, glTex );
        
        glKosFinishFrame();
        
        DCMC_HandleTextureInput( &cont, &x, &y, &z ); 
    }
}

void DCMC_LibJPG_Decode( char * fname )
{
    GlTexture * gltex = malloc(sizeof(GlTexture));
    
    ThumbnailDecJPG( fname, gltex );
    
    DCMC_HandleTexture( gltex );
    
    pvr_mem_free(gltex->TexAddr);
    free(gltex);
}

void DCMC_LibPNG_Decode( char * fname )
{
    GlTexture * gltex = malloc(sizeof(GlTexture));
    
    ThumbnailDecPNG( fname, gltex, 0 );
    
    DCMC_HandleTexture( gltex );
    
    pvr_mem_free(gltex->TexAddr);
    free(gltex);
}

void DCMC_DrawPictureBgnd()
{
    glKosBeginFrame();
        
    DCMC_RenderGlTexture( DCMC_TexID(BgndPicture), 0,60,
                          vid_mode->width,vid_mode->width*(1.f/(16.f/9.f)) );
        
    glKosFinishFrame();
    
    thd_sleep(500);
}

void DCMC_DrawMusicBgnd()
{
    glKosBeginFrame();
        
    DCMC_RenderGlTexture( DCMC_TexID(BgndMusic), 0,60,
                          vid_mode->width,vid_mode->width*(1.f/(16.f/9.f)) );
              
    glKosFinishFrame();
    
    thd_sleep(500);
}

void DCMC_DrawVideoBgnd()
{
    glKosBeginFrame();
        
    DCMC_RenderGlTexture( DCMC_TexID(BgndVideo), 0,60,
                          vid_mode->width,vid_mode->width*(1.f/(16.f/9.f)) );
        
    glKosFinishFrame();
    
    thd_sleep(500);
}

void DCMC_DrawAppBgnd()
{
    glKosBeginFrame();
        
    DCMC_RenderGlTexture( DCMC_TexID(BgndProgram), 0,60,
                          vid_mode->width,vid_mode->width*(1.f/(16.f/9.f)) );
        
    glKosFinishFrame();
    
    thd_sleep(500);
}

void DCMC_LoadFile( Font * font, char * file, DirectoryEntry * dir_entry )
{
    switch(dir_entry->Container)
    {
        case CONTAINER_MP3:
             DCMC_LibMPG123_Decode( file, font, dir_entry );
             DCMC_DrawMusicBgnd();
             break;

        case CONTAINER_AC3:
             DCMC_LibA52_Decode( file, font, dir_entry );
             DCMC_DrawMusicBgnd();
             break;
             
        case CONTAINER_ADX:
             DCMC_LibADX_Decode( file, font, dir_entry );
             DCMC_DrawMusicBgnd();
             break;

        case CONTAINER_MP4:
        case CONTAINER_M4A:
        case CONTAINER_AAC:
             DCMC_DrawMusicBgnd();
             LibFAAD_Decode( file, font, dir_entry );
             break;

        case CONTAINER_FLAC:
             DCMC_DrawMusicBgnd();
             LibFLAC_Decode( file, font, dir_entry );
             DCMC_DrawMusicBgnd();
             break;
             
        case CONTAINER_AVI:
             DCMC_DrawVideoBgnd();
             LibXVID_Decode( file, font );
             DCMC_DrawVideoBgnd();
             break;
                     
        case CONTAINER_MPG:
             DCMC_DrawVideoBgnd();
             LibMPEG2_Decode( file, font );
             DCMC_DrawVideoBgnd();
             break;
             
        case CONTAINER_SFD:
             DCMC_DrawVideoBgnd();
             LibMPEG2_DecodeSFD( file, font );
             DCMC_DrawVideoBgnd();
             break;

        case CONTAINER_JPG:
             DCMC_LibJPG_Decode( file );
             DCMC_DrawPictureBgnd();
             break;

        case CONTAINER_PNG:
             DCMC_LibPNG_Decode( file );
             DCMC_DrawPictureBgnd();
             break;

        case CONTAINER_BIN:
             bin_exec( file );
             break;        
    }
             
}

void DCMC_LoadDirectory( Font *font, GlTexture * bgnd )
{
    glKosBeginFrame(); /* Begin GL/Kos Render */
        
    DCMC_RenderGlTexture( bgnd, 0,60,640,360 ); /* Render FSEntry Thumbnail */
              
    glKosFinishFrame();  
	
    thd_sleep(1000);
        
    return DCMC_ParseDirectory( font, bgnd );
}

/** DCMC Render Routines ******************************************************/

void DCMC_RenderQuad( float x, float y, float w, float h, uint32 color )
{
    glColor1ui( color );  
    glVertex2f( x, y );
    glVertex2f( x, y+h );     
    glVertex2f( x+w, y+h );    
    glVertex2f( x+w, y );
}

void DCMC_RenderGlTexture( GlTexture * tbn, float x, float y, float w, float h )
{
    float u = tbn->ImgW/tbn->TexW;
    float v = tbn->ImgH/tbn->TexH;
    
    glBindTexture( GL_TEXTURE_2D, tbn->ID );  
    
    glBegin(GL_QUADS);
        glTexCoord2f( 0, v );
        glVertex2f( x, y+h );
        glTexCoord2f( u, v );
        glVertex2f( x+w, y+h );
        glTexCoord2f( u, 0 );
        glVertex2f( x+w, y );
        glTexCoord2f( 0, 0 );
        glVertex2f( x, y );      
    glEnd();
}

void DCMC_RenderGlTexture3D(float x, float y, float z, float DAR, GlTexture * tbn)
{
	float u = tbn->ImgW/tbn->TexW;
	float v = tbn->ImgH/tbn->TexH;
    float invDar = 1.0f/DAR;
    
    glLoadIdentity();    
    glTranslatef(x,y,z);
    
    glBindTexture( GL_TEXTURE_2D, tbn->ID );
    
	glBegin(GL_QUADS);
	    glTexCoord2f( 0, 0 );
		glVertex3f(-1.0f, 1.0f*invDar, 0.f);
		glTexCoord2f( u, 0 );
		glVertex3f( 1.0f, 1.0f*invDar, 0.f);
		glTexCoord2f( u, v );
		glVertex3f( 1.0f,-1.0f*invDar, 0.f);
		glTexCoord2f( 0, v );
		glVertex3f(-1.0f,-1.0f*invDar, 0.f);
	glEnd();
}


void DCMC_RenderGlTexture3DPoint( GlTexture * tbn, float x, float y, float z,
                                  float w, float h )
{
	float u = tbn->ImgW/tbn->TexW;
	float v = tbn->ImgH/tbn->TexH;   
    float xf = w/vid_mode->width;
    float yf = h/vid_mode->height;
    
    glLoadIdentity();    
    glTranslatef(x,y,z);
    
    glBindTexture( GL_TEXTURE_2D, tbn->ID );
    
	glBegin(GL_QUADS);
	    glTexCoord2f( 0, 0 );
		glVertex3f(-1.0f*xf, 1.0f*yf, 0.f);
		glTexCoord2f( u, 0 );
		glVertex3f( 1.0f*xf, 1.0f*yf, 0.f);
		glTexCoord2f( u, v );
		glVertex3f( 1.0f*xf,-1.0f*yf, 0.f);
		glTexCoord2f( 0, v );
		glVertex3f(-1.0f*xf,-1.0f*yf, 0.f);
	glEnd();
}

void DCMC_RenderTbn( DirectoryEntry * fs_dir, float x, float y, float w, float h )
{
    if(fs_dir->TexName!=NULL) /* Entry contains a thumbnail, use it */      
        DCMC_RenderGlTexture( &fs_dir->Thumbnail, x, y, w, h ); 
        
    /* If no thumbnail is provided by user, supply a default */
    else if( fs_dir->Container == CONTAINER_NULL )
        DCMC_RenderGlTexture( DCMC_TexID(TbnFolder), x, y, w, h );
    else if( fs_dir->Container < CONTAINER_AUDIO )
        DCMC_RenderGlTexture( DCMC_TexID(TbnProgram), x, y, w, h );
    else if( fs_dir->Container < CONTAINER_VIDEO )
        DCMC_RenderGlTexture( DCMC_TexID(TbnAudio), x, y, w, h );
    else if( fs_dir->Container < CONTAINER_IMAGE )
        DCMC_RenderGlTexture( DCMC_TexID(TbnVideo), x, y, w, h );
    else if( fs_dir->Container < CONTAINER_BIN )
        DCMC_RenderGlTexture( DCMC_TexID(TbnPicture), x, y, w, h );
    else if( fs_dir->Container == CONTAINER_BIN )
        return DCMC_TexID(TbnProgram);
}

GlTexture * DCMC_TbnPtr( DirectoryEntry * fs_dir )
{
    if(fs_dir->TexName!=NULL) /* Entry contains a thumbnail, use it */      
        return &fs_dir->Thumbnail; 
        
    /* If no thumbnail is provided by user, supply a default */
    else if( fs_dir->Container == CONTAINER_NULL )
        return DCMC_TexID(TbnFolder);
    else if( fs_dir->Container < CONTAINER_AUDIO )
        return DCMC_TexID(TbnProgram);
    else if( fs_dir->Container < CONTAINER_VIDEO )
        return DCMC_TexID(TbnAudio);
    else if( fs_dir->Container < CONTAINER_IMAGE )
        return DCMC_TexID(TbnVideo);
    else if( fs_dir->Container < CONTAINER_BIN )
        return DCMC_TexID(TbnPicture);
    else if( fs_dir->Container == CONTAINER_BIN )
        return DCMC_TexID(TbnProgram);
}

void DCMC_RenderDirectory( DirectoryEntry * dir, Font * font, int entry,
                           int entries, float size )
{
    float x,y=96.0f,w=size,h=size;
    unsigned int i;
    DirectoryEntry *dptr = dir;
    
    glBindTexture( GL_TEXTURE_2D, font->TexId );  
    
    glBegin(GL_QUADS);
    
    for(i=0;i<entries;i++)
    {
        float width = cstr_len(dptr->Name)*w;
        x = ((vid_mode->width/3.2f)/2.0f)-(width/2.0f);
         
        FontPrintString( font, dptr->Name, x, y, w, h );
     
        if(entry==i)
        {
            FontPrintString( font, "-\0", x-w-w, y, w, h );
            FontPrintString( font, "-\0", x+width, y, w, h );
        }
         
        y+=h*2.0f; ++dptr;
    }
    
    glEnd();
}

void DCMC_RenderDirectoryLS( DirectoryEntry * dir, Font * font, int entry,
                             int entries, float size )
{
    float x,y=428.0f,w=size,h=size;
    unsigned int i;
    DirectoryEntry *dptr = dir+entry;
    
    glBindTexture( GL_TEXTURE_2D, font->TexId );  
    
    glBegin(GL_QUADS);

        float width = cstr_len(dptr->Name)*w;
        
        x = ((vid_mode->width)/2.0f)-(width/2.0f);
         
        FontPrintString( font, dptr->Name, x, y, w, h );
    
    glEnd();
}

void DCMC_HandleTexture2( GlTexture * glTex, GLfloat x, float y, float z )
{
    Input cont;
    DCE_SetInput(&cont,0);
        
    while(!cont.st)
    {
        glKosBeginFrame();

        DCMC_RenderGlTexture3D( x, y, z, glTex->ImgW/glTex->ImgH, glTex );
        
        glKosFinishFrame();
        
        DCMC_HandleTextureInput( &cont, &x, &y, &z ); 
    }
}

void DCMC_ShiftLayerForward( vector3f *varr, DirectoryEntry * fs_dir, 
                             int fs_entries, int fs_entry,
                             int frames, float xstep, float zstep,
                             Font *font, float size, GlTexture * bgnd,
                             char *curName )
{
    int i,o;
    float w=size, h=size,y=436.f;

    for(i=0;i<frames;i++)
    {
        glKosBeginFrame();
        
        DCMC_RenderGlTexture( bgnd, 0,60,640,360 );
         
        for(o=0;o<fs_entries; o++)
            DCMC_RenderGlTexture3DPoint( DCMC_TbnPtr(fs_dir+o),
                                   varr[o][0],varr[o][1],varr[o][2],
                                   vid_mode->width/3.f, vid_mode->height*(1/(16.f/9.f)) );

        glBindTexture( GL_TEXTURE_2D, font->TexId );  
    
        glBegin(GL_QUADS);
            
            float width = cstr_len(curName)*w;
        
            float x = ((vid_mode->width)/2.0f)-(width/2.0f);
         
            FontPrintString( font, curName, x, y, w, h );
            
        glEnd();
                 
        glKosFinishFrame();
    
        for(o=0;o<fs_entries; o++)
        { 
           varr[o][0]-=(xstep/frames);
           
           if(o==fs_entry)
           {
               varr[o-1][2]-=zstep/frames;
               varr[o][2]+=zstep/frames;        
           } 
              
        }
    }
}

void DCMC_ShiftLayerBackward( vector3f *varr, DirectoryEntry * fs_dir, 
                             int fs_entries, int fs_entry,
                             int frames, float xstep, float zstep,
                             Font *font, float size, GlTexture * bgnd,
                             char *curName )
{
    int i,o;
    float w=size, h=size,y=436.f;
    
    for(i=0;i<frames;i++)
    {

        glKosBeginFrame();
        
        DCMC_RenderGlTexture( bgnd, 0,60,640,360 ); 
         
        for(o=0;o<fs_entries; o++)
            DCMC_RenderGlTexture3DPoint( DCMC_TbnPtr(fs_dir+o),
                                   varr[o][0],varr[o][1],varr[o][2],
                                   vid_mode->width/3.f, vid_mode->height*(1/(16.f/9.f)) );

        glBindTexture( GL_TEXTURE_2D, font->TexId );  
    
        glBegin(GL_QUADS);
            
            float width = cstr_len(curName)*w;
        
            float x = ((vid_mode->width)/2.0f)-(width/2.0f);
         
            FontPrintString( font, curName, x, y, w, h );
            
        glEnd();
                 
        glKosFinishFrame();
    
        for(o=0;o<fs_entries; o++)
        { 
           varr[o][0]+=(xstep/frames);
           if(o==fs_entry)
           {

               varr[o][2]+=zstep/20.f;
               varr[o+1][2]-=zstep/20.f;
           } 
              
        }
    }
}

void DCMC_ParseDirectory( Font *font, GlTexture * bgnd )
{
    Input input;  
    DirectoryEntry * fs_dir = NULL; 
    unsigned int fs_entries, fs_entry = 0, frame = 0, i;
    float x = 0;
    float zb = -2.3f;   
    float zh = -1.9f;
    vector3f * varr;
           
    switch(FDIR) /* Load the directory listing, filtered by container */
    {
        case DIR_IMG: // Pictures
             fs_dir = FsLoadDirectoryFiltered( FILE_DIR[FDIR].String, &fs_entries,
                                               CONTAINER_IMAGE );
             break;
            
        case DIR_VID: // Videos
             fs_dir = FsLoadDirectoryFiltered( FILE_DIR[FDIR].String, &fs_entries,
                                               CONTAINER_VIDEO );   
             break;
             
        case DIR_AUD: // Music
             fs_dir = FsLoadDirectoryFiltered( FILE_DIR[FDIR].String, &fs_entries,
                                               CONTAINER_AUDIO );       
            break;
                       
        case DIR_BIN: // Programs
             fs_dir = FsLoadDirectoryFiltered( FILE_DIR[FDIR].String, &fs_entries,
                                               CONTAINER_BIN );   
            break;
    }
    
    if(fs_dir==NULL || fs_entries==0)
    {
         printf("Empty Directory List: %s\n", FILE_DIR[FDIR].String);
         StringSet( &FILE_DIR[FDIR], "/\0" );
         return DCMC_ParseDirectory(font,bgnd);
    }
    
    varr = malloc(sizeof(vector3f)*fs_entries);
    
    for(i=0;i<fs_entries;i++)
    {
        varr[i][0] = x;
        varr[i][1] = 0.f;
        varr[i][2] = zb;
        x+=0.775f;
    }
    varr[0][2] = zh;

    DCE_SetInput( &input, 0 );
    
    while(1)
    {    
        glKosBeginFrame(); /* Begin Frame Render */
         
        /* Render Background Texture */ 
        DCMC_RenderGlTexture( bgnd, 0,60,640,360 ); 
        
        /* Render Directory Entry Thumbnails */ 
        for(i=0;i<fs_entries; i++)
            DCMC_RenderGlTexture3DPoint( DCMC_TbnPtr(fs_dir+i),
                                   varr[i][0],varr[i][1],varr[i][2],
                                   vid_mode->width/3.f, vid_mode->height*(1/(16.f/9.f)) );

        /* Render the Selected Directory Entry Name */
        glBindTexture( GL_TEXTURE_2D, font->TexId );  
        glBegin(GL_QUADS);
            float width = cstr_len((fs_dir+fs_entry)->Name)*16.f;
            float x = ((vid_mode->width)/2.0f)-(width/2.0f);
            FontPrintString( font, (fs_dir+fs_entry)->Name, x, 436.f, 16.f, 16.f );
        glEnd();
        
        glKosFinishFrame(); /* End Frame Render */
    
        DCE_SetInput(&input,0); /* Handle User Input */
        DCE_GetInput(&input);
 
        if(input.r||input.d)
        {
            if(fs_entry<fs_entries-1)
            {
                fs_entry++;
                DCMC_ShiftLayerForward( varr, fs_dir, fs_entries, fs_entry,
                                        20, .775f, fabs(zb-zh),
                                        font, 16.f,
                                        bgnd, (fs_dir+fs_entry)->Name );
            }
        }
        if(input.l||input.u)
        {
            if(fs_entry>0)
            {
                fs_entry--;
                DCMC_ShiftLayerBackward( varr, fs_dir, fs_entries, fs_entry,
                                         20, .775f, fabs(zb-zh),
                                         font, 16.f,
                                         bgnd, (fs_dir+fs_entry)->Name );
            }
        }    
            if(input.a)
            {
                StringConcat( &FILE_DIR[FDIR], (fs_dir+fs_entry)->Name );
                
                if((fs_dir+fs_entry)->Container!=CONTAINER_NULL)
                {
                    DCMC_LoadFile( font, FILE_DIR[FDIR].String, (fs_dir+fs_entry) );
                    
                    int lpos = cstr_find_last( FILE_DIR[FDIR].String, '/' );
                    
                    StringSetLen(&FILE_DIR[FDIR], FILE_DIR[FDIR].String, lpos+1);
                }
                else
                {
                    StringConcat( &FILE_DIR[FDIR], "/\0" );
                    free(varr);
                    FsFreeDirectoryEntry(fs_dir,fs_entries);

                    return DCMC_LoadDirectory( font, bgnd);
                }     
            }
            if(input.b)
            {
                if(FILE_DIR[FDIR].Size <= 6 )// Absolute path size <=6 Indicates a Root directory //
                {
                   StringSet( &FILE_DIR[FDIR], "/\0" );
                }
                else
                {
                   int lpos = cstr_find_2nd_to_last( FILE_DIR[FDIR].String, '/' );
                   StringSetLen(&FILE_DIR[FDIR], FILE_DIR[FDIR].String, lpos+1);
                }
                free(varr); 
                FsFreeDirectoryEntry(fs_dir,fs_entries);
                
                switch(FDIR) /* Render the background to add a delay */
                {
                    case DIR_IMG: // Pictures
                         DCMC_DrawPictureBgnd();
                         break;
                    case DIR_VID: // Pictures
                         DCMC_DrawVideoBgnd();
                         break;
                    case DIR_AUD: // Pictures
                         DCMC_DrawMusicBgnd();
                         break;
                    case DIR_BIN: // Pictures
                         DCMC_DrawAppBgnd();
                         break;
                }
                
                return DCMC_ParseDirectory( font, bgnd);
            }
          
            if(input.st)
                break;
    }    
    
    free(varr);
    FsFreeDirectoryEntry(fs_dir,fs_entries);
}
