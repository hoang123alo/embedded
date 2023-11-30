/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_pca.h"
/*--------------------------------------------------------------*/

void PCA_Init(void)
{
	PCA0MD    &= (U8)(~0x40U);
	PCA0MD    = 0x00U;

	WDT_Clear();
		/*
	//32.8ms watchdog enable
	PCA0CN    = 0x40U;
	PCA0MD    &= (U8)(~0x40U);
	PCA0MD    = 0x00U;
	PCA0CPL5  = 0xFFU;
	PCA0MD    |= 0x40U;
   */
    PCA0CPM0 = 0x21U;                    // Module 0 = Rising Edge Capture Mode
                                       // enable CCF flag.
	EIE1 |= (U8)0x08U;                       // Enable PCA interrupts

	WDT_Clear();

}

void Watchdog_Init(void)
{
	#if defined(WATCHDOG_32MS)	
		//32.8ms watchdog enable
		PCA0CN    = (U8)0x40;
		PCA0MD    &= (U8)(~0x40U);		// Watchdog Disable
		PCA0MD    = (U8)0x0E;				// Timer0 Watchdog Source 
		PCA0CPL5  = (U8)0x00;				// 32.8ms Interval
		PCA0MD    |= (U8)0x40;				// Watchdog Enalbe
	#endif
	#if defined(WATCHDOG_400MS)
		PCA0CN    = (U8)0x40U;
	    PCA0MD    &= (U8)(~0x40U);
	    PCA0MD    = (U8)0x0EU;
    	PCA0CPL5  = (U8)0xFFU;
    	PCA0MD    |= (U8)0x40U;
	#endif
}

void WDT_Clear(void)
{
	PCA0CPH5 = (U8)0x00;
}

