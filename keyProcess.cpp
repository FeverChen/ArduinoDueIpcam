// 
// 
// 

#include "keyProcess.h"

void key_setup()
{
	Serial.begin(SERIAL_SPEED);
	while (!Serial);
	Serial.println("Serial initialized");
}

void key_showHelp()
{
	Serial.println("Help");
}

void key_Handler(int key)
{
	switch (key)
	{
	case 'h': // help
		key_showHelp();
		break;
	default:
		break;
	}
}

void key_loop()
{
}
