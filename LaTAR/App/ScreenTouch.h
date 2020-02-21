#ifndef APP_SCREEN_TOUCH
#define APP_SCREEN_TOUCH				  

#include <stm32f4xx_hal.h>
#include "System/Pin.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Communication/uart.h"

struct RunParameters
{
	uint32_t count;
	uint32_t interval;
	uint8_t type;
	
	char count_array[4];
	char interval_array[4];
};

class ScreenTouch
{
	enum Status {
		Status_Init,
		Status_Okay,
		Status_Error
	};
	ScreenTouch(){}
public:
	static ScreenTouch& get() {
		static ScreenTouch    instance;
		return instance;
	}
	
	ScreenTouch(ScreenTouch const&) = delete;
	void operator=(ScreenTouch const&)  = delete;
		
	void init();
	
	void enable(uint8_t type);
	void disable(uint8_t type);
	
	void enableCapacitiveTouch();
	void disableCapacitiveTouch();
	void enableSolenoidTouch();
	void disableSolenoidTouch();
	
	void tap(uint8_t type, TickType_t duration);
	void tapCapacitive(TickType_t duration);
	void tapSolenoid(TickType_t duration);
	
	void extendSolenoid();
	void retractSolenoid();
	
	void runTapSequence(RxBuffer &buffer);
	void runTapSequence(uint32_t count, uint32_t interval, uint8_t type);
	
private:
	static void thread(void const * argument);
	void sendData(uint32_t index, uint32_t timestamp);
	void initPwm();
	
	TIM_HandleTypeDef pwm_handle;
	Pin switches[5];
	Pin relay;
	
	char data_buffer[10];
	char stop_buffer[2];
	RunParameters params;
	bool enabled;
	
};

#endif