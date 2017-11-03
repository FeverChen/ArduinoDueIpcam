// keyProcess.h

#ifndef _KEYCOMMAND_h
#define _KEYCOMMAND_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define	SERIAL_SPEED	115200

void key_setup();
void key_loop();
void key_Handler(int key);
void key_showHelp();

#endif

