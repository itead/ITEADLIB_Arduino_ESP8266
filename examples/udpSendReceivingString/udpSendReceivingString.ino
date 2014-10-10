/*
ESP8266 library

When you use with UNO board, uncomment the follow line in uartWIFI.h.
#define UNO

When you use with MEGA board, uncomment the follow line in uartWIFI.h.
#define MEGA

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
  
  wifi.begin();
  bool b = wifi.Initialize(STA, SSID, PASSWORD);
  if(!b)
  {
    DebugSerial.println("Init error");
  }
  delay(8000);  //make sure the module can have enough time to get an IP address 
  //String ipstring  = wifi.showIP();
  //DebugSerial.println(ipstring);		//show the ip address of module
  
  //delay(5000);
  wifi.ipConfig(UDP, "172.16.1.16", 80);	//Connect to your pc
  
  DebugSerial.println("setup done..");
  wifi.Send("setup done..");
  pinMode(13,OUTPUT);
}
void loop()
{
  
  char buf[100];
  int iLen = wifi.ReceiveMessage(buf);
  if(iLen > 0)
  {
    //if you receive "HIGH" message, set the D13 to high voltage; and vice versa
    if (strcmp(buf, "HIGH") == 0)
    {
      digitalWrite(13, HIGH);
    }
    else if (strcmp(buf, "LOW") == 0)
    {
      digitalWrite(13, LOW);
    }
  }
}