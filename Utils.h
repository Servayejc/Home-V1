#ifndef UTILS_h
#define UTILS_h

#include <Arduino.h>

typedef unsigned long time_t;

void setLed(int Ndx);
byte readSetup();

typedef struct  {
	uint8_t Second;
	uint8_t Minute;
	uint8_t Hour;
	uint8_t Wday;   // day of week, sunday is day 1
	uint8_t Day;
	uint8_t Month;
	uint16_t Year;
	// uint8_t Days;
	// offset from 1970;
} 	tmElements_t, TimeElements, *tmElementsPtr_t;



bool	ethernetConnect();
bool	xyvelyEnabled();
bool	alarmsEnabled();

bool	checkBit(byte in, int bit);

void	addAlarm(String Ala);
void	showMemory();
String  FormatTemp(float V);

void	expanderSetInput(int i2caddr, byte dir);
void	expanderWrite(int i2caddr, byte data);
byte	expanderRead(int i2caddr);

time_t	getNtpTime();
void	breakTime(time_t timeInput, tmElements_t &tm, int Offset);

String	GetTimeString();
String	GetDateString();
String	GetDateTimeString();

void	SendToXyvely(String Data, bool IsAlarm);

void	LCDSetMenu(String aMenuText);
void	LCDSetTime();
void	LCDSetStatus(String aStatus);
void	LCDSetAlarm();

void	getAddress();
void	BlinkLed14();	

#endif



