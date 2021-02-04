#include "Solenoid.h"

#include "System/FirFilter.h"
#include "System/ADC.h"
#include "cmsis_os.h"
#include "config.h"

static uint32_t last_filtered;
static Filter::Fir filter;	
static Armature state;

static bool initialized;
static bool enabled;
	
static void(*callback)(void);


void Solenoid::sense_irq_handler(uint32_t value)
{
	filter.enqueue(value);
	if (filter.primed()) {
		update(filter.value());
	}
}

void Solenoid::init()
{	
	if (initialized) {
		return;
	}
	initialized = true;
	enabled = false;
	
	state = Armature::Retracted;
	last_filtered = 0;
	callback = 0;
	
	filter.resize(32);
	
	initDrive();
	initSense();
}	

void Solenoid::initDrive()
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OC_InitTypeDef sTimConfig;
	
	// init gpio
	__GPIOE_CLK_ENABLE();
	__TIM1_CLK_ENABLE();
	
	GPIO_InitStructure.Mode			= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate	= GPIO_AF1_TIM1;
	GPIO_InitStructure.Speed		= GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull			= GPIO_PULLUP;
	GPIO_InitStructure.Pin			= GPIO_PIN_9;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	// init timer to drive adc conversions
	drive_tim_handle.Instance			= TIM1;
	drive_tim_handle.Init.Prescaler		= 42 - 1;
	drive_tim_handle.Init.Period		= 80 - 1; 
	drive_tim_handle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	drive_tim_handle.Init.CounterMode	= TIM_COUNTERMODE_UP;
	
	// commit timer init
	if(HAL_TIM_PWM_Init(&drive_tim_handle) != HAL_OK) {
		printd("failed to init pwm timer");
	}
	
	// init driver settings
	TIM_OC_InitTypeDef tim_config;
	tim_config.OCMode		= TIM_OCMODE_PWM1;
	tim_config.OCPolarity	= TIM_OCPOLARITY_HIGH;
	tim_config.OCFastMode	= TIM_OCFAST_DISABLE;
	tim_config.Pulse		= 80 - 1;
	
	// commit driver settings
	if(HAL_TIM_PWM_ConfigChannel(&drive_tim_handle, &tim_config, TIM_CHANNEL_1) != HAL_OK) {
		printd("failed to commit pwm config");
	}
	
}

void Solenoid::initSense()
{	
	GPIO_InitTypeDef gpio_config;
	
	// init gpio
	__GPIOB_CLK_ENABLE();
	gpio_config.Mode		= GPIO_MODE_ANALOG;
	gpio_config.Speed		= GPIO_SPEED_HIGH;
	gpio_config.Pull		= GPIO_NOPULL;
	gpio_config.Pin			= GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &gpio_config);
	
	// init timer to drive adc conversions
	__TIM8_CLK_ENABLE();
	sense_tim_handle.Instance			= TIM8;
	sense_tim_handle.Init.Prescaler		= 84 - 1;
	sense_tim_handle.Init.Period		= 500 - 1;
	sense_tim_handle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	sense_tim_handle.Init.CounterMode	= TIM_COUNTERMODE_UP;
	
	// commit timer config
	if (HAL_TIM_Base_Init(&sense_tim_handle) != HAL_OK) {
		printd("solenoid failed to init sense timer");
	}

	// Init TIM1 TRGO to start ADC conversion
	TIM_MasterConfigTypeDef tim_config;
	tim_config.MasterOutputTrigger = TIM_TRGO_UPDATE;
	tim_config.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&sense_tim_handle, &tim_config);

	// init adc
	__ADC1_CLK_ENABLE();
	sense_adc_handle.Instance					= ADC1;
	sense_adc_handle.Init.ClockPrescaler		= ADC_CLOCKPRESCALER_PCLK_DIV2;
	sense_adc_handle.Init.Resolution			= ADC_RESOLUTION_12B;
	sense_adc_handle.Init.ScanConvMode			= DISABLE;
	sense_adc_handle.Init.ContinuousConvMode	= ENABLE;
	sense_adc_handle.Init.DiscontinuousConvMode	= DISABLE;
	sense_adc_handle.Init.NbrOfDiscConversion	= 0;
	sense_adc_handle.Init.DataAlign				= ADC_DATAALIGN_RIGHT;
	sense_adc_handle.Init.NbrOfConversion		= 1;
	sense_adc_handle.Init.DMAContinuousRequests	= DISABLE;
	sense_adc_handle.Init.EOCSelection			= ADC_EOC_SINGLE_SEQ_CONV;
     
	// commit adc config
	if (HAL_ADC_Init(&sense_adc_handle) != HAL_OK) {
		printd("solenoid failed to init sense adc");
	}
	
	// Configure ADC channel
	ADC_ChannelConfTypeDef adc_channel;
	adc_channel.Channel			= ADC_CHANNEL_8;
	adc_channel.Rank			= 1;
	adc_channel.SamplingTime	= ADC_SAMPLETIME_480CYCLES;
	adc_channel.Offset			= 0;
  
	if (HAL_ADC_ConfigChannel(&sense_adc_handle, &adc_channel) != HAL_OK) {
		printd("solenoid failed to configure sense adc");
	}
	
	HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
	
	if (HAL_ADC_Start_IT(&sense_adc_handle) != HAL_OK) {
		printd("solenoid failed to start sense adc");
	}

	register_adc_callback(ADC1, &sense_adc_handle, sense_irq_handler);
	
}

void Solenoid::update(uint32_t value)
{			
	if (!enabled) {
		return;
	}
	
	switch (state)
	{
	case Armature::Retracted :
		if (value > 2000) {
			state = Armature::Energizing;
		}
		break;	
	case Armature::Energizing :
		if (value < last_filtered) {
			state = Armature::Extending;
		}
		break;	
	case Armature::Extending :
		if (value > last_filtered) {
			state = Armature::Extended;
			if (callback) {
				callback();
			}
		}
		break;	
	case Armature::Extended :
		if (value < 2000) {
			state = Armature::Retracted;
		}
		break;
	}
	
	last_filtered = value;
}

void Solenoid::enable()
{
	HAL_TIM_Base_Start(&sense_tim_handle);
	enabled = true;
}

void Solenoid::disable()
{
	enabled = false;
	HAL_TIM_Base_Stop(&sense_tim_handle);
}

void Solenoid::extend()
{
	HAL_TIM_PWM_Start(&drive_tim_handle, TIM_CHANNEL_1);
}

void Solenoid::retract()
{
	HAL_TIM_PWM_Stop(&drive_tim_handle, TIM_CHANNEL_1);
}

void Solenoid::bind(void(*extended_callback)(void))
{
	callback = extended_callback;
}

void Solenoid::unbind()
{
	callback = 0;
}