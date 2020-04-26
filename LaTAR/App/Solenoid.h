#ifndef APP_SOLENOID
#define APP_SOLENOID				  

#include <stm32f4xx_hal.h>
#include "System/FirFilter.h"

enum Armature {
	Retracted,
	Energizing,
	Extending,
	Extended,
};

class Solenoid
{
	TIM_HandleTypeDef drive_tim_handle;
	TIM_HandleTypeDef sense_tim_handle;
	ADC_HandleTypeDef sense_adc_handle;
		
public:
	Solenoid(){}
	~Solenoid(){}
		
	void init();
	
	void enable();
	void disable();
	
	void extend();
	void retract();
	
	void bind(void(*extended_callback)(void));
	void unbind();

private:
	void initDrive();	
	void initSense();
	
	static void sense_irq_handler(uint32_t value);
	static void update(uint32_t value);
	
};

#endif