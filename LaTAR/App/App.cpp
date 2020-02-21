#include "App.h"

#include <cstring>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"

#include "Indicator.h"
#include "ScreenTouch.h"
#include "ScreenDetect.h"
#include "Communication/uart.h"
#include "Commands.h"
#include "SyncTimer.h"

char clock_buff[32];
char cmd_buff[2];

void App::init()
{		
	clock_buff[0] = Commands::CLOCK_UPDATE;
	cmd_buff[1] = uart_delim;
	
	SyncTimer::get().init();
	
	ScreenDetect::get().init();
	ScreenTouch::get().init();
	
	// make sure we have flashing lights
	indicator_init();
	
	// setup uart to receive commands
	uart_register_callback(commandReceived);
	uart_init();	
}	

void App::commandReceived(RxBuffer &buffer)
{
	uint32_t timestamp = SyncTimer::get().getTimestamp();
	uint8_t command = buffer.dequeue();
	
	switch (command)
	{
		
	case Commands::CLOCK_SYNC:
		SyncTimer::get().reset();
		cmd_buff[0] = Commands::CLOCK_SYNC;
		uart_write(cmd_buff);
    printf("clock sync\n");
		return;
		
	case Commands::CLOCK_UPDATE:
		sprintf(clock_buff, "C%d\n", timestamp);
		uart_write(clock_buff);
    printf("clock update\n");
		return;
		
	case Commands::APP_RESET:
    printf("app reset\n");	
		ScreenDetect::get().disable();
		ScreenTouch::get().disable(0);
		ScreenTouch::get().disable(1);
		indicator_set_flash();
		return;
		
	case Commands::DEVICE_INFO:
    printf("device info\n");	
		ScreenDetect::get().disable();
		ScreenTouch::get().disable(0);
		ScreenTouch::get().disable(1);
		indicator_set_flash();
		cmd_buff[0] = Commands::DEVICE_INFO;
		uart_write(cmd_buff);
		return;
		
	case Commands::DISPLAY_START:
    printf("display start\n");
		ScreenDetect::get().enable();
		cmd_buff[0] = Commands::DISPLAY_START;
		uart_write(cmd_buff);
		return;
		
	case Commands::DISPLAY_STOP:
		ScreenDetect::get().disable();
    printf("display stop\n");
		cmd_buff[0] = Commands::DISPLAY_STOP;
		uart_write(cmd_buff);
		return;
		
	case Commands::TAP_START:
    printf("tap start\n");
		ScreenTouch::get().runTapSequence(buffer);
		return;
		
	default:
		//ScreenTouch::get().runTapSequence(100,500,1);
		return;
		
	}
}

void App::syncTimeout()
{
	
}
