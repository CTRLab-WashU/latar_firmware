#include "ADC.h"

struct AdcIrqRef
{
	ADC_TypeDef *adc;
	ADC_HandleTypeDef *handle;
	void(*callback)(uint32_t);
};

AdcIrqRef adc_references[8];
uint8_t adc_reference_count = 0;

void register_adc_callback(ADC_TypeDef *adc, ADC_HandleTypeDef *handle, void(*callback)(uint32_t))
{
	if (adc_reference_count >= 8){
		return;
	}
	
	AdcIrqRef *ref = &adc_references[adc_reference_count];
	ref->adc = adc;
	ref->handle = handle;
	ref->callback = callback;
	
	adc_reference_count++;
}

extern "C" {
	
	void ADC_IRQHandler(void)
	{
		static int i;
		
		for (i = 0; i < adc_reference_count; i++) {
			AdcIrqRef *ref = &adc_references[i];
			if (ref->adc->SR & ADC_FLAG_EOC) {
				ref->adc->SR = 0;
				ref->callback(ref->adc->DR);
			}
		}
	}

}
	