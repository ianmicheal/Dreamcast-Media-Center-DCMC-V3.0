/*
** Font.H (C) Josh PH3NOM Pearson 2013
*/

#ifndef FONT_H
#define FONT_H

#define ASCI_SYMBOL_OFT 32
#define ASCI_TOTAL_CHAR 128-ASCI_SYMBOL_OFT
#define INVALID_UV -1.0f

typedef struct
{
    float TexW, TexH;
    float CharW, CharH;
    unsigned char RowStride, ColStride;
    unsigned int TexFmt;
    unsigned int Color;
    unsigned int TexId;
    float  TexUV[ASCI_TOTAL_CHAR][4];
    void*  TexAddr;
} Font;

Font * FontInit( void* TexAddr, uint32 TexFmt,
                 float TexW, float TexH,
                 unsigned char RowStride, unsigned char ColStride,
                 uint32 Color );

void FontPrintString( Font * font, char * str, float xpos, float ypos,
                      float width, float height );

void FontSetColor( Font * font, uint32 color );

#endif
