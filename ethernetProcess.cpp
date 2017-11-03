//
//
//

#include <SPI.h>
#include <SD.h>
#include "ethernetProcess.h"
#include "cameraProcess.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0x00, 0x80, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 106);
IPAddress s_dns(168, 95, 1, 1);
IPAddress s_gateway(192, 168, 1, 1);
IPAddress s_subnet(255, 255, 255, 0);

//TFTP
const int TFTP_DATA_SIZE = 256;
const int TFTP_OPCODE_SIZE = 2;
const int TFTP_BLOCKNO_SIZE = 2;
const int TFTP_MAX_PAYLOAD = 256;
const int TFTP_ACK_SIZE(TFTP_OPCODE_SIZE + TFTP_BLOCKNO_SIZE);
const int TFTP_PACKET_MAX_SIZE(TFTP_OPCODE_SIZE + TFTP_BLOCKNO_SIZE + TFTP_MAX_PAYLOAD);

File archive;

unsigned int localPort = 69;      // local port to listen on

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

char packetBuffer[TFTP_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[4];
char ErrorBuffer[37];
String nomArchivo;
int flagR = 0;
int flagW = 0;
bool itsOpen;
int no_block = 0;
int sendit = 0;
int timeout = 0;

EthernetServer server(80);
EthernetClient client = NULL;
bool bWebClientConnected = false;
String jpegBoundary = "*abcdefg*";

bool ethernet_clientConnected()
{
	 return client.connected();
}

void ethernet_setup() {
	Serial.println("Configuring Ethernet using DHCP...");
	// start the Ethernet connection:

	// give the Ethernet shield a second to initialize:
	delay(1000);

	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// try to congifure using IP address instead of DHCP:
		Ethernet.begin(mac, ip, s_dns, s_gateway, s_subnet);
	}
	Serial.print("IP address:");
	Serial.println(Ethernet.localIP());
	Serial.print("Gateway address:");
	Serial.println(Ethernet.gatewayIP());
	Serial.print("DNS address:");
	Serial.println(Ethernet.dnsServerIP());
	Serial.print("Subnet mask:");
	Serial.println(Ethernet.subnetMask());

	// start the Ethernet and UDP:
	//Udp.begin(localPort);
	// ethernet shield using pin 4
	//SD.begin(4);
	Serial.println("Waiting for client connect in");
}

void ethernet_loop()
{
	if (!bWebClientConnected)
	{
		// listen for incoming clients
		client = server.available();
		if (client) {
			Serial.println("new client");
			// an http request ends with a blank line
			boolean currentLineIsBlank = true;
			while (client.connected()) {
				if (client.available()) {
					char c = client.read();
					Serial.write(c);
					// if you've gotten to the end of the line (received a newline
					// character) and the line is blank, the http request has ended,
					// so you can send a reply
					if (c == '\n' && currentLineIsBlank) {
						// send a standard http response header
						bWebClientConnected = true;
						client.print("HTTP/1.1 200 OK\r\n");
						Serial.print("HTTP/1.1 200 OK\r\n");
						client.print("Content-Type: multipart/x-mixed-replace; boundary=\"" + jpegBoundary + "\"\r\n");
						Serial.print("Content-Type: multipart/x-mixed-replace; boundary=\"" + jpegBoundary + "\"\r\n");
						// output the value of each analog input pin
						camera_startPreview();
						break;
					}
					if (c == '\n') {
						// you're starting a new line
						currentLineIsBlank = true;
						} else if (c != '\r') {
						// you've gotten a character on the current line
						currentLineIsBlank = false;
					}
				}
			}
		}
	}
	else if (!client.connected())
	{
		bWebClientConnected = false;
		// close the connection:
		camera_stopPreview();
		client.flush();
		//client.stop();
		Serial.println("client disconnected");
	}
	
}
