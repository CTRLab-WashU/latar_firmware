#ifndef COMMUNICATION_RELIABLE_UART_MESSAGE_H
#define COMMUNICATION_RELIABLE_UART_MESSAGE_H				  

#include <stm32f4xx_hal.h>

#include "DataStructures/RingBuffer.h"

typedef RingBuffer<uint8_t, 32> RxBuffer;

struct RuartMsg
{
	uint8_t size;
	uint8_t command;
	RxBuffer buffer;
};

#endif