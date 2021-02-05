#ifndef APP_SCREEN_DETECT
#define APP_SCREEN_DETECT				  

#include <stm32f4xx_hal.h>
#include "System/SoftTimer.h"
#include "DataStructures/RingBuffer.h"

class ScreenDetect
{
	TIM_HandleTypeDef timer_handle;
	ADC_HandleTypeDef adc_handle;
	
public:
	static ScreenDetect& get() {
		static ScreenDetect instance;
		return instance;
	}
		
	void init();
	
	void enable(uint32_t threshold);
	void disable();
	bool isEnabled();
	
	void startCalibration();
	void stopCalibration();
	
	private:
	ScreenDetect() {}
	
	static void thread(void const * argument);
	static void adc_irq_handler(uint32_t value);
	static void update_calibration(uint32_t value);
	static void update_value(uint32_t value);
	
};

#endif