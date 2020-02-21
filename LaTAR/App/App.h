#ifndef APP_H
#define APP_H				  

#include <stm32f4xx_hal.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "System/Led.h"
#include "App/Indicator.h"
#include "Communication/uart.h"

class App
{
	App(){}
public:
	static App& get() {
		static App instance;
		return instance;
	}
		
	void init();
		
private:
	static void commandReceived(RxBuffer &buffer);
	static void syncTimeout();
	
};

#endif