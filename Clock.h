#ifndef CLOCK_h
#define CLOCK_h

#include <Task.h>

#include <inttypes.h>

typedef unsigned long   time_t;

class Clock :
public TimedTask
{
	public:
	Clock(uint32_t _rate);
	void  init();
	void  run(uint32_t now);
	private:
	uint32_t rate;
	bool     on;
	void     readTime();
};

#endif
