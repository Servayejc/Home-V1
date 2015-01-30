#ifndef READTEMP_h
#define READTEMP_h

#include <Task.h>
#include "Globals.h"

// OneWire channels
static OneWire ds(OneWireReadAddress);


// Read temperatures task
class ReadTemp :
public TimedTask
{
	public:
	ReadTemp(uint32_t _rate);
	void  init();
	void  run(uint32_t now);
	float readOneTemp(byte Ndx);
	void  startConversion();
	private:
	uint32_t rate;
	bool     on;
	int      data[9];
};
#endif
