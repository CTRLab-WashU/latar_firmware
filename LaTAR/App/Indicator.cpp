#include "Indicator.h"


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "System/ManualTimer.h"
#include "System/Led.h"
#include "cmsis_os.h"


Led led[6];

enum Status {
	Status_Idle,
	Status_Flashing
};

Status status = Status_Flashing;
bool needsReset = false;
uint8_t index = 0;

const portTickType idle_delay = (100 / portTICK_RATE_MS);
const portTickType pulse_delay = (100 / portTICK_RATE_MS);
const portTickType flash_delay = (100 / portTICK_RATE_MS);

static void indicator_thread(void const * argument);

void indicator_init()
{	
	
	led[0] = Led(GPIOA, GPIO_PIN_5);
	led[0].init();
	
	led[1] = Led(GPIOA, GPIO_PIN_4);
	led[1].init();
	
	led[2] = Led(GPIOB, GPIO_PIN_10);
	led[2].init();
	
	led[3] = Led(GPIOB, GPIO_PIN_14);
	led[3].init();
	
	led[4] = Led(GPIOB, GPIO_PIN_15);
	led[4].init();
	
	led[5] = Led(GPIOD, GPIO_PIN_10);
	led[5].init();
	
	osThreadDef(indicator, indicator_thread, osPriorityNormal, 0, 512);
	osThreadCreate(osThread(indicator), NULL);
}

void indicator_set_flash()
{
	status = Status_Flashing;
}

void indicator_set_idle()
{
	status = Status_Idle;
}

void indicator_pulse_on()
{
	status = Status_Idle;
	needsReset = true;
	for (uint8_t i = 0; i < 6; i++) {
		led[i].on();
	}
}

void indicator_pulse_off()
{
	for (uint8_t i = 0; i < 6; i++) {
		led[i].off();
	}
	needsReset = false;
	status = Status_Idle;
}

static void indicator_thread(void const * argument)
{ 
	for (;;) {
		switch (status)
		{
		case Status_Idle:
			if (needsReset) {
				needsReset = false;
				for (uint8_t i = 0; i < 3; i++) {
					led[i].on();
				}
			}
			vTaskDelay(idle_delay);
			break;
		case Status_Flashing:
			needsReset = true;
			index++;
			if (index > 5) {
				index = 0;
			}
			for (uint8_t i=0;i<6;i++) {
				if (i == index) {
					led[i].on();
				} else {
					led[i].off();
				}
			}
			vTaskDelay(flash_delay);
			break;
		}
	}
}
