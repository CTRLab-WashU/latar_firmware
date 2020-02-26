#include "ScreenTouch.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "Commands.h"
#include "Communication/ruart.h"
#include "SyncTimer.h"
#include "Indicator.h"

osSemaphoreId touch_semaphore;

void ScreenTouch::init()
{		
	// init gpio
	relay.init(GPIOB, GPIO_PIN_4, GPIO_PULLUP);
	
	switches[0].init(GPIOD, GPIO_PIN_12, GPIO_PULLUP);
	switches[1].init(GPIOD, GPIO_PIN_13, GPIO_PULLUP);
	switches[2].init(GPIOD, GPIO_PIN_14, GPIO_PULLUP);
	switches[3].init(GPIOD, GPIO_PIN_15, GPIO_PULLUP);
	switches[4].init(GPIOC, GPIO_PIN_7, GPIO_PULLUP);
	
	initPwm();
	
	// init threading
	osThreadDef(screen_touch_thread, thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(screen_touch_thread), (void*)this);
	
	osSemaphoreDef(touch_semaphore);
	touch_semaphore = osSemaphoreCreate(osSemaphore(touch_semaphore), 1);
	
	enabled = false;
}	

void ScreenTouch::initPwm()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OC_InitTypeDef sTimConfig;
	
	// Enable clocks
	__GPIOE_CLK_ENABLE();
	__TIM1_CLK_ENABLE();
	
	// Init GPIO
	GPIO_InitStructure.Mode			= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate	= GPIO_AF1_TIM1;
	GPIO_InitStructure.Speed		= GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull			= GPIO_PULLDOWN;
	
	// Power output 1 PD12
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	// Init Timer
	// Init Timer 1 to drive ADC1 conversions
	pwm_handle.Instance				= TIM1;
	pwm_handle.Init.Prescaler		= 42 - 1;
	pwm_handle.Init.Period			= 90 - 1;
	pwm_handle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	pwm_handle.Init.CounterMode		= TIM_COUNTERMODE_UP;
	
	// Commit timer init
	if(HAL_TIM_PWM_Init(&pwm_handle) != HAL_OK)
	{
		printf("failed to init pwm timer");
	}
	
	// Init CC1 
	sTimConfig.OCMode				= TIM_OCMODE_PWM1;
	sTimConfig.OCPolarity			= TIM_OCPOLARITY_HIGH;
	sTimConfig.OCFastMode			= TIM_OCFAST_DISABLE;
	sTimConfig.Pulse				= 80;
	
	// Commit PWM out settings
	if(HAL_TIM_PWM_ConfigChannel(&pwm_handle, &sTimConfig, TIM_CHANNEL_1) != HAL_OK)
	{
		printf("failed to commit pwm config");
	}
}


void ScreenTouch::enableCapacitiveTouch()
{
	enabled = true;
//	for (int i = 0; i < 5; i++) {
//		switches[i].set();
//	}
}

void ScreenTouch::disableCapacitiveTouch()
{
	enabled = false;
	relay.reset();
	for (int i = 0; i < 5; i++) {
		switches[i].reset();		
	}
}

void ScreenTouch::enableSolenoidTouch(void)
{
	enabled = true;
}

void ScreenTouch::disableSolenoidTouch(void)
{
	enabled = false;
}
	
void ScreenTouch::enable(uint8_t type)
{
	switch (type)
	{
	case 0:
		enableSolenoidTouch();
		break;
	case 1:
		enableCapacitiveTouch();
		break;
	default:
		break;
	}
}

void ScreenTouch::disable(uint8_t type)
{
	switch (type)
	{
	case 0:
		disableSolenoidTouch();
		break;
	case 1:
		disableCapacitiveTouch();
		break;
	default:
		break;
	}
}

void ScreenTouch::tapCapacitive(TickType_t duration)
{
	printf("tapping capacitive\n");
	indicator_pulse_on();
	relay.set();
	vTaskDelay(duration);
	relay.reset();
	indicator_pulse_off();
}

void ScreenTouch::tapSolenoid(TickType_t duration)
{
	printf("tapping solenoid\n");
	indicator_pulse_on();
	HAL_TIM_PWM_Start(&pwm_handle, TIM_CHANNEL_1);
	vTaskDelay(duration);
	HAL_TIM_PWM_Stop(&pwm_handle, TIM_CHANNEL_1);
	indicator_pulse_off();
}

void ScreenTouch::tap(uint8_t type, TickType_t duration)
{
	switch (type)
	{
	case 0:
		tapSolenoid(duration);
		break;
	case 1:
		tapCapacitive(duration);
		break;
	default:
		break;
	}
}

void ScreenTouch::extendSolenoid()
{
	HAL_TIM_PWM_Start(&pwm_handle, TIM_CHANNEL_1);
}

void ScreenTouch::retractSolenoid()
{
	HAL_TIM_PWM_Stop(&pwm_handle, TIM_CHANNEL_1);
}

void ScreenTouch::runTapSequence(uint32_t count, uint32_t interval, uint8_t type)
{
	params.count = count;
	params.interval = interval;
	params.type = type;
	enable(params.type);
	osSemaphoreRelease(touch_semaphore);
}

RingBuffer<char,64> conversion_buffer;

void ScreenTouch::runTapSequence(RxBuffer &buffer)
{
	while (buffer.peek() != ',' && buffer.peek() != 0)
	{
		conversion_buffer.enqueue(buffer.dequeue());
	}
	params.count = strtoul(conversion_buffer.getRawBuffer().data(), NULL, 0);
	conversion_buffer.clear();
	buffer.dequeue();
	
	while (buffer.peek() != ',' && buffer.peek() != 0)
	{
		conversion_buffer.enqueue(buffer.dequeue());
	}
	params.interval = strtoul(conversion_buffer.getRawBuffer().data(), NULL, 0);
	conversion_buffer.clear();
	buffer.dequeue();
	
	while (buffer.peek() != ',' && buffer.peek() != 0)
	{
		conversion_buffer.enqueue(buffer.dequeue());
	}
	params.type = strtoul(conversion_buffer.getRawBuffer().data(), NULL, 0);
	conversion_buffer.clear();
	buffer.clear();
	
	enable(params.type);
	osSemaphoreRelease(touch_semaphore);

}

char touch_tx_buffer[64];

void ScreenTouch::sendData(uint32_t index, uint32_t timestamp)
{
	sprintf(touch_tx_buffer, "%d,%d", index, timestamp);
	ruart_write(Commands::TAP_DATA, touch_tx_buffer);
}

void ScreenTouch::thread(void const * argument)
{	
	ScreenTouch * touch = (ScreenTouch*)argument;
	
	portTickType tap_delay = (150 / portTICK_RATE_MS);  
	portTickType interval_delay = (500 / portTICK_RATE_MS);  
	RunParameters params;
	uint32_t timestamp;
		
	// ------------------------------------------------------------------------
	const portTickType detect_delay = (500 / portTICK_RATE_MS);       // 500 ms delay
	
	for(;;) {
		if (osSemaphoreWait(touch_semaphore, osWaitForever) == osOK) {
			if (!touch->enabled) {
				continue;
			}
			indicator_pulse_off();
			params = touch->params;
			touch->enable(params.type);
			
			interval_delay = (params.interval / portTICK_RATE_MS);  
			portTickType prev_wake = xTaskGetTickCount();
			
			for (int i = 0; i < touch->params.count; i++) {
				timestamp = SyncTimer::get().getTimestamp();
				touch->tap(params.type, tap_delay);
				
				touch->sendData(i, timestamp);
				vTaskDelayUntil(&prev_wake, interval_delay);
			}
			indicator_set_flash();
			touch->disable(touch->params.type);
			params.count = 0;
			params.interval = 0;
			
			ruart_write(Commands::TAP_STOP);
			printf("tap stop\n");
		}
	}
	
}

