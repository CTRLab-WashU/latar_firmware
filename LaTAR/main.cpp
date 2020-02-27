#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#include "FreeRTOS.h"
#include "task.h"

#include "App/App.h"
#include "config.h"

static void StarvationThread(void const * argument);
static void SystemClock_Config(void);

int main(void)
{
	// hardware init
	HAL_Init();  
	SystemClock_Config();
	
	// log version info
	printd("hardware revision B1\n");
	printd("firmware revision 0.0.1\n\n");
	printd("using freertos %s \n\n", tskKERNEL_VERSION_NUMBER);
	
	// init starvation thread
	osThreadDef(starvation, StarvationThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(starvation), NULL);
	
	// init the actual app
	App::get().init();

	// kick start the kernel
	osKernelStart();
	
	// we should never get here as control is now taken by the scheduler
	for (;;) ;
	
}

// steady blinking led on the main board
// if the led has inconsistant timing, we're overworking the mcu
// if the led has stalled, we've deadlocked
static void StarvationThread(void const * argument)
{
	const portTickType delay = (1000 / portTICK_RATE_MS); // 1 second delay
	
	Led led(GPIOB, GPIO_PIN_9);
	led.init();

	for (;;) {
		led.toggle();
		vTaskDelay(delay);
	}
}

static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 24;
	RCC_OscInitStruct.PLL.PLLN = 144;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		printf("failed to config osc\n");
	}
  
	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		printf("failed to config clock\n");
	}
	
}


// standard systick handler
extern "C" void SysTick_Handler(void)
{
	HAL_IncTick();
	osSystickHandler();
}
