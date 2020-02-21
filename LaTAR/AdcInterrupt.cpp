#include <stm32f4xx_hal.h>
#include <stdio.h>

#include "App/ScreenDetect.h"

extern "C" {
	
	void ADC_IRQHandler(void)
	{
		// Check if ADC2 conversion is complete
		if(ADC2->SR & ADC_FLAG_EOC) {
			ADC2->SR = 0;
			ScreenDetect::get().update(ADC2->DR);
		}
	}

}
	