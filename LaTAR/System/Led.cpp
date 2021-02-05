#include "Led.h"

Led::Led(GPIO_TypeDef *bank, uint16_t pin)
{	
	this->bank = bank;
	this->pin = pin;
}

Led::Led()
{	

}
	
Led::~Led()
{	
	
}	
	
void Led::init()
{	
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
	GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Pin = pin;
	HAL_GPIO_Init(bank, &GPIO_InitStructure);
	
	off();
}	

void Led::on()
{
	HAL_GPIO_WritePin(bank, pin, GPIO_PIN_RESET);
}

void Led::off()
{
	HAL_GPIO_WritePin(bank, pin, GPIO_PIN_SET);
}

void Led::toggle()
{
	HAL_GPIO_TogglePin(bank, pin);
}
	

