#include <Arduino.h>
#include "SendData.h"
#include "Utils.h"
#include "Globals.h"

volatile boolean  Connected;

SendData::SendData(uint32_t _rate) :
TimedTask(millis()),
rate(_rate),
on(false)
{
}

void SendData::init()
{
	NT = 0;
}

bool SendData::canRun(uint32_t now)
{
	return (now >= runTime) & runFlag  & (NT > 0); // & ethernetConnect();
}

void SendData::run(uint32_t now)
{
	Serial.println("Send data");
	if (NT > 0){
		setLed(SendDataLed);
		LCDSetStatus("Sending data");
		Serial.println("Send data");
		Data = "\n";
		bool TestMode = (digitalRead(7) == LOW);
		if (TestMode)
		{
			Serial.println("Test");
			for (byte i = 0; i < 2; i++)
			{
				Data += String(990+i);
				Data += ',';
				String Value = FormatTemp(TT[i]/NT);
				Data += Value;
				Data += "\n";
				TT[i] = 0;
			}
		}
		else
		{
			Serial.println("Production");
			for (byte i = 0; i < SensorsCount; i++)
			{
				Data += String(i);
				Data += ',';
				String Value = FormatTemp(TT[i]/NT);
				Data += Value;
				Data += "\n";
				TT[i] = 0;
			}
		}
		NT = 0;
		Serial.println(Data);
		SendToXyvely(Data, false);
		Data = "";
		LCDSetStatus(" ");
		incRunTime(rate);
	}
}


