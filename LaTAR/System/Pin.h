#ifndef SYSTEM_PIN
#define SYSTEM_PIN				  

#include <stm32f4xx_hal.h>

class Pin
{
public:
	Pin();
	~Pin();
	
	void init(GPIO_TypeDef *bank, uint16_t pin, uint32_t pull);
	void set();
	void reset();
	void toggle();
	
private:
	GPIO_TypeDef *bank;
	uint16_t pin;
};


#endif