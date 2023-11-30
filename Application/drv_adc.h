#ifndef DRV_ADC_H
#define DRV_ADC_H


/* ----- Global Define -------------------------------*/

/*----------------------------------------------------*/

/* ----- Global Value --------------------------------*/

/*----------------------------------------------------*/


/* ----- Function ------------------------------------*/
void ADC_Init(void);
U8 Get_Video_Signal(void);
U8 Get_Temperature(void);
/*----------------------------------------------------*/


/* ----- INTERRUPT -----------------------------------*/

/*----------------------------------------------------*/

/* ----- Extern Function -----------------------------*/
extern SEG_XDATA U16 RESULT;
/*----------------------------------------------------*/

#endif

