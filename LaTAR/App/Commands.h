#ifndef APP_COMMANDS_H
#define APP_COMMANDS_H	

enum Commands
{
	CLOCK_SYNC			= 0x21,  // init clock synchronzation
	CLOCK_UPDATE        = 0x22,  // 
	CLOCK_DATA			= 0x23,  // data for clock synchronzation
	
	DEVICE_INFO			= 0x24,  // device information for reference
	DEVICE_IDENTIFY		= 0x25,  // make device self identify for user
	
	APP_ERROR			= 0x26,
	APP_RESET			= 0x27,
	
	DISPLAY_START		= 0x28,
	DISPLAY_DATA		= 0x29,
	DISPLAY_STOP		= 0x2A,
	
	TAP_START			= 0x2B,
	TAP_DATA			= 0x2C,
	TAP_STOP			= 0x2D,
	
};



#endif