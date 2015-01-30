#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <UIPEthernet.h>

#include "Clock.h"
#include "Utils.h"
#include "Globals.h"

RTC_DS1307 RTC;



Clock::Clock(uint32_t _rate) :
TimedTask(millis()),
rate(_rate),
on(false)
{
}

void Clock::readTime()
{
	DateTime NOW = RTC.now();
	RealTime = NOW.get();
}

void Clock::init()
{
	Wire.begin();
	RTC.begin();
	readTime();
	//check power failure
	// time is saved in RTC memory by run function
	// in case of power failure, RTC continue to run, but time is not saved
	DateTime CurrentTime = RTC.now();
	time_t  CT = CurrentTime.get();
    
	// get last saved time
	DateTime LastTimeSaved = RTC.lastSaved();
	time_t  LT = LastTimeSaved.get();

	// calc diff in seconds between time and saved time
	if ((CT - LT) > 1)
	{
		String   Ala = "\nPower outage of: ";
	    	tmElements_t TE;
		breakTime(CT - LT, TE, 1900);
		//Ala += TE.Days;
		int H = TE.Day * 24;
		if (TE.Hour + H < 10) Ala += '0';
		Ala += TE.Hour;
		Ala += "h ";
		if (TE.Minute < 10) Ala += '0';
		Ala += TE.Minute;
		Ala += "min ";
		if (TE.Second < 10) Ala += '0';
		Ala += TE.Second;
		Ala += "sec.";
		Ala += "\nRestored at: ";
		Ala += GetDateTimeString();
		addAlarm(Ala);
		Serial.println(Ala);
	}
	
	if (ethernetConnect()) {
		LCDSetStatus("Adjust RTC via NTP");
		time_t NP = getNtpTime();
		tmElements_t TE;
		breakTime(NP, TE, 1970);
		Serial.println("NTP Done");
		RTC.adjust(DateTime(TE.Year,TE.Month,TE.Day,TE.Hour,TE.Minute,TE.Second));
       	Serial.println(GetDateTimeString());
		LCDSetStatus(" ");   
	}
	
    	Serial.println(GetDateTimeString());
}


void Clock::run(uint32_t now)
{
	Serial.print('.');
	//long x = micros();
	setLed(ClockLed);
	BlinkLed14();
	// save current time in RTC memory
	RTC.saveTime();
	readTime();
	
	if (EthernetConnected) {
		Ethernet.maintain();
	}
	incRunTime(rate);
	//Serial.println(micros()-x);
}

