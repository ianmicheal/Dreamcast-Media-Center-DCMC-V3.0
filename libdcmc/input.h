/*
**
** This File is a part of Dreamcast Media Center
** (C) Josh "PH3NOM" Pearson 2011
**
*/

#ifndef DCE_INPUT
#define DCE_INPUT

#include <dc/maple.h>
#include <dc/maple/controller.h>

typedef struct
{
    short a,b,x,y, st,
          u,d,l,r;
    float lt, rt;
} Input;

void DCE_SetInput( Input *ins, short val );

void DCE_GetInput( Input *ins );

#endif
