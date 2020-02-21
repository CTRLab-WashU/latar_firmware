#ifndef APP_COMMANDS_H
#define APP_COMMANDS_H	

enum Commands
{
	CLOCK_SYNC        = 0x42,  // init clock synchronzation
	CLOCK_UPDATE      = 0x43,  // data for clock synchronzation
	DEVICE_INFO       = 0x44,  // device information for reference
	DEVICE_IDENTIFY   = 0x45,  // make device self identify for user
	
	APP_RESET		  = 0x49,
	
	DISPLAY_START     = 0x4D,
	DISPLAY_DATA      = 0x4E,
	DISPLAY_STOP      = 0x4F,
	
	TAP_START         = 0x51,
	TAP_DATA          = 0x52,
	TAP_STOP          = 0x55,
	
};



#endif