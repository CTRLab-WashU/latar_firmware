#include "App.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "config.h"
#include "Communication/ruart.h"

#include "Indicator.h"
#include "ScreenTouch.h"
#include "ScreenDetect.h"
#include "Commands.h"
#include "SyncTimer.h"

void App::init()
{		
	ScreenDetect::get().init();
	ScreenTouch::get().init();
	
	// make sure we have flashing lights
	indicator_init();
	
	// setup uart to receive commands
	ruart_register_callback(commandReceived);
	ruart_init();	
	
//	osThreadDef(app_thread, thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
//	osThreadCreate(osThread(app_thread), (void*)this);
}	

void App::commandReceived(RuartMsg &message)
{
	switch (message.command)
	{
	
	case Commands::APP_RESET:
		printd("app reset\n");	
		ScreenDetect::get().disable();
		ScreenTouch::get().disable(0);
		ScreenTouch::get().disable(1);
		indicator_set_flash();
		return;
		
	case Commands::DEVICE_INFO:
		printd("device info\n");	
		ScreenDetect::get().disable();
		ScreenTouch::get().disable(0);
		ScreenTouch::get().disable(1);
		indicator_set_flash();
		ruart_write(Commands::DEVICE_INFO);
		return;
		
	case Commands::DISPLAY_START:
		printd("display start\n");
		ScreenDetect::get().enable();
		ruart_write(Commands::DISPLAY_START);
		return;
		
	case Commands::DISPLAY_STOP:
		ScreenDetect::get().disable();
		printd("display stop\n");
		ruart_write(Commands::DISPLAY_STOP);
		return;
		
	case Commands::TAP_START:
		printd("tap start\n");
		ScreenTouch::get().runTapSequence(message.buffer);
		return;
		
	default:
		//ScreenTouch::get().runTapSequence(100,500,1);
		return;
		
	}
}

void App::syncTimeout()
{
	
}

// useful for quick tests
void App::thread(void const * argument)
{	
	App * app = (App*)argument;
		
	const portTickType delay = (5000 / portTICK_RATE_MS);        // 5 second delay
	portTickType prev_wake = xTaskGetTickCount();
	
	uint32_t timestamps[32];
	uint8_t index = 0;
	
	SyncTimer::get().reset();
	
	for(;;) {
		if (index < 32)
		{
			timestamps[index] = SyncTimer::get().getTimestamp();
			vTaskDelayUntil(&prev_wake, delay);
			index++;
		}
		else if (index == 32)
		{
			for (uint8_t i=0;i<32;i++) {
				printf("timestamp = %d\n", timestamps[i]);			
			}
			
			for (uint8_t i = 1; i < 32; i++) {
				uint32_t delta = timestamps[i] - timestamps[i-1];
				printf("delta = %d\n", delta);			
			}
			
			index++;
		}

	}
	
}