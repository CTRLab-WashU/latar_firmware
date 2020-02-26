#ifndef COMMUNICATION_WRAPPED_BUFFER
#define COMMUNICATION_WRAPPED_BUFFER				  

#include <cstring>
#include <array>

struct wrapped_buffer
{
//	char data[64];
	std::array<char, 64> data;	
	
	wrapped_buffer(const char * buffer)
	{
		int size = strlen(buffer);
		for (int i = 0; i < 64; i++) {
			if (i < size) {
				data[i] = buffer[i];
			} else {
				data[i] = 0;			
			}
		}
	}
	
	wrapped_buffer()
	{
		for (int i=0; i<64; i++) {
			data[i] = 0;
		}
	}
	
	int size()
	{
		for (int i = 0; i < 64; i++) {
			if (data[i] == 0) {
				return i;
			}
		}
		return 64;
	}

};

#endif