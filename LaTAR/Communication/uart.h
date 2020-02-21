#ifndef COMMUNICATION_UART
#define COMMUNICATION_UART				  

#include <stm32f4xx_hal.h>
#include <functional>
#include <string>

#include "DataStructures/RingBuffer.h"

typedef RingBuffer<uint8_t, 32> RxBuffer;
static const uint8_t uart_delim = '\n';


void uart_init();
void uart_write(const char * buffer);
void uart_register_callback(std::function<void(RxBuffer&)> callback_function);

#endif