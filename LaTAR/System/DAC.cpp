#include "StatusLed.h"

StatusLed::StatusLed(GPIO_TypeDef *bank, uint16_t pin)
{	
	this->led = Led(bank, pin);
	this->counter = 0;
}
	
StatusLed::~StatusLed()
{	
	
}	
	
void StatusLed::init()
{	
	led.init();
}	

void StatusLed::setOkay()
{
	led.off();
	status = StatusLed::Status_Okay;
}

void StatusLed::setError()
{
	led.off();
	status = StatusLed::Status_Error;
}

void StatusLed::tick()
{
	switch (status)
	{
	case StatusLed::Status_Init:
		tickOkay();
		break;
	case StatusLed::Status_Okay:
		tickOkay();
		break;
	case StatusLed::Status_Error:
		tickError();
		break;
	}
}

void StatusLed::tickInit()
{
	led.toggle();
	HAL_Delay(100);
}

void StatusLed::tickOkay()
{
	led.on();
	HAL_Delay(500);
}

void StatusLed::tickError()
{
	led.toggle();
	HAL_Delay(500);
}

