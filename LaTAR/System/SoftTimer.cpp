#include "SoftTimer.h"
#include "config.h"

#include <stdio.h>

static bool timer_used[configTIMER_QUEUE_LENGTH];

SoftTimer::SoftTimer()
{	
	this->valid = false;
}

SoftTimer::~SoftTimer()
{	
	if (isValid()) {
		xTimerDelete(handle, 0);
		timer_used[internal.id] = false;			
	}
}

bool SoftTimer::init(const char * name, const uint32_t duration, std::function<void()> callback)
{	
	long id = -1;
	for (long i = 0; i < configTIMER_QUEUE_LENGTH; i++) {
		if (!timer_used[i]) {
			id = i;
			break;
		}
	}
	
	if (id==-1) {
		printd("unable to add another timer to queue\n");
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
		printd("unable to create timer\n");
		return false;
	}
	
	this->valid = true;
	timer_used[id] = true;
	printd("timer '%s' added\r\n", name);
	
	return true;
}	

bool SoftTimer::start()
{	
	if (!isValid()) {
		return false;
	}
	bool started = xTimerStart(handle, 100) == pdPASS;
	if (!started) {
		printd("unable to start timer\n");
	}
	return started;
}

void SoftTimer::stop()
{	
	if (isValid()) {
		xTimerStop(handle, 0);
	}
}

void SoftTimer::reset()
{	
	if (isValid()) {
		xTimerReset(handle, 0);
	}
}

bool SoftTimer::isValid()
{
	return valid;
}

bool SoftTimer::isRunning()
{
	if (!isValid()) {
		return false;
	}
	return (bool)xTimerIsTimerActive(handle);
}

void SoftTimer::handle_callbacks(xTimerHandle pxTimert)
{
	Internal *internal = (Internal*) pvTimerGetTimerID(pxTimert);
	internal->callback();
}