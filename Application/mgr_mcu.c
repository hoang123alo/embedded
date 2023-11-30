/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "global_define.h"
#include "mgr_mcu.h"
#include "drv_timer.h"
#include "drv_pca.h"
#include "drv_mcu.h"
#include "drv_i2c.h"
#include "drv_adc.h"
#include "drv_mem.h"
#include "drv_spi.h"
#include "mgr_diag.h"
#include "mgr_isp.h"
#include "mgr_comm.h"

/*--------------------------------------------------------------*/

void Init_McuTask(void)
{	
	SEG_XDATA U8	i = 0U;

	SFRPAGE = ACTIVE_PAGE;			// Set SFR Page for PCA0

	PCA0MD &= (U8)(~0x40U);			// Disable Watchdog Timer
	
	VDM0CN	= 0x80U;				// low 1.75V VDD Monitor

	for (i = 0; i < 255U; i++){;}	// Wait 5us for initialization
	RSTSRC   = 0x06U;    			// Enable Missing Clock Detector, VDD MONITOR
	PCA_Init();
	Watchdog_Init();
	Osc_Init();						// Initialize oscillator
	Port_Init();					// Initialize Port
	Exinterrupt_Init();
	ADC_Init();
	Global_Timer_Init();
	RSTSRC   = 0x06U;    			// Enable Missing Clock Detector, VDD MONITOR
	EA = 1;							// Enable global interrupts
}

void Operate_McuTask(void)
{
	SEG_XDATA U8 On_Time = FALSE;
	static SEG_XDATA U8	init_flag = FALSE;
	
	//FRAME VALIDE CHECK
	On_Time = Delay_Time_Get(TID_FRAME_VALIDE);
	if ( On_Time == TRUE)
	{	
		Frame_Valide_Check();
	}
	//FRAME COUNT CHECK
	On_Time = Delay_Time_Get(TID_FRAME_COUNT);
	if ( On_Time == TRUE)
	{
		Frame_Count_Check();
		Video_Signal_Check();
		V_STB_Check();
	}

	if ( init_flag == FALSE)
	{
		init_flag = TRUE;
		if ( (RSTSRC & 0x02) == 0 )
		{
			if ( (RSTSRC & 0x08U) != 0x00U )
			{
				DTC_MCU_WD_ERROR;
			}
			else
			{
				DTC_MCU_WD_CLEAR;
			}
			if ( (RSTSRC & 0x04U) != 0x00U )
			{
				DTC_MCU_MC_ERROR;
			}
			else
			{
				DTC_MCU_MC_CLEAR;
			}
		}
	}


}

void Reset_Mcu(void)
{
	MCU_Reset();
}
