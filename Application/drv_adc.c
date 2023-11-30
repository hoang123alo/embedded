#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_mcu.h"
#include "drv_timer.h"
#include "drv_adc.h"

          // Index of analog MUX inputs
// Integrate accumulator for the ADC samples from input pins
// ADC0 decimated value, one for each analog input
SEG_XDATA U8 Temperature;
SEG_XDATA U16 RESULT;

void ADC_Init(void)
{
	U8 SFRPAGE_save = (U8)SFRPAGE;

	RESULT = 0x0000U;
	SFRPAGE = ACTIVE_PAGE;

	// Initialize the Gain to account for a 5V input and 2.25 VREF
	// Solve the equation provided in Section 9.3.1 of the Datasheet

	// The 5V input is scaled by a factor of 0.44 so that the maximum input
	// voltage seen by the pin is 2.2V

	// 0.44 = (GAIN/4096) + GAINADD * (1/64)

	// Set GAIN to 0x6CA and GAINADD to 1
	// GAIN = is the 12-bit word formed by ADC0GNH[7:0] ADC0GNL[7:4]
	// GAINADD is bit ADC0GNA.0
/*
	ADC0CF |= 0x01;                     // Set GAINEN = 1
	ADC0H   = 0x04;                     // Load the ADC0GNH address
	ADC0L   = 0x6C;                     // Load the upper byte of 0x6CA
	ADC0H   = 0x07;                     // Load the ADC0GNL address
	ADC0L   = 0xA0;                     // Load the lower nibble of 0x6CA
	ADC0H   = 0x08;                     // Load the ADC0GNA address
	ADC0L   = 0x01;                     // Set the GAINADD bit
	ADC0CF &= ~0x01;                    // Set GAINEN = 0
*/
	ADC0CN = (U8)(0x03U | 0x04U) ;      // ADC0 disabled, normal tracking,
	                                    // conversion triggered on TMR2 overflow
	                                    // Output is right-justified

	REF0CN = 0x3C;                      // Enable on-chip VREF and buffer
	ADC0MX = 0x30;  // Set ADC input to initial setting

	ADC0CF = ((SYSCLK / 3000000) - 1) << 3; // Set SAR clock to 3MHz

	EIE1 |= (U8)0x04U;                       // Enable ADC0 conversion complete int.

	AD0EN = 1U;                          // Enable ADC0

	SFRPAGE = (S8)SFRPAGE_save;
}

U8 Get_Video_Signal(void)
{
	SEG_XDATA U8 ret;
	if ( RESULT > 0x100U)
	{
		ret = TRUE;
	}
	else
	{
		ret = FALSE;
	}
	return ret;
}

U8 Get_Temperature(void)
{
	return Temperature;
}


