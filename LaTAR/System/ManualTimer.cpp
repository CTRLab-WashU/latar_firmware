#include "ManualTimer.h"

ManualTimer::ManualTimer(uint32_t duration)
{	
	this->duration = duration;
	hasExpired = false;
	locked = false;
}	

ManualTimer::ManualTimer()
{	
	this->duration = 0;
}

bool ManualTimer::setDuration(const uint32_t duration)
{	
	if (locked) {
		return false;	
	}
	this->duration = duration;
	return true;
}

bool ManualTimer::expired()
{	
	locked = true;
	hasExpired = ((HAL_GetTick() - last_reset) > duration);
	locked = false;
	return hasExpired;
}

void ManualTimer::reset()
{	
	hasExpired = false;
	last_reset = HAL_GetTick(); 
}