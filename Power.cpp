#include <Arduino.h>
#include "Power.h"
#include "Utils.h"

extern long   RealTime;

Power::Power(uint32_t _rate) :
TimedTask(millis()),
rate(_rate),
on(false)
{
}

void  Power::init(){
	for (int i = 0; i <= 8; i++){
		time[i] = 0;
		Energy[i] = 0;
	}
}

void  Power::run(uint32_t now){
    //long x = micros();
	setLed(ReadPowerLed);
	ReadPower();
	incRunTime(rate);
	//Serial.println(micros()-x);
}

void Power::calcEnergy(int channel, long seconds){
    Energy[channel] += Pow[channel] * seconds / 3600;   
}

void  Power::ReadPower(){
	
	data = expanderRead(ExtenderLowAdd);
	data = data ^ 0xFF;
	if (data != oldData) {
		for (int i = 0; i <= 8; i++) {
			if (checkBit(data,i)) {
				if (time[i] == 0) {
					time[i] = RealTime;
					Serial.print("channel ");
					Serial.println(i);
				}
			}
			else {
				if (time[i] != 0) {
					Serial.print(RealTime-time[i]);
					Serial.print(" sec on channel ");
					Serial.println(i);
					calcEnergy(i,RealTime-time[i]);
					time[i] = 0;
				}
			}
		}
		oldData = data;
	}
	
}

