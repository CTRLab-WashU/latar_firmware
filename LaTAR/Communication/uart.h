#ifndef COMMUNICATION_UART
#define COMMUNICATION_UART				  

#include <stm32f4xx_hal.h>
#include <Communication/wrapped_buffer.h>

void uart_init();
void uart_tx(wrapped_buffer buffer);
//void uart_write(wrapped_buffer buffer);
//void uart_write(const char * buffer);
void uart_register_callback(void(*process)(uint8_t byte));

#endif