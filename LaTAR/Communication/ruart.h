#ifndef COMMUNICATION_RELIABLE_UART_H
#define COMMUNICATION_RELIABLE_UART_H				  

#include <stm32f4xx_hal.h>
#include <functional>
#include <string>

#include "DataStructures/RingBuffer.h"
#include "System/ManualTimer.h"
#include "ruart_message.h"

typedef RingBuffer<uint8_t, 32> RxBuffer;

void ruart_init();

void ruart_write(const uint8_t command, char *buffer);
void ruart_write(const uint8_t command, const uint32_t &v1, const uint32_t &v2, const uint32_t &v3);
void ruart_write(const uint8_t command, const uint32_t &v1, const uint32_t &v2);
void ruart_write(const uint8_t command, const uint32_t &v1);
void ruart_write(const uint8_t command);

void ruart_register_callback(std::function<void(RuartMsg&)> callback_function);




#endif