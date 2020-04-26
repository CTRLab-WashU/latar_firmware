#ifndef APP_SCREEN_TOUCH
#define APP_SCREEN_TOUCH				  

#include <stm32f4xx_hal.h>
#include "System/Pin.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Communication/ruart.h"


enum TouchType {
	SolenoidTouch,
	CapacitiveTouch
};

struct RunParameters
{
	TouchType type;
	
	uint32_t interval;
	uint32_t count;
	uint8_t cap;
	
	char count_array[4];
	char interval_array[4];
};

class ScreenTouch
{
	ScreenTouch(){}
public:
	static ScreenTouch& get() {
		static ScreenTouch instance;
		return instance;
	}
	
	ScreenTouch(ScreenTouch const&) = delete;
	void operator=(ScreenTouch const&)  = delete;
		
	void init();
	
	void enable(TouchType type);
	void disable(TouchType type);
	
	void setCapacitance(int setting);
	void enableCapacitiveTouch();
	void disableCapacitiveTouch();
	void enableSolenoidTouch();
	void disableSolenoidTouch();
	
	void calibrate();
	void tap(uint8_t type, TickType_t duration);
	void tapCapacitive(TickType_t duration);
	void tapSolenoid(TickType_t duration);
	
	void extendSolenoid();
	void retractSolenoid();
	
	void tapSequence(TouchType type, uint32_t count, uint32_t interval, uint8_t cap = 0);
	
private:
	static void thread(void const * argument);
	static void normalRun(ScreenTouch * touch);
	static void calibrationRun(ScreenTouch * touch);
	
	void sendData(uint32_t index, uint32_t timestamp);
	void initPwm();
	
	TIM_HandleTypeDef pwm_handle;
	Pin switches[5];
	Pin relay;
	
	char data_buffer[10];
	char stop_buffer[2];
	RunParameters params;
	bool enabled;
	bool calibrating = false;
	
};

#endif