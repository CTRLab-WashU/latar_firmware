#ifndef APP_H
#define APP_H				  

#include <stm32f4xx_hal.h>

#include "System/Led.h"
#include "App/Indicator.h"
#include "Communication/ruart.h"

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
	static void commandReceived(RuartMsg &buffer);
	static void syncTimeout();
	
	static void thread(void const * argument);
};

#endif