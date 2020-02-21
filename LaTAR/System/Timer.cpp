#include "Timer.h"
#include <stdio.h>

static bool timer_used[configTIMER_QUEUE_LENGTH];

Timer::Timer()
{	
	this->valid = false;
}

Timer::~Timer()
{	
	if (isValid()) {
		xTimerDelete(handle, 0);
		timer_used[internal.id] = false;			
	}
}

bool Timer::init(const char * name, const uint32_t duration, std::function<void()> callback)
{	
	long id = -1;
	for (long i = 0; i < configTIMER_QUEUE_LENGTH; i++) {
		if (!timer_used[i]) {
			id = i;
			break;
		}
	}
	
	if (id==-1) {
		printf("Unable to add another timer to queue\r\n");
		return false;
	}
	
	internal.id = id;
	internal.callback = callback;
	
	handle = xTimerCreate(name, 
		(duration / portTICK_RATE_MS),
		pdTRUE,
		(void *)&internal,
		(TimerCallbackFunction_t)&handle_callbacks);
	
	if (handle == NULL) {
		printf("Unable to create timer\r\n");
		return false;
	}
	
	this->valid = true;
	timer_used[id] = true;
	printf("timer '");
	printf(name);
	printf("' added\r\n");
	
	return true;
}	

bool Timer::start()
{	
	if (!isValid()) {
		return false;
	}
	bool started = xTimerStart(handle, 100) == pdPASS;
	if (!started) {
		printf("Unable to start timer\r\n");
	}
	return started;
}

void Timer::stop()
{	
	if (isValid()) {
		xTimerStop(handle, 0);
	}
}

void Timer::reset()
{	
	if (isValid()) {
		xTimerReset(handle, 0);
	}
}

bool Timer::isValid()
{
	return valid;
}

bool Timer::isRunning()
{
	if (!isValid()) {
		return false;
	}
	return (bool)xTimerIsTimerActive(handle);
}

void Timer::handle_callbacks(xTimerHandle pxTimert)
{
	Internal *internal = (Internal*) pvTimerGetTimerID(pxTimert);
	internal->callback();
}