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
void ruart_write(uint8_t command);
void ruart_write_tapdata(uint32_t index, uint32_t timestamp);
void ruart_write_displaydata(uint32_t index, uint32_t timestamp, uint8_t value);
void ruart_register_callback(std::function<void(RuartMsg&)> callback_function);
void ruart_write_displaycalibrationdata(uint32_t min, uint32_t max);

#endif