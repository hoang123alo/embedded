
/* ----- Global Define -------------------------------*/

#define DTC								(U8)0x01
#define ONESPEC							(U8)0x02

/*----------------------------------------------------*/

/* ----- Function ------------------------------------*/
void Init_McuTask(void);
void Reset_Mcu(void);
void Operate_McuTask(void);
void Mcu_StandBy(U8 *pError);
void Mcu_InterInit(U8 *pError);
void Mcu_Idle(U8 *pError);
void Mcu_Running(U8 *pError);
U8 Get_Dser_Vs_Status(void);

/*----------------------------------------------------*/

/* ----- Extern Function -----------------------------*/
//extern U16 i2c_master_read(U8 slave_addr, U16 addr);
/*----------------------------------------------------*/

/* ----- Global Value --------------------------------*/
/*----------------------------------------------------*/



