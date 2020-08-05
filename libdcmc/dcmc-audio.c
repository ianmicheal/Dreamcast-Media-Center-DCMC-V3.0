#include "dcmc.h"
#include "aica_cmd.h"
#include "LibA52.h"
#include "LibADX.h"
#include "LibMPG123.h"

void DCMC_LibMPG123_Decode( char * fname, Font * font, DirectoryEntry * dir_entry )
{
    DCMC_DrawMusicBgnd(); // Loading Screen with 1000ms pause
    
    #define CHUNK_SIZE 2048
    uint8 strbuf[CHUNK_SIZE];
    uint32 flen,fdone=0;
    FILE *f = NULL;
    
    f = fopen( fname, "rb" );
    fseek( f, 0, SEEK_END );
    flen = ftell(f);
    fseek( f, 0, SEEK_SET );    
    
    LibMPG123_Init();
    
    Input cont;
    DCE_SetInput( &cont, 0 );
    
    while(fdone<(flen-CHUNK_SIZE) && !cont.st)
    {
        /* Read BitStream into buffer */
        fread( strbuf, sizeof(uint8), CHUNK_SIZE, f );
        
        /* Wait for LibMPG123 to have less than a minmal amount of PCM bytes */
        while(LibMPG123_PcmHave() >= 65535*2) thd_pass();
            
        /* Decode the BitStream buffer */    
        LibMPG123_DecodeChunk( strbuf, CHUNK_SIZE );
        
        fdone += CHUNK_SIZE; /* Increment position in file */
        
        glKosBeginFrame(); /* Now, Lets render some stuff */
        
        DCMC_RenderGlTexture( DCMC_TexID(BgndMusic), 0,60,640,360 );
        
        DCMC_RenderTbn( dir_entry, 220,80,200,320);

        float width = cstr_len(dir_entry->Name)*16.0f;
        
        float x = ((vid_mode->width)/2.0f)-(width/2.0f);

        glBindTexture( GL_TEXTURE_2D, font->TexId );  
        glBegin(GL_QUADS);
            FontPrintString( font, dir_entry->Name, x, 436.0f, 16.0f, 16.0f );
        glEnd();
        
        glKosFinishFrame();
        
        /* Now, Handle User Input */
        DCE_SetInput( &cont, 0 );
        DCE_GetInput( &cont );
        
        if(cont.rt>0)
            AICA_VolumeIncrease( LibMPG123_Chan()==2 ? AICA_STEREO : AICA_MONO );
        if(cont.lt>0)
            AICA_VolumeDecrease( LibMPG123_Chan()==2 ? AICA_STEREO : AICA_MONO );
        
    }

    /* If user did not command an exit, wait for remaining PCM samples to stream */
    while(LibMPG123_PcmHave() >= 65535/2 && !cont.st ) thd_pass();
    
    LibMPG123_Exit();

    fclose(f);
}


void DCMC_LibA52_Decode( char * fname, Font * font, DirectoryEntry * dir_entry )
{
    DCMC_DrawMusicBgnd(); // Loading Screen with 1000ms pause
    
    #define CHUNK_SIZE 2048
    uint8 strbuf[CHUNK_SIZE];
    uint32 flen,fdone=0;
    FILE *f = NULL;
    
    f = fopen( fname, "rb" );
    fseek( f, 0, SEEK_END );
    flen = ftell(f);
    fseek( f, 0, SEEK_SET );    
    
    LibA52_Init( 44100, 2 ); // WARNING!!!!! Hardcoded 44.1kHz Stereo
    
    Input cont;
    DCE_SetInput( &cont, 0 );
 
    while(fdone<(flen-CHUNK_SIZE) && !cont.st)
    {
        /* Read BitStream into buffer */
        fread( strbuf, sizeof(uint8), CHUNK_SIZE, f );
        
        /* Wait for LibA52 to have less than a minmal amount of PCM bytes */
        while(LibA52_PcmHave() >= 65535*2) thd_pass();
            
        /* Decode the BitStream buffer */    
        LibA52_decode_chunk( strbuf, strbuf+CHUNK_SIZE );
        
        fdone += CHUNK_SIZE; /* Increment position in file */
        
        glKosBeginFrame(); /* Now, Lets render some stuff */
        
        DCMC_RenderGlTexture( DCMC_TexID(BgndMusic), 0,60,640,360 );
        
        DCMC_RenderTbn( dir_entry, 220,80,200,320);

        float width = cstr_len(dir_entry->Name)*16.0f;
        
        float x = ((vid_mode->width)/2.0f)-(width/2.0f);

        glBindTexture( GL_TEXTURE_2D, font->TexId );      
        glBegin(GL_QUADS);
            FontPrintString( font, dir_entry->Name, x, 436.0f, 16.0f, 16.0f );
        glEnd();
        
        glKosFinishFrame();

        /* Now, Handle User Input */
        DCE_SetInput( &cont, 0 );
        DCE_GetInput( &cont );
        
        if(cont.rt>0)
            AICA_VolumeIncrease(AICA_STEREO);
        if(cont.lt>0)
            AICA_VolumeDecrease(AICA_STEREO);
        
    }    
    /* If user did not command an exit, wait for remaining PCM samples to stream */
    while(LibA52_PcmHave() >= 65535/2 && !cont.st ) thd_pass();
    
    LibA52_Exit();

    fclose(f);
}

void DCMC_LibADX_Decode( char * fname, Font * font, DirectoryEntry * dir_entry )
{
    DCMC_DrawMusicBgnd(); // Loading Screen with 1000ms pause
    
    #define CHUNK_SIZE ADX_CHUNK_SIZE*4
    uint8 strbuf[CHUNK_SIZE];
    uint32 flen,fdone=0;
    FILE *f = NULL;
    Input cont;
        
    f = fopen( fname, "rb" );
    fseek( f, 0, SEEK_END );
    flen = ftell(f);
    fseek( f, 0, SEEK_SET );    

    DCE_SetInput( &cont, 0 );
    
    fread( strbuf, sizeof(uint8), CHUNK_SIZE, f );
    
    LibADX_Init( strbuf, CHUNK_SIZE );
    
    while(fdone<(flen-CHUNK_SIZE) && !cont.st)
    {        
        /* Decode ADX Chunk if below a threshold if 65535*2 */
        if( LibADX_PcmHave() <= 65535*2 )   
        {
            /* Read BitStream into buffer */
            fread( strbuf, sizeof(uint8), CHUNK_SIZE, f );
            LibADX_DecodeChunk( strbuf, CHUNK_SIZE );
            fdone += CHUNK_SIZE; /* Increment position in file */
        }
        
        glKosBeginFrame(); /* Now, Lets render some stuff */
        
        DCMC_RenderGlTexture( DCMC_TexID(BgndMusic), 0,60,640,360 );
        
        DCMC_RenderTbn( dir_entry, 220,80,200,320);

        float width = cstr_len(dir_entry->Name)*16.0f;
        
        float x = ((vid_mode->width)/2.0f)-(width/2.0f);

        glBindTexture( GL_TEXTURE_2D, font->TexId );      
        glBegin(GL_QUADS);
            FontPrintString( font, dir_entry->Name, x, 436.0f, 16.0f, 16.0f );
        glEnd();
        
        glKosFinishFrame();

        /* Now, Handle User Input */
        DCE_SetInput( &cont, 0 );
        DCE_GetInput( &cont );
        
        if(cont.rt>0)
            AICA_VolumeIncrease(LibADX_Channels() == 2 ? AICA_STEREO : AICA_MONO );
        if(cont.lt>0)
            AICA_VolumeDecrease(LibADX_Channels() == 2 ? AICA_STEREO : AICA_MONO );
        
    }    
    
    /* If user did not command an exit, wait for remaining PCM samples to stream */
    while(LibADX_PcmHave() >= 65535/2 && !cont.st ) thd_pass();
    
    LibADX_Exit();

    fclose(f);
}
