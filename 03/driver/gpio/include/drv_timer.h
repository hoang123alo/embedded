#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

/*************** ID TIMER DELAY ***************/
#define TID_BLINK_TIMER				0x01U
#define TID_COUNT					0x02U

/*************** TIMER DELAY ***************/
#define DT_BLINK_TIMER				500U

typedef struct {
	U8	Set;
	U32 Delay_Time;
	U32 Cur_Time;
	U32 End_Time;
}tMsg_Time_s;

typedef struct {
	U32 Tick_1ms;
	U32	Limit;
	U8	Over_Set;
}tMsg_Global_Tick;

void Global_Timer_Init(void);
void Delay_Time_Set(U8 ID, U16 Delay_Time);
U8   Delay_Time_Get(U8 ID);

#if defined(TIMER0)
void Timer0_Init(void);
#endif
#if defined(TIMER1)
void Timer1_Init(void);
#endif
#if defined(TIMER2)
void Timer2_Init(void);
#endif
#if defined(TIMER3)
void Timer3_Init(void);
#endif
#if defined(TIMER4)
void Timer4_Init(void);
#endif
#if defined(TIMER5)
void Timer5_Init(void);
#endif

#endif