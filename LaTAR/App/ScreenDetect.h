#ifndef APP_SCREEN_DETECT
#define APP_SCREEN_DETECT				  

#include <stm32f4xx_hal.h>
#include "ScreenData.h"
#include "System/SoftTimer.h"

class ScreenDetect
{
	ScreenDetect(){}
public:
	static ScreenDetect& get() {
		static ScreenDetect    instance;
		return instance;
	}
		
	void init();
	void update(uint32_t value);
	
	void enable();
	void disable();
	bool isEnabled();
	
	private:
	static void thread(void const * argument);
	void sendData(uint32_t index, uint32_t timestamp, uint8_t value);
	
	TIM_HandleTypeDef timer_handle;
	ADC_HandleTypeDef adc_handle;
	ScreenData data;
	
	uint32_t threshold_light = 5000;
	uint32_t threshold_dark = 2000;
	
	uint32_t index = 0;
	
	bool isDark = false;
	bool enabled = false;
};

#endif