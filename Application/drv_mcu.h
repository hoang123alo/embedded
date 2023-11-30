#ifndef DRV_MCU_H
#define DRV_MCU_H
/* ----- Global Define -------------------------------*/
//#define  SYSCLK						(24000000)	// System clock frequency in Hz

#define FRAME_SYNC_COUNT	((g_FrameSync)++)
#define FRAME_SYNC_GET		(g_FrameSync)
#define FRAME_SYNC_CLEAR	((g_FrameSync) = (0))
/*----------------------------------------------------*/

/* ----- Global Value --------------------------------*/
/* Pin Name Set */
//-----------------------------------------------------------------------------
// Macros and Macrodefinitions
//-----------------------------------------------------------------------------
SBIT (MCU_F_ENABLE,			SFR_P1, 4);
SBIT (MCU_FRAME_SYNC,	 	SFR_P1, 6);
SBIT (AMP_V_STB,	 		SFR_P2, 2);
SBIT (AMP_VIDEO_SIG,		SFR_P2, 3);
SBIT (MCU_SENSOR_RESET,		SFR_P2, 4);
SBIT (MCU_MCLK_ENABLE,		SFR_P2, 5);
SBIT (MCU_18V_ENABLE,	 	SFR_P2, 7);

SBIT (SDA, SFR_P2, 0);
SBIT (SCL, SFR_P2, 1);
/*----------------------------------------------------*/

/* ----- Function ------------------------------------*/
void Vdd_Monitor_Init(void);
void Osc_Init(void);
void Port_Init(void);
void Rstsrc_Init(void);
void Exinterrupt_Init(void);
void MCU_Reset(void);
void Device_Init(U8 Device);
U8 Frame_Sync_Status(void);

#ifndef __PST_PolySpace__
#else
void INT1_ISR(void);
#endif
/*----------------------------------------------------*/

/* ----- Extern Function -----------------------------*/

/*----------------------------------------------------*/
#endif










