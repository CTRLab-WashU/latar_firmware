#ifndef SYSTEM_TIMER
#define SYSTEM_TIMER	

#include <stm32f4xx_hal.h>
#include <functional>
#include "FreeRTOS.h"
#include "timers.h"

class Timer
{	

public:
	struct Internal
	{
		long id;
		std::function<void()> callback;
	};
	Timer();
	~Timer();	
	
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