#ifndef SYSTEM_ADC
#define SYSTEM_ADC				  

#include <stm32f4xx_hal.h>

void register_adc_callback(ADC_TypeDef *adc, ADC_HandleTypeDef *handle, void(*callback)(uint32_t));

#endif