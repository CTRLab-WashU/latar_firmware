#include "FirFilter.h"

using namespace Filter;


Fir::Fir(uint8_t size)
{	
	resize(size);
}

Fir::~Fir()
{	
	
}	

bool Fir::resize(uint8_t size)
{
	clear();
	this->size = size;
	return size!=0;
}

void Fir::enqueue(uint32_t value)
{
	index++;
	temp += value / size;
	
	if (index == size) {
		init = false;
		index = 0;
		val = temp;
		temp = 0;
	}
}

uint32_t Fir::value()
{
	return val;
}

void Fir::clear()
{
	index = 0;
	temp = 0;
	val = 0;
	init = true;
}

bool Fir::primed()
{
	return !init;
}