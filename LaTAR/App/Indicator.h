#ifndef APP_INDICATOR
#define APP_INDICATOR				  

#include <stm32f4xx_hal.h>

#include "FreeRTOS.h"
#include "semphr.h"



void indicator_init();
void indicator_set_flash();
void indicator_set_idle();
void indicator_pulse_on();
void indicator_pulse_off();

#endif