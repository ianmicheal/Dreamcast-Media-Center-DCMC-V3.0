/* Container Formats */

#include "cstring.h"
#include "container.h"
#include "fs.h"

unsigned char FsMediaContainer( char * fname )
{
    unsigned int i = cstr_len(fname);
    
    if((fname[i-4]=='a' || fname[i-4]=='A') &&
       (fname[i-3]=='v' || fname[i-3]=='V') &&
       (fname[i-2]=='i' || fname[i-2]=='I') )
       return CONTAINER_AVI;

    if((fname[i-5]=='d' || fname[i-5]=='D') &&
       (fname[i-4]=='i' || fname[i-4]=='I') &&
       (fname[i-3]=='v' || fname[i-3]=='V') &&
       (fname[i-2]=='x' || fname[i-2]=='X') )
       return CONTAINER_AVI;

    if((fname[i-4]=='m' || fname[i-4]=='M') &&
       (fname[i-3]=='p' || fname[i-3]=='P') &&
       (fname[i-2]=='g' || fname[i-2]=='G') )
       return CONTAINER_MPG;

    if((fname[i-4]=='m' || fname[i-4]=='M') &&
       (fname[i-3]=='3' || fname[i-3]=='3') &&
       (fname[i-2]=='u' || fname[i-2]=='U') )
       return CONTAINER_M3U;

    if((fname[i-4]=='s' || fname[i-4]=='S') &&
       (fname[i-3]=='f' || fname[i-3]=='F') &&
       (fname[i-2]=='d' || fname[i-2]=='D') )
       return CONTAINER_SFD;

    if((fname[i-5]=='m' || fname[i-5]=='M') &&
       (fname[i-4]=='p' || fname[i-4]=='P') &&
       (fname[i-3]=='e' || fname[i-3]=='E') &&
       (fname[i-2]=='g' || fname[i-2]=='G') )
       return CONTAINER_MPG;

    if((fname[i-4]=='m' || fname[i-4]=='M') &&
       (fname[i-3]=='p' || fname[i-3]=='P') &&
       (fname[i-2]=='3' || fname[i-2]=='3') )
       return CONTAINER_MP3;
       
    if((fname[i-4]=='a' || fname[i-4]=='A') &&
       (fname[i-3]=='c' || fname[i-3]=='C') &&
       (fname[i-2]=='3' || fname[i-2]=='3') )
       return CONTAINER_AC3;

    if((fname[i-4]=='w' || fname[i-4]=='W') &&
       (fname[i-3]=='a' || fname[i-3]=='A') &&
       (fname[i-2]=='v' || fname[i-2]=='V') )
       return CONTAINER_WAV;

    if((fname[i-4]=='a' || fname[i-4]=='A') &&
       (fname[i-3]=='a' || fname[i-3]=='A') &&
       (fname[i-2]=='c' || fname[i-2]=='C') )
       return CONTAINER_AAC;

    if((fname[i-4]=='m' || fname[i-4]=='M') &&
       (fname[i-3]=='p' || fname[i-3]=='P') &&
       (fname[i-2]=='4' || fname[i-2]=='4') )
       return CONTAINER_MP4;

    if((fname[i-4]=='m' || fname[i-4]=='M') &&
       (fname[i-3]=='4' || fname[i-3]=='4') &&
       (fname[i-2]=='a' || fname[i-2]=='A') )
       return CONTAINER_M4A;

    if((fname[i-4]=='a' || fname[i-4]=='A') &&
       (fname[i-3]=='d' || fname[i-3]=='D') &&
       (fname[i-2]=='x' || fname[i-2]=='X') )
       return CONTAINER_ADX;

    if((fname[i-5]=='f' || fname[i-5]=='F') &&
       (fname[i-4]=='l' || fname[i-4]=='L') &&
       (fname[i-3]=='a' || fname[i-3]=='A') &&
       (fname[i-2]=='c' || fname[i-2]=='C') )
       return CONTAINER_FLAC;

    if((fname[i-4]=='b' || fname[i-4]=='B') &&
       (fname[i-3]=='i' || fname[i-3]=='I') &&
       (fname[i-2]=='n' || fname[i-2]=='N') )
       return CONTAINER_BIN;    

    if((fname[i-4]=='p' || fname[i-4]=='P') &&
       (fname[i-3]=='v' || fname[i-3]=='V') &&
       (fname[i-2]=='r' || fname[i-2]=='R') )
       return CONTAINER_PVR;  

    if((fname[i-4]=='p' || fname[i-4]=='P') &&
       (fname[i-3]=='n' || fname[i-3]=='N') &&
       (fname[i-2]=='g' || fname[i-2]=='G') )
       return CONTAINER_PNG;  

    if((fname[i-4]=='j' || fname[i-4]=='J') &&
       (fname[i-3]=='p' || fname[i-3]=='P') &&
       (fname[i-2]=='g' || fname[i-2]=='G') )
       return CONTAINER_JPG;  

    if(FsIsFile( fname ))
        return CONTAINER_UNKNOWN;
    
    return CONTAINER_NULL;
}
