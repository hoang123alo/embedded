#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "drv_mcu.h"

void Osc_Init (void)
{
  	SFRPAGE   = CONFIG_PAGE;
  	OSCICN    = 0xC7U;
  	SFRPAGE   = ACTIVE_PAGE;
}

void Port_Init (void)
{
    SFRPAGE   = CONFIG_PAGE;
    P0MDOUT   = 0x74;
    P2MDOUT   = 0x0B;
    P0SKIP    = 0x03;
    XBR0      = 0x06;
    XBR2      = 0x40;
    SFRPAGE   = ACTIVE_PAGE;
	LED_STATUS_ERROR = 0x00U;
	LED_STATUS_UPDOWNLOAD = 0x01U;
}

void Exinterrupt_Init(void)
{
	SFRPAGE   = CONFIG_PAGE;
	TCON	  = TCON | 0x05U;		// /INT0 and /INT1 are edge triggered
	//IT01CF	= 0x60U;		// INT0 = 1.7, INT1 = 1.6 Port
	EX1		= 0x01U;		// Enable /INT1 interrupts
	SFRPAGE   = ACTIVE_PAGE;
}