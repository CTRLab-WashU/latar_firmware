#include "ScreenDetect.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "cmsis_os.h"
#include "config.h"

#include "Communication/ruart.h"
#include "System/FirFilter.h"
#include "System/ADC.h"
#include "Indicator.h"
#include "SyncTimer.h"
#include "Commands.h"

uint32_t timestamp;
uint32_t minValue = 200000;
uint32_t maxValue = 0;

static uint32_t last_filtered;
static Filter::Fir filter;

static bool calibrating = false;
static uint32_t threshold = 3500;
static uint32_t index = 0;

volatile bool dark = false;
volatile bool enabled = false;

void ScreenDetect::adc_irq_handler(uint32_t value)
{
	if (value > 40000) {
		return;
	}
	filter.enqueue(value);
	if (!filter.primed()) {
		return;
	}
	if (calibrating) {
		update_calibration(filter.value());	
		return;
	}
	update_value(filter.value());
}

void ScreenDetect::init()
{	
	filter.resize(64);
	
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
	
	register_adc_callback(ADC2, &adc_handle, adc_irq_handler);
	
	HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
	
	if (HAL_ADC_Start_IT(&adc_handle) != HAL_OK) {
		printd("screen detect failed to start adc");
		return;
	}
	
	osThreadDef(screen_detect_thread, thread, osPriorityNormal, 0, 512);
	osThreadCreate(osThread(screen_detect_thread), (void*)this);
	
}	

void ScreenDetect::enable(uint32_t threshold)
{
	::threshold = threshold;
	
	index = 0;
	calibrating = false;
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

void ScreenDetect::startCalibration()
{
	minValue = 200000;
	maxValue = 0;
	calibrating = true;
	
	indicator_pulse_on();
	HAL_TIM_Base_Start(&timer_handle);
}

void ScreenDetect::stopCalibration()
{
	calibrating = false;
	ruart_write(Commands::CALIBRATION_DISPLAY_STOP, minValue, maxValue);
	
	indicator_set_flash();
	HAL_TIM_Base_Stop(&timer_handle);
}

bool ScreenDetect::isEnabled()
{
	return enabled;
}

void ScreenDetect::update_calibration(uint32_t value)
{
	if (value < minValue) {
		minValue = value;
	}
	else if (value > maxValue) {
		maxValue = value;
	}
}
void ScreenDetect::update_value(uint32_t value)
{			
	if (!enabled) {
		return;
	}
	
	if (dark && value > threshold) {
		timestamp = SyncTimer::get().getTimestamp();
		index++;
		dark = false;
		return;
	}
		
	if (!dark && value < threshold) {
		timestamp = SyncTimer::get().getTimestamp();
		index++;
		dark = true;
		return;
	}
}

void ScreenDetect::thread(void const * argument)
{	
	ScreenDetect * detect = (ScreenDetect*)argument;
	
	bool last = dark;
	for (;;) {
		if (last != dark) {
			last = dark;
			
			if (dark) {
				indicator_pulse_off();
				ruart_write(Commands::DISPLAY_DATA, index, timestamp, 0);
				printd("dark\n");
			} else {
				indicator_pulse_on();
				ruart_write(Commands::DISPLAY_DATA, index, timestamp, 1);
				printd("light\n");
			}		
		}
	}
	
}