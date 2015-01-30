#ifndef MENU_h
#define MENU_h

#include <Task.h>
#include <LiquidTWI2.h>
#include "Globals.h"


/*static LiquidTWI2 lcd(LiquidTWIAddress);*/

class Menu :
public TimedTask
{
	public:
	Menu(uint32_t _rate);
	void  init();
	void  run(uint32_t now);
	private:
	uint32_t rate;
	byte  OldValue;
	byte  count;
	byte ShowTimeCounter;
	void  Execute();
	bool  on;
	int   MenuPos;
	int   Index;
};
#endif
