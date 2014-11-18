#include "uartWIFI.h"

#ifdef UNO
SoftwareSerial mySerial(_DBG_RXPIN_,_DBG_TXPIN_);
#endif

#ifdef DEBUG
#define DBG(message)    DebugSerial.print(message)
#define DBGLN(message)    DebugSerial.println(message)
#define DBGW(message)    DebugSerial.write(message)
#else
#define DBG(message)
#define DBGLN(message)
#define DBGW(message)
#endif // DEBUG

int8_t chlID;		//client id(0-4)
bool wifiPresent;   //track is wifi card is present

bool WIFI::begin(void)
{
	boolean result = false;
	_cell.begin(115200);	//The default baud rate of ESP8266 is 115200
	
	DebugSerial.begin(debugBaudRate);

	_cell.flush();
	_cell.setTimeout(3000);
	println(F("AT+RST"));
	result = _cell.find("ready");
	if(result)
	{
		DBGLN(F("Module is ready"));
		wifiPresent = true;
	}
    else
	{
		DBGLN(F("Module have no response"));
		wifiPresent = false;
	}
	return wifiPresent;
}

/*************************************************************************
//Initialize port

	mode:	setting operation mode
		STA: 	Station
		AP:	 	Access Point
		AT_STA:	Access Point & Station

	chl:	channel number
	ecn:	encryption
		OPEN          0
		WEP           1
		WAP_PSK       2
		WAP2_PSK      3
		WAP_WAP2_PSK  4		

	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/
bool WIFI::Initialize(byte mode, String ssid, String pwd, byte chl, byte ecn)
{
	if (!wifiPresent) return false;

	if (mode == STA)
	{	
		bool b = confMode(mode);
		if (!b)
		{
			return false;
		}
		Reset();
		confJAP(ssid, pwd);
	}
	else if (mode == AP)
	{
		bool b = confMode(mode);
		if (!b)
		{
			return false;
		}
		Reset();
		confSAP(ssid, pwd, chl, ecn);
	}
	else if (mode == AP_STA)
	{
		bool b = confMode(mode);
		if (!b)
		{
			return false;
		}
		Reset();
		confJAP(ssid, pwd);
		confSAP(ssid, pwd, chl, ecn);
	}
	
	return true;
}

/*************************************************************************
//Set up tcp or udp connection

	type:	tcp or udp
	
	addr:	ip address
	
	port:	port number
	
	a:	set multiple connection
		0 for sigle connection
		1 for multiple connection
		
	id:	id number(0-4)

	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/
boolean WIFI::ipConfig(byte type, String addr, int port, boolean a, byte id)
{
	if (!wifiPresent) return false;

	boolean result = false;
	if (a == 0 )
	{
		confMux(a);
		
		long timeStart = millis();
		while (1)
		{
			long time0 = millis();
			if (time0 - timeStart > 5000)
			{
				break;
			}
		}
		result = newMux(type, addr, port);
	}
	else if (a == 1)
	{
		confMux(a);
		long timeStart = millis();
		while (1)
		{
			long time0 = millis();
			if (time0 - timeStart > 5000)
			{
				break;
			}
		}
		result = newMux(id, type, addr, port);
	}
	return result;
}

/*************************************************************************
//receive message from wifi

	buf:	buffer for receiving data
	
	chlID:	<id>(0-4)

	return:	size of the buffer
	

***************************************************************************/
int WIFI::ReceiveMessage(char *buf)
{
	if (!wifiPresent) return 0;
	
	//+IPD,<len>:<data>
	//+IPD,<id>,<len>:<data>
	String data = "";
	if (_cell.available()>0)
	{
		
		unsigned long start;
		start = millis();
		char c0 = _cell.read();
		if (c0 == '+')
		{
			
			while (millis()-start<5000) 
			{
				if (_cell.available()>0)
				{
					char c = _cell.read();
					data += c;
				}
				if (data.indexOf("\nOK")!=-1)
				{
					break;
				}
			}
			
			int sLen = strlen(data.c_str());
			int i,j;
			for (i = 0; i <= sLen; i++)
			{
				if (data[i] == ':')
				{
					break;
				}
				
			}
			boolean found = false;
			for (j = 4; j <= i; j++)
			{
				if (data[j] == ',')
				{
					found = true;
					break;
				}
				
			}
			int iSize;
			DBGLN(data);
			
			if(found ==true)
			{
			String _id = data.substring(4, j);
			chlID = _id.toInt();
			String _size = data.substring(j+1, i);
			iSize = _size.toInt();
			//DBG(_size);
			String str = data.substring(i+1, i+1+iSize);
			strcpy(buf, str.c_str());	
			//DBG(str);
						
			}
			else
			{			
			String _size = data.substring(4, i);
			iSize = _size.toInt();
			//DBGLN(iSize);
			String str = data.substring(i+1, i+1+iSize);
			strcpy(buf, str.c_str());
			//DBG(str);
			}
			return iSize;
		}
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////


/*************************************************************************
//reboot the wifi module



***************************************************************************/
void WIFI::Reset(void)
{
 	if (!wifiPresent) return;
 	
   println(F("AT+RST"));
	unsigned long start;
	start = millis();
    while (millis()-start<5000) {                            
        if(_cell.find("ready")==true)
        {
			DBGLN(F("reboot wifi is OK"));
           break;
        }
    }
}

/*********************************************
 *********************************************
 *********************************************
             WIFI Function Commands
 *********************************************
 *********************************************
 *********************************************
 */

/*************************************************************************
//inquire the current mode of wifi module

	return:	string of current mode
		Station
		AP
		AP+Station

***************************************************************************/
String WIFI::showMode()
{
	if (!wifiPresent) return "";

    String data;
    println(F("AT+CWMODE?"));  
	unsigned long start;
	start = millis();
    while (millis()-start<2000) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("OK")!=-1)
     {
         break;
     }
  }
    if(data.indexOf("1")!=-1)
    {
        return "Station";
    }else if(data.indexOf("2")!=-1)
    {
            return "AP";
    }else if(data.indexOf("3")!=-1)
    {
         return "AP+Station";
    }
}



/*************************************************************************
//configure the operation mode

	a:	
		1	-	Station
		2	-	AP
		3	-	AP+Station
		
	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/

bool WIFI::confMode(byte a)
{
	if (!wifiPresent) return false;

    String data;
    print(F("AT+CWMODE="));  
    println(String(a));
	
	unsigned long start;
	start = millis();
    while (millis()-start<2000) {
      if(_cell.available()>0)
      {
      char a =_cell.read();
      data=data+a;
      }
      if (data.indexOf("OK")!=-1 || data.indexOf("no change")!=-1)
      {
          return true;
      }
	  if (data.indexOf("ERROR")!=-1 || data.indexOf("busy")!=-1)
	  {
		  while(_cell.available()>0)
		  {
			  char a =_cell.read();
			  data=data+a;
		  }
		  DBGLN(data);
		  return false;
	  }
	  
   }
}


/*************************************************************************
//show the list of wifi hotspot
		
	return:	string of wifi information
		encryption,SSID,RSSI
		

***************************************************************************/

String WIFI::showAP(void)
{
	if (!wifiPresent) return "";

    String data;
	_cell.flush();
    println(F("AT+CWLAP"));  
	delay(1000);
	while(1);
    unsigned long start;
	start = millis();
    while (millis()-start<8000) {
   if(_cell.available()>0)
   {
     char a =_cell.read();
     data=data+a;
   }
     if (data.indexOf("OK")!=-1 || data.indexOf("ERROR")!=-1 )
     {
         break;
     }
  }
    if(data.indexOf("ERROR")!=-1)
    {
        return "ERROR";
    }
    else{
       char head[4] = {0x0D,0x0A};   
       char tail[7] = {0x0D,0x0A,0x0D,0x0A};        
       data.replace("AT+CWLAP","");
       data.replace("OK","");
       data.replace("+CWLAP","WIFI");
       data.replace(tail,"");
	   data.replace(head,"");

        return data;
        }
 }


/*************************************************************************
//show the name of current wifi access port
		
	return:	string of access port name
		AP:<SSID>
		

***************************************************************************/
String WIFI::showJAP(void)
{
	if (!wifiPresent) return "";
	
	_cell.flush();
    println(F("AT+CWJAP?"));  
      String data;
	  unsigned long start;
	start = millis();
    while (millis()-start<3000) {
       if(_cell.available()>0)
       {
       char a =_cell.read();
       data=data+a;
       }
       if (data.indexOf("OK")!=-1 || data.indexOf("ERROR")!=-1 )
       {
           break;
       }
    }
      char head[4] = {0x0D,0x0A};   
      char tail[7] = {0x0D,0x0A,0x0D,0x0A};        
      data.replace("AT+CWJAP?","");
      data.replace("+CWJAP","AP");
      data.replace("OK","");
	  data.replace(tail,"");
      data.replace(head,"");
      
          return data;
}


/*************************************************************************
//configure the SSID and password of the access port
		
		return:
		true	-	successfully
		false	-	unsuccessfully
		

***************************************************************************/
boolean WIFI::confJAP(String ssid , String pwd)
{
	if (!wifiPresent) return false;
	
    print(F("AT+CWJAP="));
    print(F("\""));     //"ssid"
    print(ssid);
    print(F("\""));

    print(F(","));

    print(F("\""));      //"pwd"
    print(pwd);
    println(F("\""));


    unsigned long start;
	start = millis();
    while (millis()-start<3000) {                            
        if(_cell.find("OK")==true)
        {
		   return true;
           
        }
    }
	return false;
}
/*************************************************************************
//quite the access port
		
		return:
			true	-	successfully
			false	-	unsuccessfully
		

***************************************************************************/

boolean WIFI::quitAP(void)
{
	if (!wifiPresent) return false;

    println(F("AT+CWQAP"));
    unsigned long start;
	start = millis();
    while (millis()-start<3000) {                            
        if(_cell.find("OK")==true)
        {
		   return true;
           
        }
    }
	return false;

}

/*************************************************************************
//show the parameter of ssid, password, channel, encryption in AP mode
		
		return:
			mySAP:<SSID>,<password>,<channel>,<encryption>

***************************************************************************/
String WIFI::showSAP()
{
	if (!wifiPresent) return "";
	
    println(F("AT+CWSAP?"));  
      String data;
      unsigned long start;
	start = millis();
    while (millis()-start<3000) {
       if(_cell.available()>0)
       {
       char a =_cell.read();
       data=data+a;
       }
       if (data.indexOf("OK")!=-1 || data.indexOf("ERROR")!=-1 )
       {
           break;
       }
    }
      char head[4] = {0x0D,0x0A};   
      char tail[7] = {0x0D,0x0A,0x0D,0x0A};        
      data.replace("AT+CWSAP?","");
      data.replace("+CWSAP","mySAP");
      data.replace("OK","");
	  data.replace(tail,"");
      data.replace(head,"");
      
          return data;
}

/*************************************************************************
//configure the parameter of ssid, password, channel, encryption in AP mode
		
		return:
			true	-	successfully
			false	-	unsuccessfully

***************************************************************************/

boolean WIFI::confSAP(String ssid , String pwd , byte chl , byte ecn)
{
	if (!wifiPresent) return false;

    print(F("AT+CWSAP="));  
    print(F("\""));     //"ssid"
    print(ssid);
    print(F("\""));

    print(F(","));

    print(F("\""));      //"pwd"
    print(pwd);
    print(F("\""));

    print(F(","));
    print(String(chl));

    print(F(","));
    println(String(ecn));
        
	unsigned long start;
	start = millis();
    while (millis()-start<3000) {                            
        if(_cell.find("OK")==true )
        {
           return true;
        }
     }
	 
	 return false;

}


/*********************************************
 *********************************************
 *********************************************
             TPC/IP Function Command
 *********************************************
 *********************************************
 *********************************************
 */

/*************************************************************************
//inquire the connection status
		
		return:		string of connection status
			<ID>  0-4
			<type>  tcp or udp
			<addr>  ip
			<port>  port number

***************************************************************************/

String WIFI::showStatus(void)
{
	if (!wifiPresent) return "";
	
    println(F("AT+CIPSTATUS"));  
    String data;
    unsigned long start;
	start = millis();
    while (millis()-start<3000) {
       if(_cell.available()>0)
       {
       char a =_cell.read();
       data=data+a;
       }
       if (data.indexOf("OK")!=-1)
       {
           break;
       }
    }

          char head[4] = {0x0D,0x0A};   
          char tail[7] = {0x0D,0x0A,0x0D,0x0A};        
          data.replace("AT+CIPSTATUS","");
          data.replace("OK","");
		  data.replace(tail,"");
          data.replace(head,"");
          
          return data;
}

/*************************************************************************
//show the current connection mode(sigle or multiple)
		
		return:		string of connection mode
			0	-	sigle
			1	-	multiple

***************************************************************************/
String WIFI::showMux(void)
{
	if (!wifiPresent) return "";
	
    String data;
    println(F("AT+CIPMUX?"));  

      unsigned long start;
	start = millis();
    while (millis()-start<3000) {
       if(_cell.available()>0)
       {
       char a =_cell.read();
       data=data+a;
       }
       if (data.indexOf("OK")!=-1)
       {
           break;
       }
    }
          char head[4] = {0x0D,0x0A};   
          char tail[7] = {0x0D,0x0A,0x0D,0x0A};        
          data.replace("AT+CIPMUX?","");
          data.replace("+CIPMUX","showMux");
          data.replace("OK","");
		  data.replace(tail,"");
          data.replace(head,"");
          
          return data;
}

/*************************************************************************
//configure the current connection mode(sigle or multiple)
		
		a:		connection mode
			0	-	sigle
			1	-	multiple
			
	return:
		true	-	successfully
		false	-	unsuccessfully
***************************************************************************/
boolean WIFI::confMux(boolean a)
{
	if (!wifiPresent) return false;

	print(F("AT+CIPMUX="));
	println(String(a));           
	
	unsigned long start;
	start = millis();
	while (millis()-start<3000) {                            
        if(_cell.find("OK")==true )
        {
           return true;
        }
     }
	 
	 return false;
}


/*************************************************************************
//Set up tcp or udp connection	(signle connection mode)

	type:	tcp or udp
	
	addr:	ip address
	
	port:	port number
		

	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/
boolean WIFI::newMux(byte type, String addr, int port)
{
	if (!wifiPresent) return false;

    String data;
    print(F("AT+CIPSTART="));
    if(type>0)
    {
        print(F("\"TCP\""));
    }else
    {
        print(F("\"UDP\""));
    }
    print(F(","));
    print(F("\""));
    print(addr);
    print(F("\""));
    print(F(","));
    println(String(port));
        
    unsigned long start;
	start = millis();
	while (millis()-start<3000) { 
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("OK")!=-1 || data.indexOf("ALREAY CONNECT")!=-1 || data.indexOf("ERROR")!=-1)
     {
         return true;
     }
  }
  return false;
}
/*************************************************************************
//Set up tcp or udp connection	(multiple connection mode)

	type:	tcp or udp
	
	addr:	ip address
	
	port:	port number
		
	id:	id number(0-4)

	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/
boolean WIFI::newMux( byte id, byte type, String addr, int port)
{
	if (!wifiPresent) return false;

    print(F("AT+CIPSTART="));
    print(F("\""));
    print(String(id));
    print(F("\""));
    if(type>0)
    {
        print(F("\"TCP\""));
    }
	else
    {
        print(F("\"UDP\""));
    }
    print(F(","));
    print(F("\""));
    print(addr);
    print(F("\""));
    print(F(","));
    println(String(port));
    
    String data;
    unsigned long start;
	start = millis();
	while (millis()-start<3000) { 
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("OK")!=-1 || data.indexOf("ALREAY CONNECT")!=-1 )
     {
         return true;
     }
  }
  return false;
  

}
/*************************************************************************
//send data in sigle connection mode

	str:	string of message

	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/
boolean WIFI::Send(String str)
{
	if (!wifiPresent) return false;

    print(F("AT+CIPSEND="));
    println(String(str.length()));
        
    unsigned long start;
	start = millis();
	bool found;
	while (millis()-start<5000) {                            
        if(_cell.find(">")==true )
        {
			found = true;
           break;
        }
    }
	if(found)
	{
		print(str);
	}
	else
	{
		closeMux();
		return false;
	}


    String data;
    start = millis();
	while (millis()-start<5000) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("SEND OK")!=-1)
     {
         return true;
     }
  }
  return false;
}

/*************************************************************************
//send data in multiple connection mode

	id:		<id>(0-4)
	
	str:	string of message

	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/
boolean WIFI::Send(byte id, String str)
{
	if (!wifiPresent) return false;
	
    print(F("AT+CIPSEND="));

    print(String(id));
    print(F(","));
    println(String(str.length()));
        
    unsigned long start;
	start = millis();
	bool found;
	while (millis()-start<5000) {                          
        if(_cell.find(">")==true )
        {
			found = true;
           break;
        }
     }
	 if(found)
		_cell.print(str);
	else
	{
		closeMux(id);
		return false;
	}


    String data;
    start = millis();
	while (millis()-start<5000) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("SEND OK")!=-1)
     {
         return true;
     }
  }
  return false;
}

/*************************************************************************
//Close up tcp or udp connection	(sigle connection mode)


***************************************************************************/
void WIFI::closeMux(void)
{
	if (!wifiPresent) return;

    println(F("AT+CIPCLOSE"));

    String data;
    unsigned long start;
	start = millis();
	while (millis()-start<3000) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("Linked")!=-1 || data.indexOf("ERROR")!=-1 || data.indexOf("we must restart")!=-1)
     {
         break;
     }
  }
}


/*************************************************************************
//Set up tcp or udp connection	(multiple connection mode)
		
	id:	id number(0-4)

***************************************************************************/
void WIFI::closeMux(byte id)
{
	if (!wifiPresent) return;

    print(F("AT+CIPCLOSE="));
    println(String(id));
        
    String data;
    unsigned long start;
	start = millis();
	while (millis()-start<3000) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("OK")!=-1 || data.indexOf("Link is not")!=-1 || data.indexOf("Cant close")!=-1)
     {
         break;
     }
  }

}

/*************************************************************************
//show the current ip address
		
	return:	string of ip address

***************************************************************************/
String WIFI::showIP(void)
{
	if (!wifiPresent) return "";
	
    String data;
    unsigned long start;
	for(int a=0; a<3;a++)
	{
	println(F("AT+CIFSR"));  
	start = millis();
	while (millis()-start<3000) {
     while(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("AT+CIFSR")!=-1)
     {
         break;
     }
	}
	if(data.indexOf(".") != -1)
	{
		break;
	}
	data = "";
  }
	//DBGLN(data);
    char head[4] = {0x0D,0x0A};   
    char tail[7] = {0x0D,0x0D,0x0A};        
    data.replace("AT+CIFSR","");
    data.replace(tail,"");
    data.replace(head,"");
  
    return data;
}

/*************************************************************************
////set the parameter of server

	mode:
		0	-	close server mode
		1	-	open server mode
		
	port:	<port>
		
	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/

boolean WIFI::confServer(byte mode, int port)
{
	if (!wifiPresent) return false;

    print(F("AT+CIPSERVER="));  
    print(String(mode));
    print(F(","));
    println(String(port));
    
    String data;
    unsigned long start;
	start = millis();
	boolean found = false;
	while (millis()-start<3000) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("OK")!=-1 || data.indexOf("no charge")!=-1)
     {
		found = true;
         break;
     }
  }
  return found;
}

/*************************************************************************
//// Set the CIPSERVER timeout.
	timeout:	<timeout>
		
	return:
		true	-	successfully
		false	-	unsuccessfully

***************************************************************************/

boolean WIFI::setTimeout(int timeout)
{
	_cell.print(F("AT+CIPSTO="));
	_cell.println(String(timeout));
	
	String data;
	unsigned long start;
	start = millis();
	boolean found = false;
	while(millis()-start<3000){
		if(_cell.available()>0)
		{
			data += _cell.read();
		}
		if(data.indexOf("OK")!=-1 || data.indexOf("no change")!=-1)
		{
			found = true;
			break;
		}
	}
	return found;
}

/*********************************************
 *********************************************
 *********************************************
             Utility Functions
 *********************************************
 *********************************************
 *********************************************
 */

/*************************************************************************
//print and println
		
		param:	text to send either as string or as F("value")

***************************************************************************/
void WIFI::print(const __FlashStringHelper *ifsh)
{
	DBG(ifsh);
	_cell.print(ifsh);
}

void WIFI::print(const String &s)
{
	DBG(s);
	_cell.print(s);
}

void WIFI::println(const __FlashStringHelper *ifsh)
{
	DBGLN(ifsh);
	_cell.println(ifsh);
}

void WIFI::println(const String &s)
{
	DBGLN(s);
	_cell.println(s);
}
