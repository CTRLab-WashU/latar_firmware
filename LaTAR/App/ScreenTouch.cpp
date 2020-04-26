#include "ScreenTouch.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "config.h"
#include "Commands.h"
#include "Communication/wrapped_buffer.h"
#include "Communication/ruart.h"
#include "SyncTimer.h"
#include "Indicator.h"

static portTickType tap_delay = (150 / portTICK_RATE_MS);  
osSemaphoreId touch_semaphore;
static uint32_t index;

void ScreenTouch::init()
{		
	calibrating = false;
	enabled = false;
	index = 0;
	
	// init gpio
	relay.init(GPIOB, GPIO_PIN_4, GPIO_PULLUP);
	
	switches[0].init(GPIOD, GPIO_PIN_12, GPIO_PULLUP);
	switches[1].init(GPIOD, GPIO_PIN_13, GPIO_PULLUP);
	switches[2].init(GPIOD, GPIO_PIN_14, GPIO_PULLUP);
	switches[3].init(GPIOD, GPIO_PIN_15, GPIO_PULLUP);
	switches[4].init(GPIOC, GPIO_PIN_7, GPIO_PULLUP);
	
	initPwm();
	
	// init threading
	osThreadDef(screen_touch_thread, thread, osPriorityNormal, 0, 512);
	osThreadCreate(osThread(screen_touch_thread), (void*)this);
	
	osSemaphoreDef(touch_semaphore);
	touch_semaphore = osSemaphoreCreate(osSemaphore(touch_semaphore), 1);
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
	GPIO_InitStructure.Pull			= GPIO_PULLUP;
	
	// Power output 1 PD12
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	// Init Timer
	// Init Timer 1 to drive ADC1 conversions
	pwm_handle.Instance				= TIM1;
	pwm_handle.Init.Prescaler		= 42 - 1;
	pwm_handle.Init.Period			= 80 - 1; 
	pwm_handle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	pwm_handle.Init.CounterMode		= TIM_COUNTERMODE_UP;
	
	// Commit timer init
	if(HAL_TIM_PWM_Init(&pwm_handle) != HAL_OK)
	{
		printd("failed to init pwm timer");
	}
	
	// Init CC1 
	sTimConfig.OCMode				= TIM_OCMODE_PWM1;
	sTimConfig.OCPolarity			= TIM_OCPOLARITY_HIGH;
	sTimConfig.OCFastMode			= TIM_OCFAST_DISABLE;
	sTimConfig.Pulse				= 60-1;
	
	// Commit PWM out settings
	if(HAL_TIM_PWM_ConfigChannel(&pwm_handle, &sTimConfig, TIM_CHANNEL_1) != HAL_OK)
	{
		printd("failed to commit pwm config");
	}
}


void ScreenTouch::enableCapacitiveTouch()
{
	enabled = true;
}

void ScreenTouch::setCapacitance(int setting)
{
	if (setting < 0) {
		setting = 0;
	}
	if (setting > 6) {
		setting = 6;
	}
	
	for (int i = 0; i <6; i++) {
		if (i<setting) {
			switches[i].set();		
		}
		else {
			switches[i].reset();		
		}
	}
}

void ScreenTouch::disableCapacitiveTouch()
{
	enabled = false;
	relay.reset();
	for (int i = 0; i < 5; i++) {
		switches[i].reset();		
	}
}

void ScreenTouch::enable(TouchType type)
{
	index = 0;
	
void ScreenTouch::enable(TouchType type)
{
	switch (type)
	{
	case TouchType::SolenoidTouch:
		solenoid.enable();
		break;
	case TouchType::CapacitiveTouch:
		enableCapacitiveTouch();
		break;
	default:
		break;
	}
	
	enabled = true;
}

void ScreenTouch::disable(TouchType type)
{
	enabled = false;
	
	switch (type)
	{
	case TouchType::SolenoidTouch:
		solenoid.disable();
		break;
	case TouchType::CapacitiveTouch:
		disableCapacitiveTouch();
		break;
	default:
		break;
	}
}

void ScreenTouch::tapCapacitive(TickType_t duration)
{
	printd("tapping capacitive\n");
	indicator_pulse_on();
	relay.set();
	vTaskDelay(duration);
	relay.reset();
	indicator_pulse_off();
}

void ScreenTouch::tapSolenoid(TickType_t duration)
{
	printd("tapping solenoid\n");
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
	case SolenoidTouch:
		tapSolenoid(duration);
		break;
	case CapacitiveTouch:
		tapCapacitive(duration);
		break;
	default:
		break;
	}
}

void ScreenTouch::tapSequence(TouchType type, uint32_t count, uint32_t interval, uint8_t cap)
{
	params.interval = interval;
	params.count = count;
	params.type = type;
	params.cap = cap;
	
	enable(params.type);
	osSemaphoreRelease(touch_semaphore);
}

void ScreenTouch::sendData(uint32_t index, uint32_t timestamp)
{
	ruart_write(Commands::TAP_DATA, index, timestamp);
}
void ScreenTouch::calibrate()
{
	calibrating = true;	
	enabled = true;
	osSemaphoreRelease(touch_semaphore);
}

void ScreenTouch::solenoidCallback(void)
{
	ruart_write(Commands::TAP_DATA, index, SyncTimer::get().getTimestamp());
	index++;
	printd("tapped\n");
}

void ScreenTouch::thread(void const * argument)
{	
	ScreenTouch * touch = (ScreenTouch*)argument;
		
	for(;;) {
		if (osSemaphoreWait(touch_semaphore, osWaitForever) == osOK) {
			if (!touch->enabled) {
				continue;
			}
			
			indicator_pulse_off();
			
			if (touch->calibrating) {
				calibrationRun(touch);
			} else {
				normalRun(touch);
			}
			
		}
	}
	
}

void ScreenTouch::normalRun(ScreenTouch * touch)
{	
	printd("starting normal tap run\n");
	
	RunParameters params = touch->params;
	portTickType interval_delay = (params.interval / portTICK_RATE_MS);  
	portTickType prev_wake = xTaskGetTickCount();
	uint32_t timestamp;
	
	touch->enable(params.type);
	touch->setCapacitance(params.cap);
	
	for (int i = 0; i < params.count; i++) {
		timestamp = SyncTimer::get().getTimestamp();
		touch->tap(params.type, tap_delay);
		touch->sendData(i, timestamp);
		vTaskDelayUntil(&prev_wake, interval_delay);
	}
	
	touch->disable(params.type);
	ruart_write(Commands::TAP_STOP);
	indicator_set_flash();
	
	printd("stopping normal tap run\n");
}

void ScreenTouch::calibrationRun(ScreenTouch * touch)
{	
	printd("starting touch calibraion run\n");
	
	portTickType interval_delay = (500 / portTICK_RATE_MS);  
	portTickType prev_wake = xTaskGetTickCount();
	
	touch->enable(TouchType::CapacitiveTouch);
	
	for (int i=0; i<6; i++) {
		touch->setCapacitance(i);
		vTaskDelayUntil(&prev_wake, interval_delay);
		vTaskDelayUntil(&prev_wake, interval_delay);
		for (int j = 0; j < 5; j++) {
			touch->tap(1, tap_delay);
			vTaskDelayUntil(&prev_wake, interval_delay);
		}
		vTaskDelayUntil(&prev_wake, interval_delay);
		vTaskDelayUntil(&prev_wake, interval_delay);
	}

	touch->disable(TouchType::CapacitiveTouch);
	touch->calibrating = false;
	touch->enabled = false;
	ruart_write(Commands::CALIBRATION_TOUCH_STOP);
	indicator_set_flash();
	
	printd("stopping touch calibraion run\n");
}

