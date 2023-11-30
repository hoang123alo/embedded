/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_i2c.h"
#include "drv_spi.h"
#include "drv_mcu.h"
#include "drv_mem.h"
#include "drv_adc.h"        /* MISRA-C Rule 160504 */
#include "mgr_comm.h"
#include "mgr_isp.h"
#include "desc.h"
#include "nm_basic.h"
#include "il_inc.h"
#include "string.h"
/*--------------------------------------------------------------*/
static SEG_XDATA tMsg_I2C_Tx_Data_s tI2C_Tx_Msg;
SEG_XDATA volatile tMsg_CAN_SAS_Data_s tCAN_Rx_SAS_Msg;
SEG_XDATA volatile tMsg_CAN_Rx_Data_s tCAN_Rx_Msg[USE_CAN_COUNT];
extern SEG_XDATA tMsg_Ext_DTC_Type ga_Ext_tDTC_Type_Msg;
extern SEG_XDATA volatile U8 IGN_ON_Status;

void Init_CommTask(void)
{	
	SEG_XDATA U8 i = 0x00U;
	SMBUS0_Init();					// Configure and enable SMBus
	Spi_Init();
	Delay_Time_Set(TID_VCAN,DT_VCAN);

	Clear_Can_Data();	
	Clear_Sas_Data();
	Clear_I2C_Tx_Data();

	for ( i = 0; i < USE_CAN_COUNT; i++)
	{
		if ( tCAN_Rx_Msg[i].Pre_Data != 0xFFU)
		{
			tCAN_Rx_Msg[i].Is_Change = TRUE;
		}
	}

	VCan_Init();
	VCan_Start();	
}

void Operate_CommTask(void)
{
	SEG_XDATA U8 On_Time = FALSE;
	
	CAN_Rx_Data_G_SEL_DISP_Process(&tCAN_Rx_Msg[USE_G_SEL_DISP]);

	On_Time = Delay_Time_Get(TID_VCAN);
	if ( On_Time == TRUE )
	{	
		CAN_Rx_Sas_Process(&tCAN_Rx_SAS_Msg);
		CAN_Rx_Data_Process(USE_LANGUAGEINFO,&tCAN_Rx_Msg[USE_LANGUAGEINFO]);
		CAN_Rx_Data_Process(USE_HUTYPE,&tCAN_Rx_Msg[USE_HUTYPE]);
		CAN_Rx_Data_IGN_SW_Process(&tCAN_Rx_Msg[USE_IGN_SW]);
		CAN_Rx_Data_RVM_CameraOff_Process(&tCAN_Rx_Msg[USE_RVM_CAMERAOFF]);
		CAN_Rx_Data_FCZC_RVM_SW_Process(&tCAN_Rx_Msg[FCZC_RVM_SW]);
		CAN_Rx_Data_4WD_ERR_Process(&tCAN_Rx_Msg[USE_4WDERR]);
		CAN_Rx_Data_Eng_Vol_Process(&tCAN_Rx_Msg[USE_ENGVOL]);

		if ((tCAN_Rx_Msg[USE_4WDERR].Is_Change == TRUE ) ||  (tCAN_Rx_Msg[USE_ENGVOL].Is_Change == TRUE ))
		{
			tCAN_Rx_Msg[USE_4WDERR].Is_Change = FALSE;
			tCAN_Rx_Msg[USE_ENGVOL].Is_Change = FALSE;

			//Flash_OneSpec_Write(); //ENG_ONESPEC
			Comm_Tuning_Value_Type(tCAN_Rx_Msg[USE_4WDERR].Pre_Data, tCAN_Rx_Msg[USE_ENGVOL].Pre_Data);
		}
	}
	Comm_Error_Check();
}

void Comm_RearView_Data(void)
{
	Comm_IGN_Sw_Data(&tISP_Func_Msg.IGNSw);
	Comm_RVM_CameraOff_Data(&tISP_Func_Msg.Cur_RVM_CamOff);
	Comm_RVM_SW_Data(&tISP_Func_Msg.Cur_RVM_SW);
	Comm_G_Sel_Disp_Data(&tISP_Func_Msg.Cur_Gear);

	if ((tISP_Func_Msg.IGNSw == IGN_SW_ON) && (tISP_Func_Msg.Cur_RVM_CamOff != RVM_CAMERA_OFF) && (tISP_Func_Msg.RVM_SW_Status == FCZC_RVM_SW_ON))
	{
		if (tISP_Func_Msg.Cur_Gear == GEAR_R)
		{
			tISP_Func_Msg.Cur_ViewMode = RVM_PARKING_ASSIST_VIEW;
			if (tISP_Func_Msg.Pre_Gear != GEAR_R)
			{
				tISP_Func_Msg.Cur_SW_IND = RVM_SW_IND_OFF;
				tISP_Func_Msg.RVM_SW_Status = FCZC_RVM_SW_OFF;
			}
			else
			{
				tISP_Func_Msg.Cur_SW_IND = RVM_SW_IND_ON;
			}
		}
		else if (tISP_Func_Msg.Cur_Gear == GEAR_P)
		{
			tISP_Func_Msg.Cur_ViewMode = RVM_VIEW_OFF;
			tISP_Func_Msg.Cur_SW_IND = RVM_SW_IND_ON;
		}
		else if (tISP_Func_Msg.Cur_Gear == GEAR_INTERMEDIATE)
		{
			// intermediate 신호 무시
		}
		else // P,R,N,D 외 모두 D 처리
		{
			tISP_Func_Msg.Cur_ViewMode = RVM_DRIVING_ASSIST_VIEW;
			tISP_Func_Msg.Cur_SW_IND = RVM_SW_IND_ON;
		}
	}
	else
	{
		if (tISP_Func_Msg.Cur_Gear == GEAR_R)
		{
			tISP_Func_Msg.Cur_ViewMode = RVM_PARKING_ASSIST_VIEW;
			tISP_Func_Msg.Cur_SW_IND = RVM_SW_IND_OFF;
			tISP_Func_Msg.RVM_SW_Status = FCZC_RVM_SW_OFF;
		}
		else if (tISP_Func_Msg.Cur_Gear == GEAR_INTERMEDIATE)
		{
			// intermediate 신호 무시
		}
		else
		{
			tISP_Func_Msg.Cur_ViewMode = RVM_VIEW_OFF;
			tISP_Func_Msg.Cur_SW_IND = RVM_SW_IND_OFF;
			tISP_Func_Msg.RVM_SW_Status = FCZC_RVM_SW_OFF;
		}
	}

	if ((tISP_Func_Msg.Pre_RVM_SW == FCZC_RVM_SW_OFF) && (tISP_Func_Msg.Cur_RVM_SW == FCZC_RVM_SW_ON))
	{
		if (tISP_Func_Msg.RVM_SW_Status == FCZC_RVM_SW_ON)
		{
			tISP_Func_Msg.RVM_SW_Status = FCZC_RVM_SW_OFF;
		}
		else
		{
			tISP_Func_Msg.RVM_SW_Status = FCZC_RVM_SW_ON;
		}
	}

	if (((tISP_Func_Msg.Pre_RVM_SW == FCZC_RVM_SW_OFF) && (tISP_Func_Msg.Cur_RVM_SW == FCZC_RVM_SW_ON)) ||
		(tISP_Func_Msg.Pre_Gear != tISP_Func_Msg.Cur_Gear))
	{
		// Camera off 상태에서 Gear or RVM SW 변경 시 Camera off 해제
		tCAN_Rx_Msg[USE_RVM_CAMERAOFF].Pre_Data = RVM_CAMERA_INVALID;
	}

	tISP_Func_Msg.Pre_RVM_SW = tISP_Func_Msg.Cur_RVM_SW;
	tISP_Func_Msg.Pre_Gear = tISP_Func_Msg.Cur_Gear;

	// mgr_diag의 IGN On/Off 30회 후 DTC 삭제를 위한 코드
	if (tISP_Func_Msg.IGNSw == IGN_SW_ON)
	{
		if (IGN_ON_Status == IGN_CLEAR)
		{
			IGN_ON_Status = IGN_ON;
		}
	}
	else
	{
		IGN_ON_Status = IGN_CLEAR;
	}
}

void Comm_Tuning_Value_Type(U8 Type4WdErr, U8 TypeEngVol)
{
	if (Type4WdErr == TYPE_4WD)
	{
		if (TypeEngVol == TYPE_ENGVOL33)
		{
			tCAN_Rx_SAS_Msg.Tuning_Type = 0x03U;
		}
		else
		{
			tCAN_Rx_SAS_Msg.Tuning_Type = 0x02U;
		}
	}
	else
	{
		if (TypeEngVol == TYPE_ENGVOL33)
		{
			tCAN_Rx_SAS_Msg.Tuning_Type = 0x01U;
		}
		else
		{
			tCAN_Rx_SAS_Msg.Tuning_Type = 0x00U;
		}
	}
}
// ktram thay doi cua du lieu 
U8 Comm_Sas_Data(U8 *pa_Data)
{
	SEG_XDATA U8 ret = FALSE;

	pa_Data[0] = tCAN_Rx_SAS_Msg.Angle;
	pa_Data[1] = tCAN_Rx_SAS_Msg.Pre_Sign;
	ret = tCAN_Rx_SAS_Msg.Is_Change;
	tCAN_Rx_SAS_Msg.Is_Change = FALSE;
	return ret;
}

U8 Comm_Language_Data(U8 *pa_Data)
{
	SEG_XDATA U8 ret;

	ret = tCAN_Rx_Msg[USE_LANGUAGEINFO].Is_Change;
	if ( ret == TRUE)
	{
		*pa_Data = tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data;
	}
	tCAN_Rx_Msg[USE_LANGUAGEINFO].Is_Change = FALSE;
	return ret;
}

U8 Comm_Hutype_Data(U8 *pa_Data)
{
	SEG_XDATA U8 ret = 0x00U;

	ret = tCAN_Rx_Msg[USE_HUTYPE].Is_Change;
	if ( ret == TRUE)
	{
		*pa_Data = tCAN_Rx_Msg[USE_HUTYPE].Pre_Data;
	}
	tCAN_Rx_Msg[USE_HUTYPE].Is_Change = FALSE;
	return ret;
}

U8 Comm_G_Sel_Disp_Data(U8 *Data)
{
	SEG_XDATA U8 ret;

	Data[0] = tCAN_Rx_Msg[USE_G_SEL_DISP].Pre_Data;

	ret = tCAN_Rx_Msg[USE_G_SEL_DISP].Is_Change;
	tCAN_Rx_Msg[USE_G_SEL_DISP].Is_Change = FALSE;

	return ret;
}

U8 Comm_IGN_Sw_Data(U8 *Data)
{
	SEG_XDATA U8 ret;

	Data[0] = tCAN_Rx_Msg[USE_IGN_SW].Pre_Data;

	ret = tCAN_Rx_Msg[USE_IGN_SW].Is_Change;
	tCAN_Rx_Msg[USE_IGN_SW].Is_Change = FALSE;

	return ret;
}

U8 Comm_RVM_CameraOff_Data(U8 *Data)
{
	SEG_XDATA U8 ret;

	Data[0] = tCAN_Rx_Msg[USE_RVM_CAMERAOFF].Pre_Data;

	ret = tCAN_Rx_Msg[USE_RVM_CAMERAOFF].Is_Change;
	tCAN_Rx_Msg[USE_RVM_CAMERAOFF].Is_Change = FALSE;

	return ret;
}

U8 Comm_RVM_SW_Data(U8 *Data)
{
	SEG_XDATA U8 ret;

	Data[0] = tCAN_Rx_Msg[FCZC_RVM_SW].Pre_Data;

	ret = tCAN_Rx_Msg[FCZC_RVM_SW].Is_Change;
	tCAN_Rx_Msg[FCZC_RVM_SW].Is_Change = FALSE;

	return ret;
}

void Comm_Tx_RVM_View(U8 sigData)
{
	IlPutTxRVM_View(sigData);
}

void Comm_Tx_RVM_SW_IND(U8 sigData)
{
	IlPutTxRVM_SW_IND(sigData);
}
/*
void Comm_I2C_Tx_Data(void)
{
	if ( tI2C_Tx_Msg.TRX_Flag == TX )
	{
		i2c_master_write(tI2C_Tx_Msg.Slave,tI2C_Tx_Msg.Addr,tI2C_Tx_Msg.TxData,tI2C_Tx_Msg.Length);
	}
	else if ( tI2C_Tx_Msg.TRX_Flag == RX )
	{
		tI2C_Tx_Msg.RxData = i2c_master_read(tI2C_Tx_Msg.Slave,tI2C_Tx_Msg.Addr,tI2C_Tx_Msg.Length);
	}
	else
	{}
}
*/
void Comm_Error_Check(void)
{
	//SEG_XDATA volatile U16 chk_data;
	SEG_XDATA volatile U8 Ret_State = FALSE;
	SEG_XDATA U8 On_Time = FALSE;

		On_Time = Delay_Time_Get(TID_I2C_COMM_CHECK);
		if ( On_Time == TRUE)
		{
			if ( g_I2C_Err_Cnt > 0x00U )
			{
				g_I2C_Err_Cnt = 0x00U;
				DTC_ISP_CM_ERROR;
			}
			else
			{
				DTC_ISP_CM_CLEAR;
				DTC_ISP_IT_CLEAR;
			}
		}

}

void Comm_I2C_Tx(U8 Slave_Addr,U8 Length,U16 Addr, U16 TData,U16 RData )
{
	tI2C_Tx_Msg.Slave  		= Slave_Addr;
	tI2C_Tx_Msg.Length 		= Length;
	tI2C_Tx_Msg.Addr 		= Addr;
	tI2C_Tx_Msg.TxData 		= TData;
	tI2C_Tx_Msg.RxData 		= RData;
	tI2C_Tx_Msg.TRX_Flag	= TX;
	
	i2c_master_write(tI2C_Tx_Msg.Slave,tI2C_Tx_Msg.Addr,tI2C_Tx_Msg.TxData,tI2C_Tx_Msg.Length);
	//Comm_I2C_Tx_Data();
}

U16 Comm_I2C_Rx(U8 Slave_Addr,U8 Length,U16 Addr, U16 TData,U16 RData )
{
	SEG_XDATA U16 ret = 0x0000U;

	tI2C_Tx_Msg.Slave  		= Slave_Addr;
	tI2C_Tx_Msg.Length 		= Length;
	tI2C_Tx_Msg.Addr 		= Addr;
	tI2C_Tx_Msg.TxData 		= TData;
	tI2C_Tx_Msg.RxData 		= RData;
	tI2C_Tx_Msg.TRX_Flag	= RX;
	
	tI2C_Tx_Msg.RxData = i2c_master_read(tI2C_Tx_Msg.Slave,tI2C_Tx_Msg.Addr,tI2C_Tx_Msg.Length);
	//Comm_I2C_Tx_Data();
	ret = tI2C_Tx_Msg.RxData;

	return ret;		
}

void Clear_I2C_Tx_Data(void)
{
	tI2C_Tx_Msg.Slave  		= 0x00U;
	tI2C_Tx_Msg.Length 		= 0x00U;
	tI2C_Tx_Msg.Addr 		= 0x0000U;
	tI2C_Tx_Msg.TxData 		= 0x0000U;
	tI2C_Tx_Msg.RxData 		= 0x0000U;
	tI2C_Tx_Msg.TRX_Flag	= INIT;
}

void Clear_CAN_Tx_Data(void)
{
	tI2C_Tx_Msg.Slave  		= 0x00U;
	tI2C_Tx_Msg.Length 		= 0x00U;
	tI2C_Tx_Msg.Addr 		= 0x0000U;
	tI2C_Tx_Msg.TxData 		= 0x0000U;
	tI2C_Tx_Msg.RxData 		= 0x0000U;
	tI2C_Tx_Msg.TRX_Flag	= INIT;
}

void Clear_Can_Data(void)
{
	U8 i;
	for (i = 0; i< USE_CAN_COUNT; i++)
	{			   
		tCAN_Rx_Msg[i].Is_Change 	= 0x00U;
		tCAN_Rx_Msg[i].Change_Count = 0x00U;
		tCAN_Rx_Msg[i].Chatter_Time = 0x0000U;
		tCAN_Rx_Msg[i].Cur_Data		= 0xFFU;
		tCAN_Rx_Msg[i].Mid_Data		= 0xFFU;
		tCAN_Rx_Msg[i].Pre_Data		= 0xFFU;
	}

	tCAN_Rx_Msg[USE_G_SEL_DISP].Pre_Data		= GEAR_INVALID;
	tCAN_Rx_Msg[USE_IGN_SW].Pre_Data			= IGN_SW_KEY_OFF;
	tCAN_Rx_Msg[USE_RVM_CAMERAOFF].Pre_Data		= RVM_CAMERA_INVALID;
	tCAN_Rx_Msg[FCZC_RVM_SW].Pre_Data			= FCZC_RVM_SW_OFF;
}


void Clear_Sas_Data(void)
{	
	tCAN_Rx_SAS_Msg.Is_Change 		= 0x00U;
	tCAN_Rx_SAS_Msg.Pre_Sign 		= 0x00U;
	tCAN_Rx_SAS_Msg.Angle	 		= 0x00U;
	tCAN_Rx_SAS_Msg.Angle_Pre_Sign	= 0x00U;
	tCAN_Rx_SAS_Msg.Time_Out 		= 0x00U;
	tCAN_Rx_SAS_Msg.Pre_Data 		= 0x0000U;
	tCAN_Rx_SAS_Msg.Tuning_Type	= 0x00U;
}

void Flash_OneSpec_Write(void)
{
	SEG_XDATA volatile U8 ret,Cs = 0x00U;
	SEG_XDATA volatile U8 Write_Buf[USE_CAN_COUNT+1U] = {0,};
	SEG_XDATA volatile U8 Read_Buf[USE_CAN_COUNT+1U] = {0,};
	SEG_XDATA volatile FLADDR Write_Address = 0x0000U;
	SEG_XDATA volatile FLADDR Read_Address = 0x0000U;
	SEG_XDATA volatile FLADDR temp_Address1 = 0x0000U;
	SEG_XDATA volatile FLADDR temp_Address2 = 0x0000U;
	SEG_XDATA U8 i = 0x00U;
	SEG_XDATA U8 On_Time = FALSE;

	memset(Write_Buf, (char)0x00U ,(USE_CAN_COUNT+1U));
	memset(Read_Buf, (char)0x00U ,(USE_CAN_COUNT+1U));

	for (i = 0U; i < USE_CAN_COUNT; i++ )
	{
		Write_Buf[i] = tCAN_Rx_Msg[i].Pre_Data;
		Cs ^= Write_Buf[i];
	}
	Write_Buf[USE_CAN_COUNT] = Cs;
	FLASH_Read_Buf(Read_Buf,(FLADDR)ONESPEC_ADDRESS,1U,BANK2);

	FLASH_Erase_Buf((FLADDR)ONESPEC_ADDRESS,BANK2);
	if ( Read_Buf[0] == 0xFFU)
	{
		Read_Buf[0] = 0x00U;
	}
	temp_Address1 = (FLADDR)Read_Buf[0U]*(FLADDR)USE_CAN_COUNT;
	temp_Address2 = temp_Address1 + (FLADDR)1U;
	Write_Address = (FLADDR)ONESPEC_ADDRESS + temp_Address2;
	Read_Address  = Write_Address;

	Delay_Time_Set(TID_TEMP_TIMEOUT, DT_TEMP_TIMEOUT);

	while(1)
	{
		FLASH_Write_Buf(Write_Address,Write_Buf,sizeof(Write_Buf),BANK2);
		FLASH_Read_Buf(Read_Buf,Read_Address,sizeof(Read_Buf),BANK2);

		ret = (U8)memcmp(&Read_Buf,&Write_Buf,sizeof(Write_Buf));	
		if ( ret != 0x00U)
		{
			temp_Address1 = (FLADDR)USE_CAN_COUNT+(FLADDR)1U;

			Write_Address += temp_Address1;
			Read_Address  = Write_Address;
			if ( Write_Address >= (ONESPEC_ADDRESS+250U) )
			{
				Write_Address = ONESPEC_ADDRESS+1U;
				Read_Address  = ONESPEC_ADDRESS+1U;
			}
		}
		else
		{
			break;
		}

		On_Time = Delay_Time_Get(TID_TEMP_TIMEOUT);
		if (On_Time == TRUE)
		{
			break;
		}
	}
	Delay_Time_Expire(TID_TEMP_TIMEOUT);
}
/*
void Flash_OneSpec_Read(void)
{
	SEG_XDATA volatile U8 i,Cs = 0x00U;
	SEG_XDATA volatile U8 Read_Buf[USE_CAN_COUNT+1U] = {0,};
	SEG_XDATA volatile FLADDR Read_Address;
	SEG_XDATA volatile FLADDR temp_Address1;
	SEG_XDATA volatile FLADDR temp_Address2;

	memset(Read_Buf, (char)0x00U ,sizeof(Read_Buf));

	FLASH_Read_Buf(Read_Buf,(FLADDR)ONESPEC_ADDRESS,1U,BANK2);
	if ( Read_Buf[0] == 0xFFU)
	{
		Read_Buf[0] = 0x00U;
	}
	temp_Address1 = (FLADDR)Read_Buf[0U]*(FLADDR)USE_CAN_COUNT;
	temp_Address2 = temp_Address1 + (FLADDR)1U;
	Read_Address = (FLADDR)ONESPEC_ADDRESS + temp_Address2;

	FLASH_Read_Buf(Read_Buf,Read_Address,sizeof(Read_Buf),BANK2);
	for (i = 0U; i < USE_CAN_COUNT; i++ )
	{
		Cs ^= Read_Buf[i];
	}
	if ( Cs == Read_Buf[USE_CAN_COUNT]) //kwon
	{
		tCAN_Rx_Msg[USE_MDPS11].Pre_Data = Read_Buf[USE_MDPS11];
		tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data = Read_Buf[USE_LANGUAGEINFO];
		tCAN_Rx_Msg[USE_HUTYPE].Pre_Data = Read_Buf[USE_HUTYPE];
		tCAN_Rx_Msg[USE_GATEWAY].Pre_Data = Read_Buf[USE_GATEWAY];
		tCAN_Rx_Msg[USE_NAVIONOFF].Pre_Data = Read_Buf[USE_NAVIONOFF];
	}
}
*/
void Flash_OneSpec_Read(void)
{
	SEG_XDATA volatile U8 i,Cs = 0x00U;
	SEG_XDATA volatile U8 Read_Buf[USE_CAN_COUNT+1U];
	SEG_XDATA volatile FLADDR Read_Address = 0x0000U;

	memset(Read_Buf, (char)0x00U ,sizeof(Read_Buf));

	FLASH_Read_Buf(Read_Buf,(FLADDR)ONESPEC_ADDRESS,1U,BANK2);
	if ( Read_Buf[0] == 0xFFU)
	{
		Read_Buf[0] = 0x00U;
	}
	Read_Address = ONESPEC_ADDRESS+((FLADDR)Read_Buf[0U]*(FLADDR)USE_CAN_COUNT)+(FLADDR)1U;

	FLASH_Read_Buf(Read_Buf,Read_Address,sizeof(Read_Buf),BANK2);
	for (i = 0U; i < USE_CAN_COUNT; i++ )
	{
		Cs ^= Read_Buf[i];
	}
	if ( Cs == Read_Buf[USE_CAN_COUNT]) //kwon
	{
		tCAN_Rx_Msg[USE_MDPS11].Pre_Data = Read_Buf[USE_MDPS11];
		tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data = Read_Buf[USE_LANGUAGEINFO];
		tCAN_Rx_Msg[USE_HUTYPE].Pre_Data = Read_Buf[USE_HUTYPE];
		tCAN_Rx_Msg[USE_GATEWAY].Pre_Data = Read_Buf[USE_GATEWAY];
		tCAN_Rx_Msg[USE_NAVIONOFF].Pre_Data = Read_Buf[USE_NAVIONOFF];
		//tCAN_Rx_Msg[USE_ENGVOL].Pre_Data = Read_Buf[USE_ENGVOL]; //ENG_ONESPEC
		//tCAN_Rx_Msg[USE_4WDERR].Pre_Data = Read_Buf[USE_4WDERR]; //ENG_ONESPEC
	}
}