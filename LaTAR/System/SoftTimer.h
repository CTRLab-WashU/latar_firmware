#ifndef SYSTEM_TIMER_SOFT_H
#define SYSTEM_TIMER_SOFT_H	

#include <stm32f4xx_hal.h>
#include <functional>
#include "FreeRTOS.h"
#include "timers.h"

class SoftTimer
{	

public:
	struct Internal
	{
		long id;
		std::function<void()> callback;
	};
	SoftTimer();
	~SoftTimer();	
	
	bool init(const char * name, const uint32_t duration, std::function<void()> callback_function);
	
	bool start();
	void stop();
	void reset();
	
	bool isValid();
	bool isRunning();
		
private:
	xTimerHandle handle;
	Internal internal;
	bool valid;
	
	static void handle_callbacks(xTimerHandle pxTimert);
};

#endif