#ifndef FILTER_FIR
#define FILTER_FIR				  

#include <stdint.h>

namespace Filter
{
		 
class Fir
{
public:
	Fir(uint8_t size=1);
	~Fir();
	
	bool resize(uint8_t size);
	
	void enqueue(uint32_t value);
	uint32_t value();
	
	void clear();
	bool primed();
	
private:
	bool init;
	uint8_t index;
	uint8_t size;
	
	uint32_t temp;
	uint32_t val;
	
};

}

#endif