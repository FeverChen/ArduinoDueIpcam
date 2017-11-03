// 
// 
// 

#include "cameraProcess.h"
#include "uCam.h"
#include "ethernetProcess.h"

#define DEBUG 0
#define	SERIAL1_SPEED	115200

uCam camera;
static bool bPreviewRunning = false;
static bool bCaptureRunning = false;
static bool bTakePictureStart = false;
static cameraCallback cameraCb = NULL;
static void *cameraCtx = NULL;
static int packageno = 0;
static int totalBytes = 0;

void sendCommand(byte *cmd, size_t size)
{
	Serial1.write(cmd, size);
	while (!Serial1.available());
	while (Serial1.available())
	{
		Serial.print("0x");
		Serial.print(Serial1.read(), HEX);
		Serial.print(", ");
		delay(100);
	}
	Serial.println();
}

void camera_startPreview()
{
	packageno = 0;
	totalBytes = 0;
	bPreviewRunning = true;
	bTakePictureStart = false;
}

void camera_stopPreview()
{
	#if DEBUG
	Serial.print("Image size: ");
	Serial.println(camera.imageSize, DEC);
	Serial.print("number of packages: ");
	Serial.println(camera.numberOfPackages(), DEC);
	#endif
	bPreviewRunning = false;
	// flush
	while(camera.getData() > 0)
	{
		delay(10);
	}
}

void camera_captrue()
{
	packageno = 0;
	totalBytes = 0;
	bCaptureRunning = true;
	bTakePictureStart = false;
}

void defaultCallback(void *ctx, int dataLen, byte *imgBuff, int imageSize, int sentByte, int nPackages, int noPackage)
{
	if (cameraCb)
	{
		cameraCb(cameraCtx, dataLen, imgBuff, imageSize, sentByte, nPackages, noPackage);
	}
	#if DEBUG
	Serial.print("\rReceive bytes ");
	Serial.print(sentByte);
	#endif
}

void setCameraCallback(void *ctx, cameraCallback cb)
{
	cameraCtx = ctx;
	cameraCb = cb;
}

void camera_setup()
{
	Serial1.begin(SERIAL1_SPEED);
	while (!Serial1);

	while (!camera.init(
	uCam::COLOR_TYPE_JPEG, uCam::RAW_RESOLUTION_160X120, uCam::JPEG_320X240,
	uCam::COLOR_TYPE_JPEG, uCam::RAW_RESOLUTION_160X120, uCam::JPEG_640x480));

	Serial.println("Init camera OK!");
}

void camera_loop()
{
	if (bPreviewRunning || bCaptureRunning)
	{
		int bytes = 0;
		
		if (bTakePictureStart && (bytes = camera.getData()) > 0) 
		{
			//Serial.println("retrieving camera data...");
			defaultCallback(cameraCtx, bytes, camera.imgBuffer, camera.imageSize, totalBytes, camera.numberOfPackages(), packageno++);
			totalBytes += bytes;
		}
		else if (bTakePictureStart && bytes == 0)
		{
			bCaptureRunning = false;
			bTakePictureStart = false;
			//Serial.println("retrieving camera data done!");
		}
		else
		{
			totalBytes = 0;
			packageno = 0;
			bTakePictureStart = true;
			camera.takePicture(uCam::FUNCTION_MODE_PREVIEW);
			//Serial.println("Starting retrieving camera data...");
		}
	}
}
