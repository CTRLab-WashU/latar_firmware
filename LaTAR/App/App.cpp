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



uint32_t App::parseUnsignedInt(RxBuffer &buffer)
{
	static char conversion_buffer[32] = { 0 };
	static uint8_t byte;
	static int index;

	index = 0;

	while (true) {
		byte = buffer.peek();
		if (byte == 0 || byte == ',' || buffer.isEmpty()) {
			index--;
			break;
		}
		conversion_buffer[index] = buffer.dequeue();
		index++;
	}

	uint32_t value = strtoul(conversion_buffer, NULL, 0);

	for (index; index >= 0; index--) {
		conversion_buffer[index] = 0;
	}

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
	
	ScreenTouch::get().tapSequence(TouchType::SolenoidTouch, 10, 500);
	
	for(;;) {
		vTaskDelay(delay);
	}
	
}