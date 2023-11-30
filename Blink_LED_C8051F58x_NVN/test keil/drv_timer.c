#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_timer.h"

SEG_XDATA tMsg_Global_Tick 	g_Global_Tick_Msg;
SEG_XDATA tMsg_Time_s		ga_tCAN_Time_Msg[TID_COUNT];

void Global_Timer_Init(void)
{
	U8 i = 0U;
	g_Global_Tick_Msg.Tick_1ms = (U32)0;
	g_Global_Tick_Msg.Limit = (U32)0;
	g_Global_Tick_Msg.Over_Set = (U8)0;

	for( i = 0U; i < TID_COUNT; i++)
	{
		ga_tCAN_Time_Msg[i].Set 	 	= 0;
		ga_tCAN_Time_Msg[i].Delay_Time  = 0;
		ga_tCAN_Time_Msg[i].Cur_Time 	= 0;
		ga_tCAN_Time_Msg[i].End_Time 	= 0;
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

#if defined(TIMER0)
void Timer0_Init()
{
}
#endif

#if defined(TIMER1)
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
#endif

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

void Delay_Time_Set(U8 ID, U16 Delay_Time)
{
	SEG_XDATA volatile U8 Set_Delay_Time = 0x00U;

	ga_tCAN_Time_Msg[ID].Cur_Time = g_Global_Tick_Msg.Tick_1ms;
	ga_tCAN_Time_Msg[ID].Delay_Time = (U32)Delay_Time;
	ga_tCAN_Time_Msg[ID].Set = TRUE;

	ga_tCAN_Time_Msg[ID].End_Time = (ga_tCAN_Time_Msg[ID].Cur_Time + ga_tCAN_Time_Msg[ID].Delay_Time);
	if ( (g_Global_Tick_Msg.Over_Set == FALSE) && (g_Global_Tick_Msg.Limit <= ga_tCAN_Time_Msg[ID].End_Time) )
	{
		g_Global_Tick_Msg.Limit = ga_tCAN_Time_Msg[ID].End_Time;
	}
}

U8 Delay_Time_Get(U8 ID)
{
	U8 ret = (U8)0;
	U8 i   = (U8)0;
	U8 cnt = (U8)0;

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