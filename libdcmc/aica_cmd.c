/*
** AICA_CMD (C) Josh PH3NOM Pearson 2013
**
** Thanks to yamato@20to4.net for the AICA Hardware Reference v0.8
**
*/

#include "aica_cmd.h"

volatile unsigned long *AICA_BASE_SH4       = (unsigned long*) 0xa0700000;
volatile unsigned long *AICA_MASTER_VOLUME  = (unsigned long*) 0xa0702800;
volatile unsigned long *AICA_ARM_CLOCK      = (unsigned long*) 0xa07028a8;

static   unsigned char  AICA_VOL = AICA_MAX_VOLUME;

unsigned short AICA_ARM_CLOCK_GET()
{
    return *AICA_ARM_CLOCK+1;
}

// NoOp? Lol...
void AICA_ARM_CLOCK_SET(unsigned short m)
{
    if(m<=44)
    {
       unsigned int mHz = 0x00000000;
       mHz |= (m+1);
       *AICA_ARM_CLOCK = mHz;
    }
}

void AICA_VolumeIncrease( unsigned char ch )
{
    if(AICA_VOL < AICA_MAX_VOLUME)
    {
        unsigned int CMD_VOL = 0x00000000;
        ++AICA_VOL;
        CMD_VOL |= AICA_VOL;
        CMD_VOL |= (ch<<8);
        *AICA_MASTER_VOLUME = CMD_VOL;
    }
}

void AICA_VolumeDecrease( unsigned char ch )
{
    if(AICA_VOL > 0)
    {
        unsigned int CMD_VOL = 0x00000000;
        --AICA_VOL;
        CMD_VOL |= AICA_VOL;
        CMD_VOL |= (ch<<8);
        *AICA_MASTER_VOLUME = CMD_VOL;
    }
}
