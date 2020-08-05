#ifndef DC_TIMER_H
#define DC_TIMER_H

/* Timer for frame-rate management ********************************************/
#include <time.h>
#include <arch/timer.h>
	
/* Get current hardware timing using arch/timer.h */
unsigned int GetTime();

#endif
