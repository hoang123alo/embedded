/****************************************************************************
* NAME: main.c
*----------------------------------------------------------------------------
* Copyright (c) MCNEX Inc.
*----------------------------------------------------------------------------
* CREATED_BY: Jung Gu Seo
* CREATION_DATE: 2011/03/14
* $Author: dglee $
* $Revision: 1.0 $
* $Date: 2016/08/10 $
*----------------------------------------------------------------------------
* PURPOSE: 
* - 
*****************************************************************************/

/*_____ I N C L U D E __________________________________________*/

#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "mgr_mcu.h"
#include "mgr_comm.h"
#include "mgr_diag.h"
#include "mgr_isp.h"
#include "drv_pca.h"
#include "drv_can.h"
#include "drv_mcu.h"
#include "drv_adc.h"
#include "drv_timer.h"
#include "string.h"
#include "main.h"

#define ANALOG_INPUTS 			2
#define INT_DEC             	256        // Integrate and decimate ratio
SEG_XDATA U8 AMUX_INPUT = 0x00U;

void Temperature_compute(void);
void Calibrate(S16 cal_input, S32 temp);

U8 main(void);

#ifndef __PST_PolySpace__
INTERRUPT_PROTO (TIMER2_ISR, INTERRUPT_TIMER2);
INTERRUPT_PROTO (ADC_ISR, INTERRUPT_ADC0_EOC);
#endif


extern SEG_XDATA tMsg_Global_Tick g_Global_Tick_Msg;
extern SEG_XDATA tMsg_Time_s	ga_tCAN_Time_Msg[TID_COUNT];

extern SEG_XDATA U16 RESULT;
extern SEG_XDATA U8 Temperature;
//U32 ADC_SUM = 0;;;;                       // Accumulates the ADC sample
unsigned long ADC_SUM = 0;                      // Accumulates the ADC samples
bit CONV_COMPLETE = 0;                 // ADC accumulated result ready flag
SEG_XDATA U8 vcan_count = 0x00U;

SEG_XDATA U8 EEPROM_Update_Mode = FALSE;

SEG_XDATA volatile U16 ga_Video_Signal[20] = {(U16)0x00,};
SEG_XDATA volatile U32 ga_Video_Signal_Sum = (U32)0x00U;
SEG_XDATA volatile U8 ga_Video_Signal_Cnt = 0x00U;
/*****************************************************************************
FUNCTION:
  - main

Purpose:
  - 
*****************************************************************************/

#ifndef __PST_PolySpace__
U8 main(void)
{ 
	SEG_XDATA U8 On_Time=0x00U;
	SEG_XDATA U8 temper=0x00U;
	WDT_Clear();

	Init_DiagTask();
	Init_McuTask();
	Init_CommTask();
	Init_IspTask();

	Delay_Time_Set(TID_DTC_WRITE,DT_DTC_WRITE);
	Delay_Time_Set(TID_ADC_CHANGE_TIMER,DT_ADC_CHANGE_TIMER);
	while(1)
	{
		WDT_Clear();

		if ( EEPROM_Update_Mode == FALSE)
		{
			Operate_CommTask();
			Operate_IspTask();
			Operate_McuTask();
			Operate_DiagTask();

			On_Time = Delay_Time_Get(TID_ADC_CHANGE_TIMER);
			if (On_Time == TRUE)
			{
				if (AMUX_INPUT == 0U)
				{
					Temperature_compute();				//temper = Get_Temperature();
					AMUX_INPUT = 1U;
					REF0CN = 0x33;                      // Enable on-chip VREF and buffer, Set VREF to 2.25V setting
	           		ADC0MX = 0x13;  					// Set ADC input to initial setting  
				}
				else
				{
					AMUX_INPUT = 0U;
			    	REF0CN = 0x3C;                      // Temp. Sensor on, VREF = 2.2V / VDD = 2.6V
					ADC0MX = 0x30;						// Set ADC input to temp-sensor
				}
			}
													
		}
	}
}
#else
U8 main(void)
{
	SEG_XDATA U8 On_Time=0x00U;
	WDT_Clear();
	Init_McuTask();
	Init_CommTask();

	Init_DiagTask();
	Init_IspTask();

	Delay_Time_Set(TID_ADC_CHANGE_TIMER,DT_ADC_CHANGE_TIMER);

	return 0;
}

void	pst_interrupt_isr(void)
{
	while( 1 )
	{
		if(get_random() > (U32)0) 
		{
			CAN_ISR();
		}
		
		if(get_random() > (U32)0)  
		{
			TIMER2_ISR();
		}

		if(get_random() > (U32)0)  
		{
			INT1_ISR();
		}
		
		if(get_random() > (U32)0)
		{
			break;
		}	
	}
}

void	pst_main_loop_isr(void)
{
	SEG_XDATA U8 On_Time=0x00U;
	for(;;)
    {
		if (get_random() > (U32)0)
		{
			WDT_Clear();
			if ( EEPROM_Update_Mode == FALSE)
			{
				Operate_CommTask();
				Operate_IspTask();
				Operate_McuTask();
				Operate_DiagTask();
			}
		}
	}
}
#endif	// #ifndef __PST_PolySpace__

#ifndef __PST_PolySpace__
INTERRUPT(TIMER2_ISR, INTERRUPT_TIMER2)
#else
void TIMER2_ISR(void)
#endif
{
	SEG_XDATA U8 i = 0x00U;
	static SEG_XDATA volatile U8 vcan_count = 0x00U;
	TF2H = 0x00U;

	g_Global_Tick_Msg.Tick_1ms++;
	
	vcan_count++;
	if ( vcan_count >= 10U )
	{
		vcan_count = 0x00U;
		VCan_Task();
	}

#if 1
	if ( g_Global_Tick_Msg.Tick_1ms >= 0x8FFFFFFFU )
	{
		g_Global_Tick_Msg.Tick_1ms = (U32)0;
		for ( i = (U8)0; i < TID_COUNT; i++)
		{
			ga_tCAN_Time_Msg[i].Cur_Time = (U32)0;

			if (ga_tCAN_Time_Msg[i].End_Time > 0x8FFFFFFFU)
			{
				ga_tCAN_Time_Msg[i].End_Time = (volatile U32)(ga_tCAN_Time_Msg[i].End_Time - 0x8FFFFFFFU);
			}
			else
			{
				ga_tCAN_Time_Msg[i].End_Time = (U32)0;
			}
		}				
	}
#else
	if ( g_Global_Tick_Msg.Over_Set == FALSE)
	{
		if ( g_Global_Tick_Msg.Tick_1ms >= 0xFFFFU )
		{
			if ( g_Global_Tick_Msg.Tick_1ms >= g_Global_Tick_Msg.Limit )
			{
				g_Global_Tick_Msg.Tick_1ms = (U32)0;
				for ( i = (U8)0; i < TID_COUNT; i++)
				{
					ga_tCAN_Time_Msg[i].End_Time = (U32)0;
				}				
			}
			else
			{
				g_Global_Tick_Msg.Over_Set = TRUE;
				g_Global_Tick_Msg.Limit = (volatile U32)(g_Global_Tick_Msg.Limit - 0xFFFFU);
			}
		}
	}
	else
	{
		g_Global_Tick_Msg.Limit--;
		if ( g_Global_Tick_Msg.Limit <= (U8)0 )
		{
			g_Global_Tick_Msg.Tick_1ms = (U16)0;
			g_Global_Tick_Msg.Over_Set = FALSE;
			for ( i = 0; i < TID_COUNT; i++)
			{
				ga_tCAN_Time_Msg[i].End_Time -= ga_tCAN_Time_Msg[i].Cur_Time;
			}
		}
	}
#endif
}

#ifndef __PST_PolySpace__
INTERRUPT (ADC_ISR, INTERRUPT_ADC0_EOC)
#else
void ADC_ISR(void)
#endif
{
   static U32 accumulator = 0;         // Accumulator for averaging
   static U16 measurements = SAMPLING_NUMBER; // Measurement counter
   SEG_XDATA U8 Settling_Cnt = 0x00U;

	EIE1 &= ~0x04;

   // Checks if obtained the necessary number of samples
	if ( AMUX_INPUT == 0x00U)
	{
	   if(measurements == 0)
	   {
	      ADC_SUM = accumulator;           // Copy total into ADC_SUM
	      measurements = SAMPLING_NUMBER;  // Reset counter
	      accumulator = 0;                 // Reset accumulator
	      CONV_COMPLETE = 1;               // Set result ready flag
	   }
	   else
	   {
	      accumulator += ADC0 >> 6;        // If not, keep adding while shifting
		  //accumulator += 0x1cc;        // If not, keep adding while shifting
		  //accumulator += ADC0;        // If not, keep adding while shifting
	                                       // out unused bits in ADC0
	      measurements--;
	   }
	}
	else
	{
		ga_Video_Signal[ga_Video_Signal_Cnt] = ADC0;
		ga_Video_Signal_Sum += (U32)ADC0;   
		ga_Video_Signal_Cnt++;
		if ( ga_Video_Signal_Cnt >= 20U )
		{
			RESULT = ga_Video_Signal_Sum / (U32)ga_Video_Signal_Cnt;
			ga_Video_Signal_Sum = (U32)0x00;	
			ga_Video_Signal_Cnt = 0x00U;	
		}
		/*RESULT = (U16)ADC0;*/
	}
	for (Settling_Cnt = 0; Settling_Cnt < 67; Settling_Cnt++) {}//50us settling time

	EIE1 |= 0x04; 
	AD0INT = 0;
}

void Temperature_compute(void)
{
   S32 temp_scaled;                    // Stores scaled temperature value

   S32 temp_whole;                     // Stores unscaled whole number portion
                                       // of temperature for output.
   S16 temp_frac;                      // Stores unscaled decimal portion of
                                       // temperature for output.
   SFRPAGE = ACTIVE_PAGE;              // Set for PCA0MD

   PCA0MD &= ~0x40;                    // Disable the watchdog timer

   EA = 1;                             // Enable all interrupts

      if(CONV_COMPLETE)
      {
         temp_scaled = ADC_SUM;        // Apply our derived equation to ADC

         //if (REVID == REV_B)
         if (1)
         {
            temp_scaled *= SLOPE_REV_B;
			
            // With a left-justified ADC, we have to shift the decimal place
            // of temp_scaled to the right so we can match the format of
            // OFFSET. Once the formats are matched, we can subtract OFFSET.
            temp_scaled = temp_scaled/1024; //OVER_ROUND;
			
            temp_scaled -= OFFSET_REV_B;
			
         }
         //else
         //{
         //   temp_scaled *= SLOPE_REV_C;

            // With a left-justified ADC, we have to shift the decimal place
            // of temp_scaled to the right so we can match the format of
            // OFFSET. Once the formats are matched, we can subtract OFFSET.
         //   temp_scaled = temp_scaled/1024; // >> OVER_ROUND;

         //   temp_scaled -= OFFSET_REV_C;
         //}

         // Calculate the temp's whole number portion by unscaling
         temp_whole = temp_scaled/SCALE;
		 Temperature = temp_whole;
         // The temp_frac value is the unscaled decimal portion of temperature
         temp_frac = (S16)((temp_scaled - temp_whole*SCALE) / (SCALE/10));

         if(temp_frac < 0)
         {
            temp_frac *= -1;           // If negative, remove negative from
                                       // temp_frac portion for output.
         }

         CONV_COMPLETE = 0;
      }
}

/* end of file */
