#ifndef __DRV_MCU_H__
#define __DRV_MCU_H__

/********************** Macros **********************/
SBIT (LED_STATUS_ERROR,			SFR_P2, 0);
SBIT (LED_STATUS_UPDOWNLOAD,	SFR_P2, 1);
SBIT (BUTTON,	 				SFR_P1, 2);

/********************** Function Prototypes **********************/
void Osc_Init(void);
void Port_Init(void);
void Exinterrupt_Init(void);

#endif