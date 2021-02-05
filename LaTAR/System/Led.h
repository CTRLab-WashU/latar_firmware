#ifndef SYSTEM_LED
#define SYSTEM_LED				  

#include <stm32f4xx_hal.h>

	class Led
	{
	public:
		Led(GPIO_TypeDef *bank, uint16_t pin);
		Led();
		~Led();
		
		void init();
		void on();
		void off();
		void toggle();
		
	private:
		GPIO_TypeDef *bank;
		uint16_t pin;

	};


#endif