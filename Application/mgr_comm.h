#ifndef MGR_COMM_H
#define MGR_COMM_H

#include "compiler_defs.h"
#include "C8051F580_defs.h"

/* ----- Global Define -------------------------------*/
/*
#define TW8836		0x00U
#define DESERIALIZE	0x01U
#define SERIALIZE	0x02U
*/
#define  TW8836_SLAVE_ADDRESS   (U8)0x8A
#define  TW8836_ID_ADRESS 	    (U8)0x00
#define  TW8836_ID		 		(U16)0x0036

#define  INIT		      		0x00U           // (MR) data byte received
#define  RX			      		0x01U           // (MR) data byte received
#define  TX			      		0x02U           // (MR) data byte received

#define USE_MDPS11				0x00U
#define USE_LANGUAGEINFO		0x01U
#define USE_HUTYPE				0x02U
#define USE_GATEWAY				0x03U
#define USE_NAVIONOFF			0x04U
#define USE_G_SEL_DISP			0x05U
#define USE_IGN_SW				0x06U
#define USE_RVM_CAMERAOFF		0x07U
#define FCZC_RVM_SW				0x08U
#define USE_4WDERR				0x09U
#define	USE_ENGVOL				0x0AU
#define USE_CAN_COUNT			0x0BU		

#define TYPE_2WD				0x00U
#define TYPE_4WD				0x01U

#define TYPE_ENGVOL20			0x14U
#define TYPE_ENGVOL22			0x16U
#define TYPE_ENGVOL33			0x21U
/*----------------------------------------------------*/

/* ----- Function ------------------------------------*/
void Init_CommTask(void);
void Operate_CommTask(void);
void Rx_Data_Process(void);
//void Comm_I2C_Tx_Data(void);
void Comm_I2C_Tx(U8 Slave_Addr,U8 Length,U16 Addr, U16 TData,U16 RData );
U16   Comm_I2C_Rx(U8 Slave_Addr,U8 Length,U16 Addr, U16 TData,U16 RData );
void Clear_I2C_Tx_Data(void);
void Clear_CAN_Tx_Data(void);
void Clear_Can_Data(void);
void Clear_Sas_Data(void);

void Comm_RearView_Data(void);
void Comm_Tuning_Value_Type(U8 Type4WdErr, U8 TypeEngVol);
U8 Comm_Sas_Data(U8 *pa_Data);
U8 Comm_Hutype_Data(U8 *pa_Data);
U8 Comm_Language_Data(U8 *pa_Data);
U8 Comm_G_Sel_Disp_Data(U8 *Data);
U8 Comm_IGN_Sw_Data(U8 *Data);
U8 Comm_RVM_CameraOff_Data(U8 *Data);
U8 Comm_RVM_SW_Data(U8 *Data);
U8 Comm_ENG_Vol_Data(U8 *pa_Data);
U8 Comm_4WD_Err_Data(U8 *pa_Data);
void Comm_Tx_RVM_View(U8 sigData);
void Comm_Tx_RVM_SW_IND(U8 sigData);

void Flash_OneSpec_Write(void);
void Flash_OneSpec_Read(void);
void Comm_Error_Check(void);

void IlPutTxRVM_View(U8 sigData);

void IlPutTxRVM_SW_IND(U8 sigData);
/*----------------------------------------------------*/

/* ----- Extern Function -----------------------------*/
/*----------------------------------------------------*/

/* ----- Global Value --------------------------------*/

typedef struct {
	U8  Slave;
	U8  Length;
	U8  TRX_Flag;
	U16 Addr;
	U16 TxData;
	U16 RxData;
}tMsg_I2C_Tx_Data_s;

typedef struct {
	U8  Slave;
	U8  Length;
	U8  TRX_Flag;
	U16 Addr;
	U16 TxData;
	U16 RxData;
}tMsg_CAN_Tx_Data_s;

typedef struct {
	U8 Is_Change;
	U8 Change_Count;
	U8 Time_Out;
	U8 Cur_Data;
	U8 Mid_Data;
	U8 Pre_Data;
	U16 Chatter_Time;
}tMsg_CAN_Rx_Data_s;

typedef struct {
	U8  Is_Change;
	U8  Pre_Sign;
	U8  Angle;
	U8  Angle_Pre_Sign;
	U8  Time_Out;
	U16 Pre_Data;
	U8	Tuning_Type;
}tMsg_CAN_SAS_Data_s;

typedef struct {
	U8  Is_Change;
	U8  Pre_Sign;
	U8  Angle;
	U16 Pre_Data;
}tMsg_CAN_State_s;


/*
struct tCAN_Tx_Data_s {
	U8 Flag;
	U8 Dest;
	U8 Length;
	U8 *TxData;
}

struct tCAN_Rx_Data_s {
	U8 Flag;
	U8 Src;
	U8 Length;
	U8 RxData;
}
*/
/*----------------------------------------------------*/

extern SEG_XDATA volatile tMsg_CAN_SAS_Data_s tCAN_Rx_SAS_Msg;
#endif

