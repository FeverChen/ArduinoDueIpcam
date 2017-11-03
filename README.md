# ArduinoDueIpcam

Introduction:
The small IP camera made by Arduino Due development board will send continous JPEG frames to client side.

The hardware requirement:
1. The arduino due board
2. uCamIII 
3. Arduino Ethernet Shield2

Development environment:
Atmel studio

pin setting:
<pre>
uCamIII      <->    Arduino Due
	5V                 5V
	GND                GND
	RES                RESET
	TX                 RX1(pin19)
	RX                 TX1(pin18)
</pre>

Console baud rate: 115200 bps

Test method:
use JPlayer (motion JPEG player) connect to the Arduino Due tiny web server

for example:
if server ip is 192.168.1.100
under JPlayer, type http://192.168.1.100
