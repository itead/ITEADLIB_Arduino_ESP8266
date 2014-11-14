/*
ESP8266 library

When you use with UNO board, uncomment the follow line in uartWIFI.h.
#define UNO

When you use with MEGA board, uncomment the follow line in uartWIFI.h.
#define MEGA

When you use with Leonardo board, uncomment the follow line in uartWIFI.h.
#define LEO

Connection:
When you use it with UNO board, the connection should be like these:
ESP8266_TX->D0
ESP8266_RX->D1
ESP8266_CH_PD->3.3V
ESP8266_VCC->3.3V
ESP8266_GND->GND

FTDI_RX->D3			//The baud rate of software serial can't be higher that 19200, so we use software serial as a debug port
FTDI_TX->D2

When you use it with MEGA board, the connection should be like these:
ESP8266_TX->RX1(D19)
ESP8266_RX->TX1(D18)
ESP8266_CH_PD->3.3V
ESP8266_VCC->3.3V
ESP8266_GND->GND

When you want to output the debug information, please use DebugSerial. For example,

DebugSerial.println("hello");


Note:	The size of message from ESP8266 is too big for arduino sometimes, so the library can't receive the whole buffer because  
the size of the hardware serial buffer which is defined in HardwareSerial.h is too small.

Open the file from \arduino\hardware\arduino\avr\cores\arduino\HardwareSerial.h.
See the follow line in the HardwareSerial.h file.

#define SERIAL_BUFFER_SIZE 64

The default size of the buffer is 64. Change it into a bigger number, like 256 or more.





*/


#define SSID       "Itead_1(Public)"
#define PASSWORD   "27955416"


#include "uartWIFI.h"
#include <SoftwareSerial.h>
WIFI wifi;


void setup()
{
  
  if(!wifi.begin())
  {
  	DebugSerial.println("Begin error");
  }
  bool b = wifi.Initialize(STA, SSID, PASSWORD);
  if(!b)
  {
    DebugSerial.println("Init error");
  }
  DebugSerial.print("Connecting...");

  String wifistring  = wifi.showJAP();
  while(wifistring == "") {
    delay(200);
    wifistring = wifi.showJAP();
  }

  String ipstring = wifi.showIP();
  while(ipstring == "") {
    delay(200);
    ipstring = wifi.showIP();
  }

  DebugSerial.println(wifistring); //show the name of current wifi access port, "AP" label is included here
  DebugSerial.print("IP Address:"); DebugSerial.println(ipstring); //show the ip address of module
}
void loop()
{
  

}
