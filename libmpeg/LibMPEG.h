#ifndef LibMPEG_H
#define LibMPEG_H

typedef struct
{
    unsigned short int Width,Height;
    float FrameRate;
    unsigned char DAR;
} MpegSequenceHeader;

typedef unsigned int MpgSeqHdr;

MpegSequenceHeader * LibMPEG_ParseSequenceHeader( FILE * ins );

#endif
