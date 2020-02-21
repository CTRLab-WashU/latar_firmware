#ifndef APP_SCREEN_DATA
#define APP_SCREEN_DATA				  

#include <stm32f4xx_hal.h>

class ScreenData
{
public:
	ScreenData(uint32_t index, uint32_t timestamp);
	ScreenData();
	~ScreenData();
	
	const char* toJson();
	
	uint32_t index;
	uint32_t timestamp;
};

#endif