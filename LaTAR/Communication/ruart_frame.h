#ifndef COMMUNICATION_RELIABLE_UART_FRAME_H
#define COMMUNICATION_RELIABLE_UART_FRAME_H				  

#include <stm32f4xx_hal.h>
#include <functional>
#include <string>

#include "DataStructures/ascii.h"
#include "ruart_message.h"

static const uint8_t frame_ack			= ascii::ack; 
static const uint8_t frame_nak			= ascii::nak; 
static const uint8_t frame_start		= ascii::stx; 
static const uint8_t frame_stop			= ascii::etx;

struct RuartFrame
{
	RxBuffer buffer;
	uint8_t counter = 0;
	bool active = false;
	
	bool isActive()
	{
		return active;
	}
	
	void start()
	{
		buffer.clear();
		active = true;
	}
	
	bool stop(RuartMsg &message)
	{
		active = false;
		
		if (buffer.size() < 2) {
			return false;
		}
		message.buffer.clear();
		message.size = buffer.dequeue();
		message.command = buffer.dequeue();
		counter = 1;
		
		while (!buffer.isEmpty()) {
			message.buffer.enqueue(buffer.dequeue());		
			counter++;
		}
		
		return counter == message.size;
	}
		
};


#endif