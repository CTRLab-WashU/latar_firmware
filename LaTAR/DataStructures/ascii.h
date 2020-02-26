#ifndef ASCII_H
#define ASCII_H

#include <cstdint>

struct ascii
{
	static const uint8_t soh = 0x01; // start of heading
	static const uint8_t stx = 0x02; // start of text
	static const uint8_t etx = 0x03; // end of text
	static const uint8_t eot = 0x04; // end of transmission
	static const uint8_t enq = 0x05; // enquiry
	static const uint8_t ack = 0x06; // acknowledgment
	static const uint8_t bel = 0x07; // bell
									 
	static const uint8_t bs  = 0x08; // backspace
	static const uint8_t ht  = 0x09; // horizontal tab
	static const uint8_t lf  = 0x0A; // line feed
	static const uint8_t vt  = 0x0B; // vertical tab
	static const uint8_t ff  = 0x0C; // form feed
	static const uint8_t cr  = 0x0D; // carriage return
	static const uint8_t so  = 0x0E; // shift out /x-on
	static const uint8_t si  = 0x0F; // shift in / x-off
	static const uint8_t dle = 0x10; // data line escape
	static const uint8_t dc1 = 0x11; // device control 1
	static const uint8_t dc2 = 0x12; // device control 2
	static const uint8_t dc3 = 0x13; // device control 3
	static const uint8_t dc4 = 0x14; // device control 4
	static const uint8_t nak = 0x15; // negative acknowledgement
	static const uint8_t syn = 0x16; // synchronous idle
	static const uint8_t etb = 0x17; // end of transmit block
};

#endif