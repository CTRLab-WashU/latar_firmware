#include "Pin.h"

Pin::Pin()
{	

}

Pin::~Pin()
{	
	
}	

void Pin::init(GPIO_TypeDef *bank, uint16_t pin, uint32_t pull)
{	
	this->bank = bank;
	this->pin = pin;
	
	if (bank == GPIOA) {
		__GPIOA_CLK_ENABLE();
	} else if(bank == GPIOB) {
		__GPIOB_CLK_ENABLE();
	} else if(bank == GPIOC) {
		__GPIOC_CLK_ENABLE();		
	} else if(bank == GPIOD) {
		__GPIOD_CLK_ENABLE();		
	} else if(bank == GPIOE) {
		__GPIOE_CLK_ENABLE();		
	}
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Pull = pull;
	GPIO_InitStructure.Pin = pin;
	HAL_GPIO_Init(bank, &GPIO_InitStructure);

}	

void Pin::set()
{
	HAL_GPIO_WritePin(bank, pin, GPIO_PIN_SET);
}

void Pin::reset()
{
	HAL_GPIO_WritePin(bank, pin, GPIO_PIN_RESET);
}

void Pin::toggle()
{
	HAL_GPIO_TogglePin(bank, pin);
}

