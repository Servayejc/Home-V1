#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <UIPEthernet.h>
#include <Temboo.h>
#include "Mail.h"
#include "Utils.h"
#include "Globals.h"

UIPClient      sendMailClient;

SendAlarm::SendAlarm(uint32_t _rate) :
TimedTask(millis()),
rate(_rate),
on(false)
{
}

void SendAlarm::init()
{

}

bool SendAlarm::canRun(uint32_t now) {
	
	return (now >= runTime) & runFlag & alarmsEnabled();// & ethernetConnect();
}

void SendAlarm::logToXively()
{
	String	Data = "\n";
	Data += "999";
	Data += ',';
	Data += '"';
	Data += AlarmText;
	Data += '"';
	Data += "\n";
	SendToXyvely(Data, true);
}

void SendAlarm::run(uint32_t now)
{
	AlarmText.trim();
	Serial.println("alarms");
	if (AlarmText.length() > 0)
	{
		logToXively();
		
		setLed(SendAlarmsLed);
		Serial.print("-");
		Serial.print(AlarmText);
		Serial.println("-");
		LCDSetStatus("Sending Alarm");
		Serial.println("Sending alarm.");
		TembooChoreo   SendEmailChoreo(sendMailClient);

		// Set Temboo account credentials
		SendEmailChoreo.setAccountName(TEMBOO_ACCOUNT);
		SendEmailChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
		SendEmailChoreo.setAppKey(TEMBOO_APP_KEY);
		// Set profile to use for execution
		SendEmailChoreo.setProfile("arduinoTemperatureEmail");
		// Set Choreo inputs
		String   MessageBodyValue = String("Alarm!\n") + AlarmText;
		SendEmailChoreo.addInput("MessageBody", MessageBodyValue);
		SendEmailChoreo.addInput("Subject", "Alarme Maison");
		// Identify the Choreo to run
		SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");
		// Run the Choreo
		unsigned int   returnCode = SendEmailChoreo.run();

		// A return code of zero means everything worked
		Serial.println(returnCode);
		if (returnCode == 0)
		{
			Serial.println("Done!\n");
			AlarmText = "";
		}
		else
		{
			// A non-zero return code means there was an error
			// Read and print the error message
			while (SendEmailChoreo.available())
			{
				char  c = SendEmailChoreo.read();
				Serial.print(c);
			}

			Serial.println();
			incRunTime(5000); //  error, wait 5 sec;
		}

		SendEmailChoreo.close();
		showMemory();
		LCDSetStatus(" ");
	}
	
	incRunTime(rate);
}
