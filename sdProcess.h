// sdProcess.h

#ifndef _SDPROCESS_h
#define _SDPROCESS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

void sd_setup();
void sd_loop();

#endif

