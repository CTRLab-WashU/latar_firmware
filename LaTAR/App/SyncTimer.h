#ifndef APP_SYNC_TIMER
#define APP_SYNC_TIMER				  

#include <stm32f4xx_hal.h>
#include <functional>
#include <stdint.h>

class SyncTimer
{
	SyncTimer() {}
public:
	static SyncTimer& get() {
		static SyncTimer instance;
		return instance;
	}
		
	void init();
	void reset();
	uint32_t getTimestamp();
	bool isValid();
	
	void registerTimeoutCallback(std::function<void()> callback);
};

#endif