#include "Clock.h"

namespace System
{
	
	Clock::Clock()
	{	

	}
	
	Clock::~Clock()
	{	

	}	
	
	void Clock::init()
	{	
		// Enable clocks
		__HAL_RCC_TIM2_CLK_ENABLE();
	
		// Configure timer
		timer_handle.Instance			= TIM2;
		timer_handle.Init.Period		= 0xffffffff;
		timer_handle.Init.Prescaler		= 84 - 1;
		timer_handle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
		timer_handle.Init.CounterMode	= TIM_COUNTERMODE_UP;
		HAL_TIM_Base_Init(&timer_handle);
	
		// Reset the timer before we start
		reset();
	
		// Enable interrupts. We want to know when the timer rolls over
		HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
	
		// We want update events
		__HAL_TIM_ENABLE_IT(&timer_handle, TIM_IT_UPDATE);

		// Start the timer
		HAL_TIM_Base_Start_IT(&timer_handle);
	}	

	uint32_t Clock::now()
	{
		return TIM2->CNT;
	}
	
	void Clock::reset()
	{
		TIM2->CNT = 0;
		timer_valid = true;
	}
	
	bool Clock::isValid()
	{
		return timer_valid;
	}
	
}

