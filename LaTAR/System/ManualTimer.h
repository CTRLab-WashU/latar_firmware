#ifndef SYSTEM_MANUAL_TIMER
#define SYSTEM_MANUAL_TIMER	

#include <stm32f4xx_hal.h>

class ManualTimer
{	
public:
	ManualTimer(uint32_t duration);
	ManualTimer();
		
	bool setDuration(const uint32_t duration);
	bool expired();
	void reset();
		
private:
	uint32_t last_reset;
	uint32_t duration;
	bool locked;
	bool hasExpired;
};

#endif