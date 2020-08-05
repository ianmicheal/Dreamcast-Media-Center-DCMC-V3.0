/*
** AICA_CMD (C) Josh PH3NOM Pearson 2013

** Thanks to yamato@20to4.net for the AICA Hardware Reference v0.8
**
*/

#ifndef AICA_CMD_H
#define AICA_CMD_H

#define AICA_STEREO         0x00
#define AICA_MONO           0x80
#define AICA_MAX_VOLUME     0xF

/* Adjust the AICA Master Volume - This effects all streaming channels */

void AICA_VolumeIncrease( unsigned char ch );

void AICA_VolumeDecrease( unsigned char ch );

/* Adjust the AICA Clock Speed - This effects nothing? */

unsigned short AICA_ARM_CLOCK_GET(); // Get the clock speed

void AICA_ARM_CLOCK_SET(unsigned short m); // Set the clock speed

#endif
