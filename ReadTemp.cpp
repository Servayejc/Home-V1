#include <OneWire.h>
#include "ReadTemp.h"
#include "Utils.h"
#include "Globals.h"

ReadTemp::ReadTemp(uint32_t _rate) :
TimedTask(millis()),
rate(_rate),
on(false)
{
}

void ReadTemp::init()
{
	NT = 0;
	for (byte i = 0; i < SensorsCount; i++) TT[i] = 0;
}

void ReadTemp::startConversion()
{
	// send broadcast command to all temperature sensors
	// so we do not need to wait 750 ms for each sensor
	ds.reset();
	ds.write(0xCC, 0);                     // SKIP ROM - Send command to all devices
	ds.write(0x44, 0);                     // START CONVERSION
}

float ReadTemp::readOneTemp(byte Ndx)
{
	
	byte  data[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	long  SignBit;
	float celcius;

	ds.reset();
	ds.select(Sensors[Ndx]);
	ds.write(0xBE);                        // READ SCRATCHPAD
	for (byte i = 0; i < 9; i++)
	{                                      // we need 9 bytes
		data[i] = ds.read();

		//Serial.print(data[i], HEX);
	}

	// convert the data to actual temperature
	unsigned int   raw = (data[1] << 8) | data[0];
	SignBit = raw & 0x8000;                // test most sig bit
	if (SignBit) raw = (raw ^ 0xffff) + 1; // negative // 2's comp
	celcius = (raw * 100) / 16.0;
	if (SignBit) celcius = -celcius;
	return celcius;
}

void ReadTemp::run(uint32_t now)
{
    //long x = micros();
	setLed(ReadTempLed);
	NT++;
	for (byte i = 0; i < SensorsCount; i++)
	{
		float T;
		T = readOneTemp(i);
		TT[i] = TT[i] + T;
		Temperatures[i] = TT[i] / NT;
	}

	// prepare next measurement
	startConversion();
	//Serial.print("t");
	incRunTime(rate);
	//Serial.println(micros()-x);
}
