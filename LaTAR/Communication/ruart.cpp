// hardware abstractions
#include <stm32f4xx_hal.h>
#include <stm32_hal_legacy.h>
#include "stm32f4xx_hal_uart.h"

// std libraries
#include <functional>
#include <stdio.h>
#include <cstring>

// local libraries
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

// app
#include "DataStructures/RingBuffer.h"
#include "DataStructures/ascii.h"
#include "System/SoftTimer.h"
#include "SyncTimer.h"
#include "Commands.h"

#include "wrapped_buffer.h"
#include "ruart_frame.h"
#include "ruart.h"
#include "uart.h"
#include "config.h"

#include "App/Indicator.h"

static const uint8_t update_offset	    = 0x30;

// ----------------------------------------------------------------------------

namespace ruart
{
	std::function<void(RuartMsg&)> msg_callback;
	RuartFrame frame;
	
	RingBuffer<wrapped_buffer, 32> tx_queue;
	SoftTimer interval_timer;
	ManualTimer tx_timer;

	RuartMsg tx_msg;
	bool tx_busy;
	
	char byte_buffer[1];	
}

char ruart_temp_buffer[32];

void ruart_handle_byte(uint8_t byte);
void ruart_write_byte(uint8_t command);
void ruart_tx_timeout();
void ruart_sync_timeout();

// ----------------------------------------------------------------------------

void ruart_init()
{
	ruart::tx_busy = false;
	ruart::tx_timer.setDuration(1000);
	ruart::interval_timer.init("ruart_tx_timer", 1000, ruart_tx_timeout);
	ruart::interval_timer.start();
	
	SyncTimer::get().init(ruart_sync_timeout);
	
	uart_register_callback(ruart_handle_byte);
	uart_init();
}

void ruart_write(RuartMsg msg)
{	
	wrapped_buffer tx;
	tx.data[0] = frame_start;
	tx.data[1] = msg.size + update_offset;
	tx.data[2] = msg.command;
	
	for (uint8_t i = 1; i < msg.size; i++) {
		tx.data[i + 2] = msg.buffer.dequeue();
	}
	tx.data[msg.size + 2] = frame_stop;
	
	ruart::tx_queue.enqueue(tx);

	if (!ruart::tx_busy) {
		ruart::tx_busy = true;
		ruart::tx_timer.reset();
		uart_tx(ruart::tx_queue.peek());	
	}
}

void ruart_write(uint8_t command)
{
	ruart::tx_msg.buffer.clear();
	ruart::tx_msg.command = command;
	ruart::tx_msg.size = 1;
	
	ruart_write(ruart::tx_msg);
}


void ruart_write_tapdata(uint32_t index, uint32_t timestamp)
{
	sprintf(ruart_temp_buffer, "%d,%d", index, timestamp);
	
	wrapped_buffer tx;
			
	tx.data[0] = frame_start;
	tx.data[1] = 0;
	tx.data[2] = Commands::TAP_DATA;
			
	uint8_t size = 0;
	for (int i = 0; i < 32; i++) {
		if (ruart_temp_buffer[i] != 0) {
			tx.data[i + 3] = ruart_temp_buffer[i];
			size = i;
		}
	}
	tx.data[size + 4] = frame_stop;
	size += (2 + update_offset);
	tx.data[1] = size;
		
	ruart::tx_queue.enqueue(tx);
	
	if (!ruart::tx_busy) {
		ruart::tx_busy = true;
		ruart::tx_timer.reset();
		uart_tx(ruart::tx_queue.peek());	
	}
	
}

void ruart_write_displaydata(uint32_t index, uint32_t timestamp, uint8_t value)
{
	sprintf(ruart_temp_buffer, "%d,%d,%d", index, timestamp, value);
	
	wrapped_buffer tx;
			
	tx.data[0] = frame_start;
	tx.data[1] = 0;
	tx.data[2] = Commands::DISPLAY_DATA;
			
	uint8_t size = 0;
	for (int i = 0; i < 32; i++) {
		if (ruart_temp_buffer[i] != 0) {
			tx.data[i + 3] = ruart_temp_buffer[i];
			size = i;
		}
	}
	tx.data[size + 4] = frame_stop;
	size += (2 + update_offset);
	tx.data[1] = size;
	
	ruart::tx_queue.enqueue(tx);
	
	if (!ruart::tx_busy) {
		ruart::tx_busy = true;
		ruart::tx_timer.reset();
		uart_tx(ruart::tx_queue.peek());	
	}
	
}

void ruart_write_byte(uint8_t command)
{
	ruart::byte_buffer[0] = command;
	uart_tx(ruart::byte_buffer);
}

void ruart_register_callback(std::function<void(RuartMsg&)> callback_function)
{
	ruart::msg_callback = callback_function;
}

// ----------------------------------------------------------------------------
uint32_t ruart_timestamp_t = 0;
bool ruart_syncing = false;

wrapped_buffer st_buffer;

void ruart_handle_byte(uint8_t byte)
{
	
	if (byte == Commands::CLOCK_SYNC)
	{
		SyncTimer::get().reset();
		ruart_write_byte(Commands::CLOCK_SYNC);
		ruart_syncing = true;
		indicator_pulse_on();
		printd("in clock sync mode\n");
		ruart::tx_queue.clear();
		return;
	}
	
	if (ruart_syncing) {
		switch (byte)
		{
		case Commands::CLOCK_UPDATE:
			ruart_syncing = false;
			ruart_write_byte(Commands::CLOCK_UPDATE);
			
			printd("out of clock sync mode\n");
			indicator_set_flash();
			printd("sending queued timestamps\n");
			
			st_buffer.data[0] = frame_start;
			st_buffer.data[1] = 1 + update_offset;
			st_buffer.data[2] = Commands::CLOCK_UPDATE;
			st_buffer.data[3] = frame_stop;
			
			ruart::tx_queue.enqueue(st_buffer);
			
			ruart::tx_timer.reset();
			uart_tx(ruart::tx_queue.peek());
			break;
			
		default :
			ruart_timestamp_t = SyncTimer::get().getTimestamp();
			ruart_write_byte(byte);
			
			printd("%d\n", ruart_timestamp_t);
			
			char stamp_buffer[32];
			for (int i=0;i<32;i++){
				stamp_buffer[i] = 0;
			}
			
			std::snprintf(stamp_buffer, 32, "%u", ruart_timestamp_t);
			
			wrapped_buffer tx;
			
			tx.data[0] = frame_start;
			tx.data[1] = 0;
			tx.data[2] = Commands::CLOCK_DATA;
			tx.data[3] = byte;
			
			uint8_t size = 0;
			for (int i = 0; i < 32; i++) {
				if (stamp_buffer[i]!=0) {
					tx.data[i + 4] = stamp_buffer[i];
					size = i;
				}
			}
			tx.data[size + 5] = frame_stop;
			size += (3 + update_offset);
			tx.data[1] = size;
			
			ruart::tx_queue.enqueue(tx);
			break;
			
		}
		return;
	}
	
	switch (byte)
	{		
	case frame_ack:
		ruart::tx_queue.dequeue();
		if (!ruart::tx_queue.isEmpty()) {
			ruart::tx_timer.reset();
			uart_tx(ruart::tx_queue.peek());
		} else {
			ruart::tx_busy = false;
			ruart::tx_timer.reset();
		}
		break;
		
	case frame_nak:
		ruart::tx_timer.reset();
		uart_tx(ruart::tx_queue.peek());
		break;
		
	case frame_start:
		ruart::frame.start();
		break;
		
	case frame_stop:
		if (ruart::frame.stop(ruart::tx_msg)) {
			if (ruart::msg_callback != 0) {
				ruart::msg_callback(ruart::tx_msg);
			}
			ruart_write_byte(frame_ack);
		} else {
			ruart_write_byte(frame_nak);
		}
		break;
		
	default:
		if (ruart::frame.isActive()) {
			ruart::frame.buffer.enqueue(byte);		
		}
		break;
	}
	
}

void ruart_tx_timeout()
{
	if (!ruart::tx_busy) {
		return;
	}
	
	if (!ruart::tx_timer.expired()) {
		return;
	}
	
	printd("resending last message\n");
	ruart::tx_timer.reset();
	uart_tx(ruart::tx_queue.peek());
}

void ruart_sync_timeout()
{
	// todo: tell the server
}