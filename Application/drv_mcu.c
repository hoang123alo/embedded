/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_mcu.h"
#include "drv_timer.h"
#include "drv_timer.h"

/*--------------------------------------------------------------*/
SEG_XDATA volatile U8 g_FrameSync= 0x00U;

#ifndef __PST_PolySpace__
INTERRUPT_PROTO (INT1_ISR, INTERRUPT_INT1);
#else
void INT1_ISR(void);
#endif

void Port_Init (void)
{
    // P0.0  -  Skipped,     Open-Drain, Digital
    // P0.1  -  Skipped,     Open-Drain, Digital
    // P0.2  -  Skipped,     Open-Drain, Digital
    // P0.3  -  Skipped,     Open-Drain, Digital
    // P0.4  -  Skipped,     Open-Drain, Digital
    // P0.5  -  Skipped,     Open-Drain, Digital
    // P0.6  -  CAN_TX (CAN0), Push-Pull,  Digital
    // P0.7  -  CAN_RX (CAN0), Open-Drain, Digital

    // P1.0  -  SCK  (SPI0), Push-Pull,  Digital
    // P1.1  -  MISO (SPI0), Open-Drain, Digital
    // P1.2  -  MOSI (SPI0), Push-Pull,  Digital
    // P1.3  -  NSS  (SPI0), Push-Pull,  Digital
    // P1.4  -  S_M_PP,	     Push-Pull,  Digital
    // P1.5  -  Skipped,     Open-Drain, Digital
    // P1.6  -  FRAME_SYNC,  Open-Drain, Digital
    // P1.7  -  Skipped,     Open-Drain, Digital

    // P2.0  -  SDA (SMBus), Open-Drain, Digital
    // P2.1  -  SCL (SMBus), Open-Drain, Digital
    // P2.2  -  V_STB, 		 Open-Drain, Digital
    // P2.3  VIDEO_SIG(ADC0) Open-Drain, Analog
    // P2.4  -  ISP_RESET,   Push-Pull,  Digital
    // P2.5  -  CLK_EN,		 Push-Pull,  Digital
    // P2.6  -  FRAME_SYC,   Open-Drain, Digital
    // P2.7  -  1.8V_Enable, Push-Pull,  Digital

    // P3.0  -  Unassigned,  Open-Drain, Digital

    SFRPAGE   = CONFIG_PAGE;
    P2MDIN    = 0xF7U;
    P0MDOUT   = 0x40U;
    P1MDOUT   = 0x1DU;
    P2MDOUT   = 0xB0U;
    P0SKIP    = 0x3FU;
    P1SKIP    = 0xF0U;
    XBR0      = 0x0EU;
    XBR2      = 0x40U;
    SFRPAGE   = ACTIVE_PAGE;
	MCU_F_ENABLE = 0x00U;
	MCU_MCLK_ENABLE = 0x00U;
	//MCU_18V_ENABLE = 0x00U;
	MCU_SENSOR_RESET = 0x00U;
	
}

void Vdd_Monitor_Init(void)
{
#if defined(VDDMONITOR_ENABLE)
	#if defined(VDDMONITOR_HIGH)
	// High 2.30V VDD Monitor
	VDM0CN = 0x90U;			
	#endif

	#if defined(VDDMONITOR_LOW)
	// Low 1.75V VDD Monitor
	VDM0CN = 0x80U;			
	#endif
#else
	VDM0CN = 0x00U;
#endif
}

void Osc_Init (void)
{
  SFRPAGE   = CONFIG_PAGE;
  OSCICN    = 0xC7U;
  SFRPAGE   = ACTIVE_PAGE;
}

void Rstsrc_Init(void)
{
	RSTSRC   = 0x06U;   
}

void Exinterrupt_Init(void)
{
	SFRPAGE   = CONFIG_PAGE;
	TCON	  = TCON | 0x05U;		// /INT0 and /INT1 are edge triggered
	IT01CF	= 0x60U;				// INT0 = 1.7, INT1 = 1.6 Port
	EX1		= 0x01U;				// Enable /INT1 interrupts
	SFRPAGE   = ACTIVE_PAGE;
}

U8 Frame_Sync_Status(void)
{
	U8 ret = 0x00U;
	ret = FRAME_SYNC_GET;
	FRAME_SYNC_CLEAR;
	return ret;
}

void MCU_Reset(void)
{
   RSTSRC = 0x12U;    // Initiate software reset with vdd monitor enabled
}

void Device_Init(U8 Device)
{
	
	switch(Device)
	{
		case APTINA_AP0100:
			MCU_18V_ENABLE = 0x01U;
			MCU_MCLK_ENABLE = 0x01U;
			Wait_ms(10);
			MCU_SENSOR_RESET = 0x00U;
			Wait_ms(3);
			MCU_SENSOR_RESET = 0x01U;
			Wait_ms(320);
		break;
		case APTINA_ASX344:
			MCU_18V_ENABLE = 0x01U;
			MCU_MCLK_ENABLE = 0x01U;
			Wait_ms(10);
			MCU_SENSOR_RESET = 0x00U;
			Wait_ms(3);
			MCU_SENSOR_RESET = 0x01U;
			Wait_ms(300);
		break;
		default:
		break;
	}	
}
#ifndef __PST_PolySpace__
INTERRUPT(INT1_ISR, INTERRUPT_INT1)	// Frame Sync
#else
void INT1_ISR(void)
#endif
{
	FRAME_SYNC_COUNT;
}
