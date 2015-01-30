#include <Arduino.h>
#include "Menu.h"
#include "Globals.h"
#include "Utils.h"
#include <avr/wdt.h>

extern float   Temperatures[5];
extern byte    OnWireAddress[9];
extern String  Time;

Menu::Menu(uint32_t _rate) :
TimedTask(millis()),
rate(_rate),
on(false)
{
}

void Menu::init()
{
	Serial.println("Menu");
	OldValue = 0;
	MenuPos = 2;

	// set default menu
	LCDSetMenu(MenuTexts[MenuPos]);
	count = 0;
	ShowTimeCounter = 0;
}


void Menu::run(uint32_t now)
{
	wdt_reset();
	if (LCD_Enabled) {
		//long x = micros();
		//Serial.println("Menu");
		ShowTimeCounter ++;

		setLed(MenuLed);
	
		if (ShowTimeCounter >= 10) {
			ShowTimeCounter = 0;
			LCDSetTime();
		}
		// read MCP23017 portA (buttons)
		byte  n = lcd.readButtons();
		//Serial.println(n);
		if (n != OldValue)
		{
			OldValue = n;
			byte  OldMenuPos = MenuPos;
			if (checkBit(n, 0)) MenuPos = MenuStruct[MenuPos][1]; // S
			if (checkBit(n, 1)) MenuPos = MenuStruct[MenuPos][0]; // E
			if (checkBit(n, 2)) MenuPos = MenuStruct[MenuPos][3]; // N
			if (checkBit(n, 3)) MenuPos = MenuStruct[MenuPos][2]; // W
			if (checkBit(n, 4)) Execute();                 // Enter
			if (MenuPos == 0) MenuPos = OldMenuPos;
			if (OldMenuPos != MenuPos)
			{
				LCDSetMenu(MenuTexts[MenuPos]);
				Index = 0;
			}
		}
		incRunTime(rate);
		//Serial.println(micros()-x);
	}
}

void Menu::Execute()
{
	if (LCD_Enabled) {	//Serial.print("Menupos = ");
		//Serial.println(MenuPos);
		switch (MenuPos)
		{
			case 2:
			addAlarm("This is a test alarm");
			break;

			case 3:
			readSetup();
			break;

			case 5:
			getAddress();
			for (int i = 0; i < 8; i++)
			{  // we need 9 bytes
				lcd.print(OnWireAddress[i], HEX);
				lcd.print(" ");
			}
			break;

			case 9:
			case 11:
			if (MenuStruct[MenuPos][4] != 255)
			{
				float    T = Temperatures[MenuStruct[MenuPos][4]];
				String   Data = FormatTemp(T);
				lcd.print(Data);
			}
			break;
		}
	}	
}
