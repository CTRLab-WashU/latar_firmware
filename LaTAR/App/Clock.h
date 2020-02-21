#ifndef SYSTEM_CLOCK
#define SYSTEM_CLOCK				  

#include <stm32f4xx_hal.h>

namespace System
{
	class Clock
	{
		Clock();
		~Clock();
	public:
		static void init();
		static uint32_t now();
		static void reset();
		static bool isValid();
		
	private:
		static TIM_HandleTypeDef timer_handle;
		static bool timer_valid;
	};
}

#endif