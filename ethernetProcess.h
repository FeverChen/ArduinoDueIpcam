// ethernetProcess.h

#ifndef _ETHERNETPROCESS_h
#define _ETHERNETPROCESS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <Ethernet2.h>
extern bool bWebClientConnected;
extern EthernetClient client;
extern String jpegBoundary;

void ethernet_setup();
void ethernet_loop();
bool ethernet_clientConnected();

#endif

