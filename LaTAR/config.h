#ifndef HM_CONFIG
#define HM_CONFIG		

#include <cstdio>

#define DISABLE_PRINTF 0

#if DISABLE_PRINTF
#define printd(f, ...) (0)
#else
#define printd(f, ...) printf(f,  ## __VA_ARGS__)
#endif	
	
#endif
