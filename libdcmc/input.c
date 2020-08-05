/*
** DC-Engine-3D (C) 2012 Josh "PH3NOM" Pearson 
**
** Controller Input Functions for the Engine
**
*/

#include "input.h"

void DCE_SetInput( Input *ins, short val )
{
    ins->u = ins->d = ins->l = ins->r = 
    ins->a = ins->b = ins->x = ins->y = 
    ins->st = ins->lt = ins->rt = 0;
}

void DCE_GetInput( Input *ins )
{
     maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);    

     if (cont)
     {  
         cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
         
         if( state->buttons & CONT_DPAD_UP )
             ins->u = 1;
         if( state->buttons & CONT_DPAD_DOWN )
             ins->d = 1;
         if( state->buttons & CONT_DPAD_LEFT )
             ins->l = 1;         
         if( state->buttons & CONT_DPAD_RIGHT )
             ins->r = 1;
         if (state->buttons & CONT_Y)
             ins->y = 1;
         if (state->buttons & CONT_X) 
             ins->x = 1;
         if (state->buttons & CONT_A)
             ins->a = 1;
         if (state->buttons & CONT_B)                                   
             ins->b = 1;
         if (state->buttons & CONT_START)                                   
             ins->st = 1;
             
         ins->rt = state->rtrig;
         ins->lt = state->ltrig;
     } 
}

/* Handle User Input Requests during video playback ***************************/

#include "aica_cmd.h"
#include "render.h"

static Input input;

inline unsigned char HandleInput( unsigned char AICA_CH )
{
    DCE_SetInput( &input, 0 );
    DCE_GetInput( &input );
    
    /* Set Display Aspect Ratio */
    if(input.u) SetDisplay_4_3();
    if(input.l) SetDisplay_16_9();
    if(input.d) SetDisplay_235_1();
    if(input.r) SetDisplayDefault();
    if(input.y)
    {
        if(RenderInfo())
            RenderHideInfo();
        else
            RenderShowInfo();
    }
    if(input.x)
        RenderDelayIncrease();
    if(input.b)
        RenderDelayDecrease();
                        
    /* Adjust the Volume */
    if(input.rt>0.0f)
       AICA_VolumeIncrease( AICA_CH );

    if(input.lt>0.0f)    
      AICA_VolumeDecrease( AICA_CH );

    return input.st;
}
