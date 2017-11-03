#include "uCam.h"
#include "ethernetProcess.h"
#include "cameraProcess.h"
#include "sdProcess.h"
#include "keyProcess.h"

static void cameraCb(void *ctx, int dataLen, byte *imgBuff, int imageSize, int sentBytes, int nPackages, int noPackage)
{
	if (bWebClientConnected)
	{
		if (noPackage == 0)
		{
			client.print("\n--"+ jpegBoundary +"\n") ;
			Serial.print("\r\n--"+ jpegBoundary +"\r\n") ;
			client.println("Content-Type: image/jpeg\n\n");
			Serial.println("Content-Type: image/jpeg\r\n\r\n");
		}
        for (int i=0;i<dataLen;i++)
		{
			client.print((char) imgBuff[i]);
		}
		client.flush();
		Serial.print("\rReceive bytes ");
		Serial.print(sentBytes);
	}
}

void setup() {
	// put your setup code here, to run once:
	// 1. init ethernet ip, gateway, subnet mask...
	// 2. init SD card, detect SD card existing
	// 3. init camera
	key_setup();
	sd_setup();
	camera_setup();
	ethernet_setup();
	setCameraCallback(NULL, cameraCb);
}

void loop() {
	// put your main code here, to run repeatedly:
	key_loop();
	sd_loop();
	camera_loop ();
	ethernet_loop();
}
