#include <Arduino.h>
#include <Wire.h>
#include <MemoryFree.h>
#include <UIPEthernet.h>
#include <RTCLib.h>
#include <OneWire.h>
#include "Globals.h"
#include "Utils.h"

OneWire ds2(OneWireNetwork);
bool Led14Status;

void setLed(int Ndx)
{
	if (LCD_Enabled) {
		Wire.beginTransmission(LedAddress);
		Wire.write(0x12);      // address bank A
		Wire.write(1 << Ndx);  // value to send
		Wire.endTransmission();
	}
}

byte readSetup()
{
	if (LCD_Enabled) {
		Wire.beginTransmission(LedAddress);
		Wire.write(MCP23017_GPIOB); // set MCP23017 memory pointer to GPIOB address
		Wire.endTransmission();
		Wire.requestFrom(LedAddress, 1); // request one byte of data from MCP20317
		byte inputs=Wire.read(); // store the incoming byte into "inputs"
		Serial.println(inputs, BIN);
		//return inputs;
	}
	return 0;
}

bool ethernetConnect()
{
	if (xyvelyEnabled() || alarmsEnabled()) {
		if (!EthernetConnected){
			if (Ethernet.begin(mac) != 0) {
				EthernetConnected = true;
			}
		}
	}
	return (EthernetConnected);
}

bool xyvelyEnabled()
{
	return true; //((readSetup() && ENABLEXYVELY) == 0);
}

bool alarmsEnabled()
{
	return true; //((readSetup() && ENABLEALARMS) == 0);
}

bool checkBit(byte in, int bit)
{
	return in & (1 << bit);
}

void addAlarm(String Ala)
{
	//AlarmText += Time;
	AlarmText += "\t";
	AlarmText += Ala;
	AlarmText += "\n";
};

void showMemory()
{
	Serial.print("Memory used = ");
	Serial.print(0x4000 - freeMemory());
	Serial.println(" of 16384");
};

String FormatTemp(float V)
{
	int      FirstPart = (int) V / 100;
	int      LastPart = (int) V % 100;
	String   R = " ";
	R += FirstPart;
	R += ".";
	R += abs(LastPart);
	return R;
};

// I2C routines to talk to 8574 and 8574A
/* ===================== I2C Expender Set Input mode ====================*/
void expanderSetInput(int i2caddr, byte dir) {
	Wire.beginTransmission(i2caddr);
	Wire.write(dir);  // outputs high for input
	Wire.endTransmission();
}

/* ===================== I2C Expender Write Data ====================*/
void expanderWrite(int i2caddr, byte data){
	Wire.beginTransmission(i2caddr);
	Wire.write(data);
	Wire.endTransmission();
}

/* ===================== I2C Expender Read Data ====================*/
byte expanderRead(int i2caddr) {
	byte i2cdata = 0;
	Wire.requestFrom(i2caddr, 1, true);
	if(Wire.available()) {
		i2cdata = Wire.read();
	}
	return  i2cdata;
	Serial.println(i2cdata);
}


// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

void breakTime(time_t timeInput, tmElements_t &tm, int Offset){
	// break the given time_t into time components
	// this is a more compact version of the C library localtime function
	// note that year is offset from 1970 !!!
	uint8_t year;
	uint8_t month, monthLength;
	time_t time;
	unsigned long days;

	time = timeInput;
	tm.Second = time % 60;
	time /= 60; // now it is minutes
	tm.Minute = time % 60;
	time /= 60; // now it is hours
	tm.Hour = time % 24;
	time /= 24; // now it is days
	tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1
	
	year = 0;
	days = 0;
	while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	
	tm.Year = year+Offset;
	
	days -= LEAP_YEAR(year) ? 366 : 365;
	time  -= days; // now it is days in this year, starting at 0
	//	tm.Days = days;
	days=0;
	month=0;
	monthLength=0;
	for (month=0; month<12; month++) {
		if (month==1) { // february
			if (LEAP_YEAR(year)) {
				monthLength=29;
				} else {
				monthLength=28;
			}
			} else {
			monthLength = monthDays[month];
		}
		
		if (time >= monthLength) {
			time -= monthLength;
			} else {
			break;
		}
	}
	tm.Month = month + 1;  // jan is month 1
	tm.Day = time + 1;     // day of month
}

String GetTimeString() {
	tmElements_t TE;
	DateTime CurrentTime = RealTime;
	time_t CT = CurrentTime.get();
	String Temp("");
	breakTime(CT, TE, 2000);
	if (TE.Hour < 10) Temp = "0";
	{Temp += TE.Hour;}
	Temp += ":";
	if (TE.Minute < 10) Temp += '0';
	Temp += TE.Minute;
	Temp += ":";
	if (TE.Second < 10) Temp += '0';
	Temp += TE.Second;
	return Temp;
}

String GetDateString() {
	tmElements_t TE;
	DateTime CurrentTime = RealTime;
	time_t CT = CurrentTime.get();
	breakTime(CT, TE, 2000);
	String Temp("");
	Temp += TE.Year;
	Temp += "-";
	if (TE.Month < 10) Temp += '0';
	Temp += TE.Month;
	Temp += "-";
	if (TE.Day < 10) Temp += '0';
	Temp += TE.Day;
	return Temp;
}

String GetDateTimeString() {
	String Temp = GetDateString();
	Temp += " ";
	Temp += GetTimeString();
	return Temp;
}

void SendToXyvely(String Data, bool IsAlarm){
	IPAddress server(216, 52, 233, 121); //Xively
	Serial.println("connecting...");
	if (client.connect(server, 80))
	{
		client.print("PUT /v2/feeds/");
		if (IsAlarm == true) {
			client.print(ALARM_FEED);
			//Serial.println(ALARM_FEED);
		}
		else {
			client.print(FEED);
			//Serial.println(FEED);
		}
		client.println(".csv HTTP/1.1");
		client.println("Host: api.pachube.com");
		client.print("X-pachubeApiKey: ");
		if (IsAlarm == true) {
			client.println(ALARM_APIKEY);
			//Serial.println(ALARM_APIKEY);
		}
		else {
			client.println(APIKEY);
			//Serial.println(APIKEY);
		}
		client.print("User-Agent: ");
		client.println(USERAGENT);
		client.print("Content-Length: ");
		client.println(Data.length());
		Serial.print("===");
		Serial.print(Data);
		Serial.println("===");
		// last pieces of the HTTP PUT request:
		client.println("Content-Type: text/csv");
		client.println("Connection: close");

		// here's the actual content of the PUT request:
		client.println(Data);
		Serial.println("send ok");
	}
	else
	{
		// if you couldn't make a connection:
		Serial.println("connection failed");
		Serial.println("disconnecting.");
		client.stop();
	}
}

// NTP Servers:
// IPAddress timeServer(132, 163, 4, 101);
// time-a.timefreq.bldrdoc.gov
IPAddress timeServer(132, 163, 4, 102);
// time-b.timefreq.bldrdoc.gov

// IPAddress timeServer(132, 163, 4, 103);
// time-c.timefreq.bldrdoc.gov

const
int timeZone = -5;

EthernetUDP Udp;

/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;			// Stratum, or type of clock
	packetBuffer[2] = 6;			// Polling Interval
	packetBuffer[3] = 0xEC;			// Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;
	// all NTP fields have been given values, now
	// we can send a packet requesting a timestamp:
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();
}


time_t getNtpTime()
{
	while (Udp.parsePacket() > 0) ; // discard any previously received packets
	//	Serial.println("Transmit NTP Request");
	sendNTPpacket(timeServer);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			//		Serial.println("Receive NTP Response");
			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			return secsSince1900 - 2208988800UL + timeZone * 3600;
		}
	}
	Serial.println("No NTP Response :-(");
	return 0; // return 0 if unable to get the time
}

void fillLine(int ndx){
	if (LCD_Enabled) {
		for (byte i = 0; i < 20-ndx; i++) {
			lcd.print(" ");
		}
	}
}

void LCDSetMenu(String aMenuText) {
	if (LCD_Enabled) {
		lcd.setCursor(0, 0);
		lcd.print(aMenuText);
		fillLine(aMenuText.length());
	}
}

void LCDSetStatus(String aStatus){
	if (LCD_Enabled) {
		lcd.setCursor(0, 1);
		lcd.print(aStatus);
		fillLine(aStatus.length());
	}
}

void LCDSetTime() {
	if (LCD_Enabled) {
		lcd.setCursor(0, 2);
		lcd.print(GetTimeString());
		fillLine(8);
	}
}

void LCDSetAlarm() {
	if (LCD_Enabled) {
		lcd.setCursor(0, 3);
		lcd.print(AlarmText);
		fillLine(AlarmText.length());
	}
}

void getAddress() {
	// read address
	ds2.reset();
	ds2.write(0x33, 1);
	for (int i = 0; i < 8; i++)
	{  // we need 9 bytes
		OnWireAddress[i] = ds2.read();
		Serial.print(OnWireAddress[i], HEX);
		Serial.print(" ");
	}
	Serial.println();
}

void	BlinkLed14() {
	if (Led14Status == false)
	{
		digitalWrite(14, HIGH);
		Led14Status = true;
	}
	else
	{
		digitalWrite(14, LOW);
		Led14Status = false;
	}
}