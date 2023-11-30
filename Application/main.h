
#ifndef MAIN_H
#define MAIN_H

/* ----- Global Define -------------------------------*/
#define SCALE                    1000L // Scale for temp calculations
#define SAMPLING_NUMBER            256 // Number of samples per calculation
#define OVER_ROUND                  10 // Number of shifts (>>N) to reach the
#define REV_B                     0x01 // Value of the REVID register for
#define SLOPE_REV_B               3047   // Slope of the temp transfer function
#define OFFSET_REV_B           265610 // Offset for the temp transfer function
	                                  // These values are for revision B
                                       // devices with the equation
                                       //     Vtemp = 2.67T(C) + 713mV
#define REV_C                     0x02 // Value of the REVID register for
#define SLOPE_REV_C               2261 // Slope of the temp transfer function
#define OFFSET_REV_C           265610L // Offset for the temp transfer function
/*----------------------------------------------------*/

/* ----- Global Value --------------------------------*/

/*----------------------------------------------------*/


/* ----- Function ------------------------------------*/
/*----------------------------------------------------*/


/* ----- INTERRUPT -----------------------------------*/
/*----------------------------------------------------*/

/* ----- Extern Function -----------------------------*/
extern SEG_XDATA U8 EEPROM_Update_Mode;
#ifndef __PST_PolySpace__
#else
  extern void	TIMER2_ISR(void);
  extern void 	CAN_ISR(void);
  extern U32	get_random(void);
  extern void	pst_main_loop_isr(void);
  extern void	pst_interrupt_isr(void);
  extern void 	INT1_ISR(void);
  extern void 	ADC_ISR(void);
#endif
/*----------------------------------------------------*/

#endif
