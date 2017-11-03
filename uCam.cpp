/*
*    uCam.cpp
*
*    http://www.4dsystems.com.au/productpages/uCAM-II/downloads/uCAM-II_datasheet_R_1_4.pdf
*/

#include "uCam.h"
//#define cameraDebugSerial

// From http://pastebin.com/ZuMu48w7

byte _SYNC_COMMAND[6] = { 0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00 };
byte _SYNC_ACK_REPLY[6] = { 0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00 };
byte _SYNC_ACK_REPLY_EXT[6] = { 0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00 };
byte _FINAL_ACK[6] = { 0xAA, 0x0E, 0xFF, 0x00, 0x00, 0x00 };

byte _PREVIEW_INITIAL_COMMAND[6] = { 0xAA, 0x01, 0x00, 0x0, 0x0, 0x0};
byte _CAPTURE_INITIAL_COMMAND[6] = { 0xAA, 0x01, 0x00, 0x0, 0x0, 0x0};
byte _GENERIC_ACK_REPLY[6] = { 0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00 };

byte _PACK_SIZE[6] = { 0xAA, 0x06, 0x08, (UCAMII_BUF_SIZE + 6)&0xff, ((UCAMII_BUF_SIZE+6)&0xff00)>>8, 0x00 };
byte _SNAPSHOT[6] = { 0xAA, 0x05, 0x00, 0x00, 0x00, 0x00 };
byte _GET_PICTURE[6] = { 0xAA, 0x04, 0x02, 0x00, 0x00, 0x00 };

byte _RESET_COMMAND[6] = { 0xAA, 0x08, 0x00, 0x00, 0x00, 0xFF };
byte _RESET_ACK_REPLY[6] = { 0xAA, 0x0E, 0x08, 0x00, 0x00, 0x00 };

uCam::uCam() {
	this->image_pos = 0;
	this->package_no = 0;
	debugSerial = &Serial;
	camSerial = &Serial1;
}

void uCam::setDebug(UARTClass *interface) {
	this->debugSerial = interface;
}

boolean uCam::init()
{
	init(COLOR_TYPE_JPEG, RAW_RESOLUTION_128X128, JPEG_320X240, 
		COLOR_TYPE_JPEG, RAW_RESOLUTION_128X128, JPEG_640x480);
}

boolean uCam::init(
	enum color_type capture_cy, enum raw_resolution capture_rr, enum jpeg_resolution capture_jr,
	enum color_type preview_cy, enum raw_resolution preview_rr, enum jpeg_resolution preview_jr)
{
	bCurrentMode = FUNCTION_MODE_NONE;

	_CAPTURE_INITIAL_COMMAND[3] = capture_cy;
	_CAPTURE_INITIAL_COMMAND[4] = capture_rr;
	_CAPTURE_INITIAL_COMMAND[5] = capture_jr;
	
	_PREVIEW_INITIAL_COMMAND[3] = preview_cy;
	_PREVIEW_INITIAL_COMMAND[4] = preview_rr;
	_PREVIEW_INITIAL_COMMAND[5] = preview_jr;
	
	return attempt_sync();
}

int uCam::attemp_reset()
{
	while (camSerial->available());
#ifdef cameraDebugSerial
	this->debugSerial->println("Sending RESET...");
#endif
	for (int i = 0; i < 6; i++) {
		camSerial->write(_RESET_COMMAND[i]);
	}
	if (this->wait_for_bytes(_RESET_ACK_REPLY)) {
		return 1;
	}
	return 0;
}

void uCam::send_generic_ack()
{
#ifdef cameraDebugSerial
	this->debugSerial->println();
	this->debugSerial->print("Sending Final ACK command ");
	for (int i = 0; i < 6; i++) {
		this->debugSerial->print("0x");
		this->debugSerial->print(_FINAL_ACK[i], HEX);
		this->debugSerial->print(" ");
	}
	this->debugSerial->println();
#endif
	for (int i = 0; i < 6; i++) {
		camSerial->write(_FINAL_ACK[i]);
	}
}

int uCam::attempt_sync() {
	int attempts = 0;
	byte cam_reply;
	int ack_success = 0;
	int last_reply = 0;

	delay(800);

	while (attempts < 60 && ack_success == 0) {
		// Flush
		while (camSerial->available())
			cam_reply = camSerial->read();
		
		delay(10+attempts);

#ifdef cameraDebugSerial
		this->debugSerial->print("#");
		this->debugSerial->print(attempts);
		this->debugSerial->print(" Sending SYNC command ");
		for (int i = 0; i < 6; i++) {
			this->debugSerial->print("0x");
			this->debugSerial->print(_SYNC_COMMAND[i], HEX);
			this->debugSerial->print(" ");
		}
#endif
		for (int i = 0; i < 6; i++) {
			camSerial->write(_SYNC_COMMAND[i]);
		}

		if (this->wait_for_bytes(_SYNC_ACK_REPLY)) {
			if (this->wait_for_bytes(_SYNC_ACK_REPLY_EXT))
			{
				send_generic_ack();
				return 1;
			}
		}
		attempts++;
	}
	return 0;
}


// Return number of packages ready
int uCam::numberOfPackages() {
	return this->imageSize / UCAMII_BUF_SIZE;
}

int uCam::send_initial(enum function_mode mode= FUNCTION_MODE_NONE) {

	if (mode == FUNCTION_MODE_NONE)
	{
		return 1;
	}
	// flush
	while (camSerial->available() > 0) {
		camSerial->read();
	}

	delay(100);

#ifdef cameraDebugSerial
	this->debugSerial->println("Sending INITIALISE...");
#endif
	if (mode == FUNCTION_MODE_PREVIEW)
	{
		for (int i = 0; i < 6; i++) {
			camSerial->write(_PREVIEW_INITIAL_COMMAND[i]);
		}
	}
	else if (mode == FUNCTION_MODE_CAPTURE)
	{
		for (int i = 0; i < 6; i++) {
			camSerial->write(_CAPTURE_INITIAL_COMMAND[i]);
		}
	}
	// @todo why 500 delay?
	delay(500);
	if (this->wait_for_bytes(_GENERIC_ACK_REPLY)) {
#ifdef cameraDebugSerial
		this->debugSerial->println("INITIALISE success");
#endif
		return 1;
	}

#ifdef cameraDebugSerial
	this->debugSerial->println("INITIALISE fail");
#endif

	return 0;
}

boolean uCam::wait_for_bytes(byte command[6]) {
	byte cam_reply;
	int i = 0;
	int received;
	short found_bytes;
	found_bytes = 0;
	// @todo millis() wait, millis() wrap around watch out
	// unsigned long start = millis();

#ifdef cameraDebugSerial
	this->debugSerial->print("\r\nWAIT: ");
	for (i = 0; i < 6; i++) {
		this->debugSerial->print("0x");
		this->debugSerial->print(command[i], HEX);
		this->debugSerial->print(" ");
	}
	this->debugSerial->print("\r\nGOT : ");
	i = 0;
#endif
	//delay(1);

	while (camSerial->available()) {
		cam_reply = camSerial->read();
		if (i < 6) {
			if ((cam_reply == command[i]) || command[i] == 0x00) {
				found_bytes++;
				i++;
			}
		}

#ifdef cameraDebugSerial
		this->debugSerial->print("0x");
		this->debugSerial->print(cam_reply, HEX);
		this->debugSerial->print(" ");
#endif
		received++;
		if (found_bytes == 6) {
			return true;
		}
	}
#ifdef cameraDebugSerial
	this->debugSerial->println();
#endif
	return false;
}



bool uCam::takePicture(enum function_mode mode) {
	
	if (bCurrentMode == mode || 
		(send_initial(mode) &&
		set_package_size() && 
		do_snapshot()))
	{
		bCurrentMode = mode;
		return get_picture();
	}
}


int uCam::set_package_size() {

	byte ack[6] = { 0xAA, 0x0E, 0x06, 0x00, 0x00, 0x00 };

	delay(100);

#ifdef cameraDebugSerial
	this->debugSerial->println("Sending packet size...");
#endif

	for (int i = 0; i < 6; i++) {
		camSerial->write(_PACK_SIZE[i]);
#ifdef cameraDebugSerial
		this->debugSerial->print(_PACK_SIZE[i], HEX);
		this->debugSerial->print(" ");
#endif

	}
	// @todo why 500 delay?
	delay(500);
	if (this->wait_for_bytes(ack)) {
#ifdef cameraDebugSerial
		this->debugSerial->println("\r\npacket size success");
#endif
		return 1;
	}

#ifdef cameraDebugSerial
	this->debugSerial->println("packet size fail");
#endif

	return 0;
}

int uCam::do_snapshot() {

	byte ack[6] = { 0xAA, 0x0E, 0x05, 0x00, 0x00, 0x00 };

	delay(100);

#ifdef cameraDebugSerial
	this->debugSerial->println("Sending snapshot...");
#endif


	for (int i = 0; i < 6; i++) {
		camSerial->write(_SNAPSHOT[i]);
	}
	// @todo why 500 delay?
	delay(500);
	if (this->wait_for_bytes(ack)) {
#ifdef cameraDebugSerial
		this->debugSerial->println("snapshot success");
#endif
		return 1;
	}

#ifdef cameraDebugSerial
	this->debugSerial->println("snapshot fail");
#endif

	return 0;

}

int uCam::get_picture() {

	byte ack[6] = { 0xAA, 0x0E, 0x04, 0x00, 0x00, 0x00 };
	unsigned long imageSize;
	short i;

	package_no = 0;
	delay(100);

#ifdef cameraDebugSerial
	this->debugSerial->print("Sending get picture command ");
	for (int i = 0; i < 6; i++) {
		this->debugSerial->print("0x");
		this->debugSerial->print(_GET_PICTURE[i], HEX);
		this->debugSerial->print(" ");
	}
	this->debugSerial->println();
#endif

	for (int i = 0; i < 6; i++) {
		camSerial->write(_GET_PICTURE[i]);
	}
	// @todo why 500 delay?
	delay(500);
	if (this->wait_for_bytes(ack)) {
#ifdef cameraDebugSerial
		this->debugSerial->println("picture success");
#endif
		// get the 6 bytes ACK
		for (i = 0; i <= 5; i++) {
			ack[i] = 0;
			while (!camSerial->available());

			ack[i] = camSerial->read();
			// last 3 bytes are the image size
#ifdef cameraDebugSerial
			this->debugSerial->print(i, DEC);
			this->debugSerial->print(" value: ");
			this->debugSerial->println(ack[i], HEX);
#endif
		}

		imageSize = 0;
		imageSize = (imageSize << 8) | ack[5];
		imageSize = (imageSize << 8) | ack[4];
		imageSize = (imageSize << 8) | ack[3];

		this->imageSize = imageSize;
		this->image_pos = this->imageSize;
		if (imageSize > 0) {
			return 1;
		}
	}

#ifdef cameraDebugSerial
	this->debugSerial->println("picture fail");
#endif

	return 0;
}

// @todo return false if time exceeded
int uCam::getData() {

	unsigned char high = (unsigned char)(this->package_no >> 8);
	unsigned char low = this->package_no & 0xff;
	byte my_ack[6] = { 0xAA, 0x0E, 0x00, 0x00, low, high };

	int i = 0;
	byte s;
	int bytes;

	if (this->image_pos == 0) {
		return 0;
	}
	// request bytes
	for (int i = 0; i < 6; i++) {
		camSerial->write(my_ack[i]);
	}


	// Set number of bytes we should wait for
	if (this->image_pos < UCAMII_BUF_SIZE) {
		bytes = this->image_pos + 6;
	}
	else {
		bytes = UCAMII_BUF_SIZE + 6;
	}
#ifdef cameraDebugSerial
	this->debugSerial->print("REMAINING: ");
	this->debugSerial->print(this->image_pos, DEC);
	this->debugSerial->print(" BYTES PER CHUNK: ");
	this->debugSerial->println(bytes, DEC);
#endif

	for (i = 0; i < bytes; i++) {
		while (!camSerial->available()); // wait for bytes
		s = camSerial->read();
		// Skip first 4 and last 2, Page 10 of the datasheet
		if (i >= 4 && i < bytes - 2) {
#ifdef cameraDebugSerial
			this->debugSerial->print("*");
#endif
			this->imgBuffer[i - 4] = s;
			this->image_pos--;
		}
#ifdef cameraDebugSerial
		this->debugSerial->print(this->imgBuffer[s], HEX);
		this->debugSerial->print(" ");
#endif
	}


#ifdef cameraDebugSerial
	this->debugSerial->println("");
#endif

	this->package_no++;
	if (this->image_pos <= 0) {
		// send the final thank you goodbye package
		my_ack[4] = 0xF0;
		my_ack[5] = 0xF0;
		for (int i = 0; i < 6; i++) {
			camSerial->write(my_ack[i]);
		}
	}
	return bytes - 6;
}

