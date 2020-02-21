#include "SyncTimer.h"
#include <iostream>

static TIM_HandleTypeDef sync_timer_handle;
static bool sync_timer_is_valid;
static std::function<void()> sync_timer_callback;

void SyncTimer::init()
{
	// Enable clocks
	__HAL_RCC_TIM2_CLK_ENABLE();
	
	// Configure timer
	sync_timer_handle.Instance				= TIM2;
	sync_timer_handle.Init.Period			= 0xffffffff;
	sync_timer_handle.Init.Prescaler		= 84 - 1;
	sync_timer_handle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	sync_timer_handle.Init.CounterMode		= TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&sync_timer_handle);
	
	// Reset the timer before we start
	reset();
	
	// Enable interrupts. We want to know when the timer rolls over
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	
	// We want update events
	__HAL_TIM_ENABLE_IT(&sync_timer_handle, TIM_IT_UPDATE);

	// Start the timer
	HAL_TIM_Base_Start_IT(&sync_timer_handle);
}

extern "C" void TIM2_IRQHandler()
{
	// Check if this is an update flag
	if(__HAL_TIM_GET_FLAG(&sync_timer_handle, TIM_FLAG_UPDATE))
	{
		// Clear the interrupt
		__HAL_TIM_CLEAR_FLAG(&sync_timer_handle, TIM_FLAG_UPDATE);
		//printf("sync timer is now invalid\r\n");
		sync_timer_is_valid = 0;
		//sync_timer_callback();
	}
}

// Check if the timer is valid
bool SyncTimer::isValid()
{
	return sync_timer_is_valid;
}

uint32_t SyncTimer::getTimestamp(void)
{
	return TIM2->CNT;
}

// Reset the syncrotimer to render it valid again
void SyncTimer::reset()
{
	// Clear the timer
	TIM2->CNT = 0;
	
	// Time is now valid
	sync_timer_is_valid = true;
}

void SyncTimer::registerTimeoutCallback(std::function<void()> callback)
{
	sync_timer_callback = callback;
}