// cameraProcess.h

#ifndef _CAMERAPROCESS_h
#define _CAMERAPROCESS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

typedef void (*cameraCallback)(void *ctx, int dataLen, byte *imgBuff, int imageSize, int sentBytes, int nPackages, int noPackage);

void camera_setup();
void camera_loop();
void camera_startPreview();
void camera_stopPreview();
void camera_captrue();
void setCameraCallback(void *ctx, cameraCallback cb);

#endif

