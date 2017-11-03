/*
uCam.h
*/

#ifndef UCAMII_h
#define UCAMII_h
#define UCAMII_BUF_SIZE 64
#include <Arduino.h>
#include <UARTClass.h>

class uCam {

public:
	enum color_type {
		COLOR_TYPE_8BIT_GRAY = 0x03,
		COLOR_TYPE_VYUY = 0x08,
		COLOR_TYPE_RGB565 = 0x06,
		COLOR_TYPE_JPEG = 0x07,
	};
	enum raw_resolution {
		RAW_RESOLUTION_80X60 = 0x1,
		RAW_RESOLUTION_160X120 = 0x3,
		RAW_RESOLUTION_128X128 = 0x9,
		RAW_RESOLUTION_128X96 = 0xb,
	};
	enum jpeg_resolution {
		JPEG_160X128 = 0x3,
		JPEG_320X240 = 0x5,
		JPEG_640x480 = 0x7
	};
	enum function_mode {
		FUNCTION_MODE_NONE,
		FUNCTION_MODE_PREVIEW,
		FUNCTION_MODE_CAPTURE
	};

	uCam();
	boolean init();
	boolean init(
			// preview mode
			enum color_type preview_cy, enum raw_resolution preview_rr, enum jpeg_resolution preview_jr,
			// capture mode
			enum color_type capture_cy, enum raw_resolution capture_rr, enum jpeg_resolution capture_jr
		);
	boolean takePicture(enum function_mode mode);

	void setDebug(UARTClass *interface);
	int numberOfPackages();
	UARTClass *debugSerial;
	unsigned long imageSize;

	int getData();
	byte imgBuffer[UCAMII_BUF_SIZE];    // this is also set in _PACK_SIZE
	UARTClass *camSerial;
private:
	bool bCurrentMode;
	unsigned long image_pos;
	int package_no;
	int send_initial(enum function_mode mode);
	int set_package_size();
	int do_snapshot();
	int get_picture();
	int attempt_sync();
	int attemp_reset();
	boolean wait_for_bytes(byte command[6]);
	void send_generic_ack();
};

#endif
