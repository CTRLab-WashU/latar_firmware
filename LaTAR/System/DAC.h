#ifndef SYSTEM_STATUS_LED
#define SYSTEM_STATUS_LED				  

#include <stm32f4xx_hal.h>
#include "Led.h"

	class StatusLed
	{
		enum Status {
			Status_Init,
			Status_Okay,
			Status_Error
		};
	public:
		StatusLed(GPIO_TypeDef *bank, uint16_t pin);
		~StatusLed();
		
		void init();
		void tick();
		
		void setOkay();
		void setError();
		
	private:
		Led led;
		Status status = Status_Init;
		uint8_t counter;
		
		void tickInit();
		void tickOkay();
		void tickError();
	};

#endif