/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_timer.h"
#include "drv_mcu.h"
#include "drv_pca.h"
/*--------------------------------------------------------------*/

SEG_XDATA tMsg_Global_Tick g_Global_Tick_Msg;
SEG_XDATA tMsg_Time_s	ga_tCAN_Time_Msg[TID_COUNT];


void Global_Timer_Init(void)
{
	U8 i = 0x00U;
	g_Global_Tick_Msg.Tick_1ms = (U32)0U;
	g_Global_Tick_Msg.Limit = (U32)0U;
	g_Global_Tick_Msg.Over_Set = (U8)0U;

	for( i = 0U; i < TID_COUNT; i++)
	{
		ga_tCAN_Time_Msg[i].Set 	 	= (U8)0U;
		ga_tCAN_Time_Msg[i].Delay_Time  = (U32)0U;
		ga_tCAN_Time_Msg[i].Cur_Time 	= (U32)0U;
		ga_tCAN_Time_Msg[i].End_Time 	= (U32)0U;
	}

#if defined(TIMER0)
	Timer0_Init();
#endif
#if defined(TIMER1)
	Timer1_Init();
#endif
#if defined(TIMER2)
	Timer2_Init();
#endif
#if defined(TIMER3)
	Timer3_Init();
#endif
#if defined(TIMER4)
	Timer4_Init();
#endif
#if defined(TIMER5)
	Timer5_Init();
#endif

}
/*
#if defined(TIMER1)
void Timer1_Init()
{
	SEG_XDATA U8 SFRPAGE_save = (U8)SFRPAGE;
	SFRPAGE = (U8)ACTIVE_PAGE;

	// Make sure the Timer can produce the appropriate frequency in 8-bit mode
	// Supported SMBus Frequencies range from 10kHz to 100kHz.  The CKCON register
	// settings may need to change for frequencies outside this range.
	CKCON |= 0x08U;                   // Timer1 clock source = SYSCLK

	TMOD = 0x20U;                        // Timer1 in 8-bit auto-reload mode

	// Timer1 configured to overflow at 1/3 the rate defined by SMB_FREQUENCY
#if defined(I2C_SPEED_400KHZ)
	TH1 = 0xECU;
#elif defined(I2C_SPEED_100KHZ)
	TH1 = 0xB0U;//100Khz
#endif
	TL1 = TH1;                          // Init Timer1

	TR1 = 0x01;                            // Timer1 enabled

	SFRPAGE = SFRPAGE_save;
}
#endif
*/
void Timer1_Init (void)
{
	SEG_XDATA U8 SFRPAGE_save = SFRPAGE;
   SFRPAGE = ACTIVE_PAGE;

// Make sure the Timer can produce the appropriate frequency in 8-bit mode
// Supported SMBus Frequencies range from 10kHz to 100kHz.  The CKCON register
// settings may need to change for frequencies outside this range.
	CKCON |= (U8)0x08;                   // Timer1 clock source = SYSCLK

	TMOD = 0x20U;                        // Timer1 in 8-bit auto-reload mode

   // Timer1 configured to overflow at 1/3 the rate defined by SMB_FREQUENCY
	TH1 = 0xECU;//-(S8)(SYSCLK/SMB_FREQUENCY/3);400KHz
//	TH1 = 0xB0U;//100Khz

	TL1 = TH1;                          // Init Timer1

	TR1 = 0x01U;                            // Timer1 enabled

	SFRPAGE = SFRPAGE_save;
}
#if defined(TIMER2)
void Timer2_Init (void)
{
	SEG_XDATA U8 SFRPAGE_save = SFRPAGE;
   SFRPAGE = ACTIVE_PAGE;

   TMR2CN  = 0x00U;                    // Stop Timer2; Clear TF2;
                                       // use SYSCLK/12 as timebase 500ns unit
   CKCON  &= (U8)(~0x60U);             // Timer2 clocked based on T2XCLK;

//   TMR2RL  = 0xB1E0U;           // Init reload values//for 10ms -20000
   TMR2RL  = 0xF830U;           // Init reload values//for 1ms	-2000
   
   TMR2    = 0xFFFFU;                   // Set to reload immediately
   ET2     = 0x01U;                     // Enable Timer2 interrupts
   TR2     = 0x01U;                     // Start Timer2

   SFRPAGE = SFRPAGE_save;
}
#endif
#if defined(TIMER3)
void Timer3_Init(void)
{
	SEG_XDATA U8 SFRPAGE_save = SFRPAGE;
   SFRPAGE = ACTIVE_PAGE;

   TMR3CN = 0x00U;                      // Timer3 configured for 16-bit auto-
                                       // reload, low-byte interrupt disabled

   CKCON &= (U8)(~0x40U);                     // Timer3 uses SYSCLK/12

   TMR3RL = 0x3CB0U;//-(S16)(SYSCLK/12/40);      // Timer3 configured to overflow after
   TMR3 = TMR3RL;                      // ~25ms (for SMBus low timeout detect):
                                       // 1/.025 = 40

   EIE1 |= (U8)0x40;                       // Timer3 interrupt enable
   TMR3CN |= (U8)0x04;                     // Start Timer3

   SFRPAGE = SFRPAGE_save;
}
#endif
#if defined(TIMER4)
void Timer4_Init()
{
}
#endif
#if defined(TIMER5)
void Timer5_Init(void)
{
    SFRPAGE   = ACTIVE2_PAGE;
    TMR5CN    = 0x04U;
    TMR5CF    = 0x0AU;
    TMR5CAPL  = 0x6AU;
    TMR5CAPH  = 0xFFU;
    SFRPAGE   = ACTIVE_PAGE;
}
#endif

void Wait_ms (U16 ms)
{
	// All registers are on all pages, so no need to set SFR page
	SEG_XDATA volatile U8 reg_tval = 0x00U;

	TCON &= (U8)(~0x30U);                      // Stop Timer0; Clear TF0
	TMOD &= (U8)(~0x0FU);                      // 16-bit free run mode
	TMOD |= (U8)0x01;

	CKCON |= (U8)0x04;                      		// Timer0 counts SYSCLKs

	while (ms > 0U)
	{
		TR0 = 0x00U;			            	// Stop Timer0
		TH0 = (U8)-(SYSCLK/1000/ 256);   	// Overflow in 1ms
		//TL0 = (S8)(-(SYSCLK/1000));
		TL0 = 0xB4U;
		TF0 = 0x00U;                         	// Clear overflow indicator
		TR0 = 0x01U;                         	// Start Timer0

		while( 1 ){
			reg_tval = (U8)TF0;
			if( reg_tval != 0x00U )
			{
				break;
			}
		}
		ms--;                            // Update ms counter
		WDT_Clear();
	}
	TR0 = 0x00U;                         // Stop Timer0

	WDT_Clear();
}

void Delay_Time_Expire(U8 ID)
{
	ga_tCAN_Time_Msg[ID].Set 	 	 = (U8)0;
	ga_tCAN_Time_Msg[ID].Delay_Time  = (U32)0;
	ga_tCAN_Time_Msg[ID].Cur_Time 	 = (U32)0;
	ga_tCAN_Time_Msg[ID].End_Time 	 = (U32)0;
}

void Delay_Time_Set(U8 ID, U16 Delay_Time)
{
	ga_tCAN_Time_Msg[ID].Cur_Time = g_Global_Tick_Msg.Tick_1ms;
	ga_tCAN_Time_Msg[ID].Delay_Time = (U32)Delay_Time;
	ga_tCAN_Time_Msg[ID].Set = TRUE;

	ga_tCAN_Time_Msg[ID].End_Time = (ga_tCAN_Time_Msg[ID].Cur_Time + ga_tCAN_Time_Msg[ID].Delay_Time);

#if 0
	if ( (g_Global_Tick_Msg.Over_Set == FALSE) && (g_Global_Tick_Msg.Limit <= ga_tCAN_Time_Msg[ID].End_Time) )
	{
		g_Global_Tick_Msg.Limit = ga_tCAN_Time_Msg[ID].End_Time;
	}
#endif
}

U8 Delay_Time_Get(U8 ID)
{
	U8 ret = (U8)0x00U;

	ga_tCAN_Time_Msg[ID].Cur_Time = g_Global_Tick_Msg.Tick_1ms;
	if (ga_tCAN_Time_Msg[ID].Set == TRUE )
	{
		if( ga_tCAN_Time_Msg[ID].Cur_Time >= ga_tCAN_Time_Msg[ID].End_Time )
		{
			Delay_Time_Set(ID,(U16)ga_tCAN_Time_Msg[ID].Delay_Time);
			ret = TRUE;
		}
	}
	return ret;
}

U16 Get_Time(void)
{
	return (U16)g_Global_Tick_Msg.Tick_1ms;
}

