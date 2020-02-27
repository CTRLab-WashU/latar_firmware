#include "ScreenDetect.h"
#include <stdio.h>
#include "Commands.h"
#include "SyncTimer.h"
#include "Communication/uart.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "cmsis_os.h"
#include "config.h"
#include "Indicator.h"

#include "Communication/ruart.h"

osSemaphoreId detect_semaphore;

uint32_t timestamp;
char display_detect_buffer[32];

void ScreenDetect::init()
{	
	GPIO_InitTypeDef gpio_init;
	
	__GPIOA_CLK_ENABLE();
	__ADC2_CLK_ENABLE();
	
	// Screen sensor PA3 ADC123_IN3
	gpio_init.Mode		= GPIO_MODE_ANALOG;
	gpio_init.Speed		= GPIO_SPEED_HIGH;
	gpio_init.Pull		= GPIO_NOPULL;
	gpio_init.Pin		= GPIO_PIN_6;
	HAL_GPIO_Init(GPIOA, &gpio_init);
	
	
	// Init Timer 1 to drive ADC1 conversions
	__TIM3_CLK_ENABLE();
	timer_handle.Instance			= TIM3;
	timer_handle.Init.Prescaler		= 84 - 1;
	timer_handle.Init.Period		= 500 - 1;
	timer_handle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	timer_handle.Init.CounterMode	= TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&timer_handle);

	// Init TIM1 TRGO to start ADC conversion
	TIM_MasterConfigTypeDef timer_master_config;
	timer_master_config.MasterOutputTrigger = TIM_TRGO_UPDATE;
	timer_master_config.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&timer_handle, &timer_master_config);

	// Init ADC2
	adc_handle.Instance						= ADC2;
	adc_handle.Init.ClockPrescaler			= ADC_CLOCKPRESCALER_PCLK_DIV2;
	adc_handle.Init.Resolution				= ADC_RESOLUTION_12B;
	adc_handle.Init.ScanConvMode			= ENABLE;
	adc_handle.Init.ContinuousConvMode		= DISABLE;
	adc_handle.Init.DiscontinuousConvMode	= DISABLE;
	adc_handle.Init.NbrOfDiscConversion		= 0;
	adc_handle.Init.ExternalTrigConvEdge	= ADC_EXTERNALTRIGCONVEDGE_RISING;
	adc_handle.Init.ExternalTrigConv		= ADC_EXTERNALTRIGCONV_T3_TRGO;
	adc_handle.Init.DataAlign				= ADC_DATAALIGN_RIGHT;
	adc_handle.Init.NbrOfConversion			= 1;
	adc_handle.Init.DMAContinuousRequests	= DISABLE;
	adc_handle.Init.EOCSelection			= ADC_EOC_SINGLE_SEQ_CONV;
     
	// Commit ADC settings
	HAL_ADC_Init(&adc_handle);

	// Configure ADC channel
	ADC_ChannelConfTypeDef adc_channel;
	adc_channel.Channel			= ADC_CHANNEL_6;
	adc_channel.Rank			= 1;
	adc_channel.SamplingTime	= ADC_SAMPLETIME_480CYCLES;
	adc_channel.Offset			= 0;
  
	if (HAL_ADC_ConfigChannel(&adc_handle, &adc_channel) != HAL_OK) {
		printd("screen detect failed to configure adc channel");
		return;
	}
	
	HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
	
	if (HAL_ADC_Start_IT(&adc_handle) != HAL_OK) {
		printd("screen detect failed to start adc");
		return;
	}
	
	osThreadDef(screen_detect_thread, thread, osPriorityNormal, 0, 512);
	osThreadCreate(osThread(screen_detect_thread), (void*)this);
	
	osSemaphoreDef(detect_semaphore);
	detect_semaphore = osSemaphoreCreate(osSemaphore(detect_semaphore), 1);
	
}	

void ScreenDetect::enable()
{
	index = 0;
	enabled = true;
	indicator_pulse_off();
	HAL_TIM_Base_Start(&timer_handle);
}

void ScreenDetect::disable()
{
	enabled = false;
	indicator_set_flash();
	HAL_TIM_Base_Stop(&timer_handle);
}

bool ScreenDetect::isEnabled()
{
	return enabled;
}

void ScreenDetect::update(uint32_t value)
{		
	if (!enabled) {
		return;
	}
	
	if (isDark && value > threshold_light) {
		timestamp = SyncTimer::get().getTimestamp();
		index++;
		isDark = false;
		return;
	}
		
	if (!isDark && value < threshold_dark) {
		timestamp = SyncTimer::get().getTimestamp();
		index++;
		isDark = true;
		return;
	}
}

void ScreenDetect::sendData(uint32_t index, uint32_t timestamp, uint8_t value)
{
	ruart_write_displaydata(index, timestamp, value);
}

void ScreenDetect::thread(void const * argument)
{	
	ScreenDetect * detect = (ScreenDetect*)argument;
	
	bool isDark = detect->isDark;
	for (;;) {
		if (detect->isDark!= isDark) {
			isDark = detect->isDark;
			
			if (detect->isDark) {
				indicator_pulse_off();
				detect->sendData(detect->index, timestamp, 0);
				printd("dark\n");
			} else {
				indicator_pulse_on();
				detect->sendData(detect->index, timestamp, 1);
				printd("light\n");
			}		
		}
	}
	
}