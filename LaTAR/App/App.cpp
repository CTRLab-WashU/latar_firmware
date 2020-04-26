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

uint32_t threshold = 0;

TouchType type;
uint32_t interval = 0;
uint32_t count = 0;
uint8_t  cap = 0;

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
		
	case Commands::DEVICE_INFO:
		printd("device info\n");	
		ScreenDetect::get().disable();
		ScreenTouch::get().disable(TouchType::SolenoidTouch);
		ScreenTouch::get().disable(TouchType::CapacitiveTouch);
		indicator_set_flash();
		ruart_write(Commands::DEVICE_INFO);
		return;
		
	case Commands::DISPLAY_START:
		printd("display start\n");
		threshold = parseUnsignedInt(message.buffer);
		ScreenDetect::get().enable(threshold);
		ruart_write(Commands::DISPLAY_START);
		return;
		
	case Commands::DISPLAY_STOP:
		ScreenDetect::get().disable();
		printd("display stop\n");
		ruart_write(Commands::DISPLAY_STOP);
		return;
		
	case Commands::TAP_START:
		printd("tap start\n");
		
		cap = parseUnsignedInt(message.buffer);
		message.buffer.dequeue();
		count = parseUnsignedInt(message.buffer);
		message.buffer.dequeue();
		interval = parseUnsignedInt(message.buffer);
		message.buffer.dequeue();
		type = (TouchType)parseUnsignedInt(message.buffer);

		ScreenTouch::get().tapSequence(type, count, interval, cap);
		return;
		
	case Commands::CALIBRATION_DISPLAY_START:
		printd("start display calibration\n");
		ScreenDetect::get().startCalibration();
		ruart_write(Commands::CALIBRATION_DISPLAY_START);
		return;
		
	case Commands::CALIBRATION_DISPLAY_STOP:
		printd("stop display calibration\n");
		ScreenDetect::get().stopCalibration();
		return;
		
	case Commands::CALIBRATION_TOUCH_START:
		printd("start touch calibration\n");
		ScreenTouch::get().calibrate();
		return;

	default:
		return;
		
	}
}

RingBuffer<char, 32> conversion_buffer;

uint32_t App::parseUnsignedInt(RxBuffer &buffer)
{
	while (buffer.peek() != ',' && buffer.peek() != 0)
	{
		conversion_buffer.enqueue(buffer.dequeue());
	}
	uint32_t value = strtoul(conversion_buffer.getRawBuffer().data(), NULL, 0);
	conversion_buffer.clear();
	
	return value;
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