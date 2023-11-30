/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "global_define.h"
#include "drv_mcu.h"
#include "drv_timer.h"
#include "drv_pca.h"
#include "drv_spi.h"
#include "drv_i2c.h"
#include "drv_adc.h"
#include "drv_mem.h"
#include "drv_spi.h"
#include "mgr_diag.h"
#include "mgr_isp.h"
#include "mgr_comm.h"
#include "mgr_mcu.h"
#include "string.h"

#include "desc.h"
#include "nm_basic.h"
#include "il_inc.h"
#include "main.h"
/*--------------------------------------------------------------*/
SEG_XDATA tMsg_Ext_DTC_Type ga_Ext_tDTC_Type_Msg;

static SEG_XDATA volatile U8 DTC_Set_Off = FALSE;
static SEG_XDATA volatile U8 DTC_Write_Set = FALSE;
static SEG_XDATA volatile U8 DTC_Err_Sensor_Reset = FALSE;
static SEG_XDATA volatile U8 Calibration_Data[3U] = {0x00U,0x00U,0x00U};

SEG_XDATA volatile U8 CalibrationMode_OnOff = FALSE;
SEG_XDATA volatile U8 OpticOffset_Set = FALSE;
SEG_XDATA volatile U8 IGN_ON_Status = IGN_CLEAR;

extern SEG_XDATA U8 EEPROM_Update_Mode;
extern SEG_XDATA volatile U8 Optical_Axis_Data[9U];

extern U16 IlGetRxSAS_Angle(void);

void Init_DiagTask(void)
{	
	Clear_DTC_Struct();	
	Flash_DTC_Read();
	IGN_ON_Status = IGN_CLEAR;

#if defined(DIAG_TEST_MCU_WD_ERR)
	ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_WT_ERR].Status = DTC_CONFIRM_ERROR;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR] = DTC_CONFIRM_ERROR;
	DTC_Write_Set = TRUE;
#endif

#if defined(DIAG_TEST_MCU_MC_ERR)
	ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_MC_ERR].Status = DTC_CONFIRM_ERROR;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR] = DTC_CONFIRM_ERROR;
	DTC_Write_Set = TRUE;
#endif

#if defined(DIAG_TEST_ISP_FV_ERR)
	ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FV_ERR].Status = DTC_CURRENT_ERROR;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = DTC_CURRENT_ERROR;
	DTC_Write_Set = TRUE;
#endif

#if defined(DIAG_TEST_ISP_FC_ERR)
	ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FC_ERR].Status = DTC_CURRENT_ERROR;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = DTC_CURRENT_ERROR;
	DTC_Write_Set = TRUE;
#endif

#if defined(DIAG_TEST_ISP_CM_ERR)
	ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_COMM_ERR].Status = DTC_CURRENT_ERROR;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = DTC_CURRENT_ERROR;
	DTC_Write_Set = TRUE;
#endif

#if defined(DIAG_TEST_ISP_IS_ERR)
	ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_ISPSTAT_ERR].Status = DTC_CURRENT_ERROR;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = DTC_CURRENT_ERROR;
	DTC_Write_Set = TRUE;
#endif

#if defined(DIAG_TEST_ISP_IT_ERR)
	ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENOSR_INIT_ERR].Status = DTC_CURRENT_ERROR;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = DTC_CURRENT_ERROR;
	DTC_Write_Set = TRUE;
#endif
}

void Clear_DTC_Struct(void)
{
	U8 i = 0U;
	for(i = 0; i < DTC_CODE_COUNT;i++)
	{
		ga_Ext_tDTC_Type_Msg.tDTC_Type[i].Status = 0xFFU;
		ga_Ext_tDTC_Type_Msg.tDTC_Type[i].Count = 0x00U;
	}
	ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_INTERNAL_ERROR] = 0x00U;
	ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_SENSOR_ERROR] = 0x00U;
	ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_OUTPUT_ERROR] = 0x00U;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR] = FALSE;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = FALSE;
	ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_OUTPUT_ERROR] = FALSE;
}

void Clear_DTC_Status(void)
{
	U8 i = 0U;
	for(i = DTC_SENSOR_FV_ERR; i < DTC_CODE_COUNT;i++)
	{
		ga_Ext_tDTC_Type_Msg.tDTC_Type[i].Status = 0xFFU;
		ga_Ext_tDTC_Type_Msg.tDTC_Type[i].Count = 0x00U;
	}
}

void Operate_DiagTask(void)
{
	SEG_XDATA U8 ID = SYS_MANU_DATE;
	SEG_XDATA U8 On_Time = FALSE;
	U8 Reserved_Address = 0U;
	U8 i = 0U;

	WDT_Clear();

	On_Time = Delay_Time_Get(TID_DIAG_ECU_RESET);
	if ( On_Time == TRUE)
	{
		MCU_Reset();
	}
	WDT_Clear();
	On_Time = Delay_Time_Get(TID_FIRME_UPDATE);
	if ( On_Time == TRUE)
	{
		PSBANK = 0x30;
		Reserved_Address = *(U8 SEG_CODE*)(0xFC00U);
	}
	WDT_Clear();

	On_Time = Delay_Time_Get(TID_DTC_WRITE);
	if ( On_Time == TRUE)
	{
		if ( DTC_Write_Set == TRUE)
		{
			Flash_DTC_Write();
			DTC_Write_Set = FALSE;
		}

		if (DTC_Err_Sensor_Reset == TRUE) 
		{
			DTC_Err_Sensor_Reset = FALSE;
			//Delay_Time_Set(TID_SENSOR_RESET,DT_SENSOR_RESET);
			Delay_Time_Expire(TID_OVERLAY_GUIDELINE);
			//Delay_Time_Expire(TID_SENSOR_IDLE_PERIOD);
			Delay_Time_Expire(TID_FRAME_VALIDE);
			Delay_Time_Expire(TID_FRAME_COUNT);
			Delay_Time_Expire(TID_I2C_COMM_CHECK);

			for (i = DTC_SENSOR_FV_ERR; i < DTC_CODE_COUNT; i++)
			{
				ga_Ext_tDTC_Type_Msg.tDTC_Type[i].Count = 0x00U;
			}

			SMBUS0_Init();
			Isp_InterInit();
		}
	}
	

	if (IGN_ON_Status == IGN_ON)
	{
		if ( ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR] == DTC_CONFIRM_ERROR )
		{
			ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_INTERNAL_ERROR]++;
			if ( ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_INTERNAL_ERROR] > 30U)
			{
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_WT_ERR].Status = DTC_CLEAR;
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_MC_ERR].Status = DTC_CLEAR;
	
				ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_INTERNAL_ERROR] = 0x00U;
				ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR] = DTC_CLEAR;
			}
			DTC_Write_Set = TRUE;
		}	

		if ( ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] == DTC_CONFIRM_ERROR )
		{
			ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_SENSOR_ERROR]++;
			if ( ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_SENSOR_ERROR] > 30U)
			{
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FV_ERR].Status = DTC_CLEAR;
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FC_ERR].Status = DTC_CLEAR;
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_ISPSTAT_ERR].Status = DTC_CLEAR;
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_COMM_ERR].Status = DTC_CLEAR;
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENOSR_INIT_ERR].Status = DTC_CLEAR;

				ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_SENSOR_ERROR] = 0x00U;
				ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = DTC_CLEAR;
			}
			DTC_Write_Set = TRUE;
		}	

		if ( ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_OUTPUT_ERROR] == DTC_CONFIRM_ERROR )
		{
			ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_OUTPUT_ERROR]++;
			if ( ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_OUTPUT_ERROR] > 30U)
			{
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_VIDEO_OUT_CUT_ERR].Status = DTC_CLEAR;
				ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_VIDEO_OVER_VOL_ERR].Status = DTC_CLEAR;
	
				ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_OUTPUT_ERROR] = 0x00U;
				ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_OUTPUT_ERROR] = DTC_CLEAR;
			}
			DTC_Write_Set = TRUE;
		}	
		IGN_ON_Status = IGN_PRE_ON;
	}
}

void Record_DTC_Error(U8 ID)
{
	SEG_XDATA U8 is_error = FALSE;

	if (DTC_Set_Off == FALSE)
	{
		if (ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Status != DTC_CURRENT_ERROR )
		{
			ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Count++;
			if ( ID == DTC_MCU_WT_ERR ) 
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_WT_ERR].Count > 30U ){ is_error = TRUE;}
			}
			if ( ID == DTC_MCU_MC_ERR )
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_MC_ERR].Count > 30U ){ is_error = TRUE;}
			}
			if ( ID == DTC_SENSOR_FV_ERR ) 
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FV_ERR].Count > 19U ){ is_error = TRUE;}
			}
			if ( ID == DTC_SENSOR_FC_ERR )
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FC_ERR].Count > 19U ){ is_error = TRUE;}
			}
			if ( ID == DTC_SENSOR_ISPSTAT_ERR ) 
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_ISPSTAT_ERR].Count > 19U ){ is_error = TRUE;}
			}
			if ( ID == DTC_SENSOR_COMM_ERR )
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_COMM_ERR].Count > 19U ){ is_error = TRUE;}
			}
			if ( ID == DTC_SENOSR_INIT_ERR ) 
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENOSR_INIT_ERR].Count > 0U ){ is_error = TRUE;}
			}
			if ( ID == DTC_VIDEO_OUT_CUT_ERR )
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_VIDEO_OUT_CUT_ERR].Count > 0U ){ is_error = TRUE;}
			}
			if ( ID == DTC_VIDEO_OVER_VOL_ERR )
			{
				if (ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_VIDEO_OVER_VOL_ERR].Count > 0U ){ is_error = TRUE;}
			}	
			
			if ( is_error == TRUE)
			{
				WDT_Clear();
				ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Count = 0x00U;
				if ( (ID == DTC_MCU_WT_ERR) || (ID == DTC_MCU_MC_ERR) )
				{
					ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Status = DTC_CONFIRM_ERROR;
					ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR] = DTC_CONFIRM_ERROR;
					ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_INTERNAL_ERROR] = 0x00U;
				}
				else
				{
					ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Status = DTC_CURRENT_ERROR;
					if ( (ID == DTC_SENSOR_FV_ERR) || (ID == DTC_SENSOR_FC_ERR) ||
					     (ID == DTC_SENSOR_ISPSTAT_ERR) || (ID == DTC_SENSOR_COMM_ERR) || (ID == DTC_SENOSR_INIT_ERR) )
					{
						if ((DTC_Err_Sensor_Reset == FALSE) &&
						   (ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] != DTC_CURRENT_ERROR))
						{
							DTC_Err_Sensor_Reset = TRUE;
						}

						ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = DTC_CURRENT_ERROR;
						ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_SENSOR_ERROR] = 0x00U;
					}
					else if ( (ID == DTC_VIDEO_OUT_CUT_ERR) || (ID == DTC_VIDEO_OVER_VOL_ERR) )
					{
						ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_OUTPUT_ERROR] = DTC_CURRENT_ERROR;
						ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_OUTPUT_ERROR] = 0x00U;
					}
					else{}
				}
				DTC_Write_Set = TRUE;
			}
		}
	}
}

void Record_DTC_Clear(U8 ID)
{
#if defined(DTC_RECORD_CLEAR)
	
	if (ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Status == DTC_CLEAR )
	{
		if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Count != 0x00U)
		{
			ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Count = 0x00U;
			//DTC_Write_Set = TRUE;
		}
	}	
	else if (ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Status == DTC_CURRENT_ERROR )// && tDTC_Type[ID].Status == DTC_CONFIRM_ERROR)
	{
		ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Count = 0x00U;
		ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Status = DTC_CONFIRM_ERROR;
		
		if ( (ID == DTC_MCU_WT_ERR) || (ID == DTC_MCU_MC_ERR) )
		{
			ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_INTERNAL_ERROR] = 0x00U;
			ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR] = DTC_CONFIRM_ERROR;
		}
		else if ( (ID == DTC_SENSOR_FV_ERR) || (ID == DTC_SENSOR_FC_ERR) ||
		     (ID == DTC_SENSOR_ISPSTAT_ERR) || (ID == DTC_SENSOR_COMM_ERR) || (ID == DTC_SENOSR_INIT_ERR) )
		{
			ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_SENSOR_ERROR] = 0x00U;
			ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] = DTC_CONFIRM_ERROR;
		}
		else if ( (ID == DTC_VIDEO_OUT_CUT_ERR) || (ID == DTC_VIDEO_OVER_VOL_ERR) )
		{
			ga_Ext_tDTC_Type_Msg.DTC_Clear_Count[DTC_CODE_OUTPUT_ERROR] = 0x00U;
			ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_OUTPUT_ERROR] = DTC_CONFIRM_ERROR;
		}
		else{}
		DTC_Write_Set = TRUE;
	}	
	else
	{
		ga_Ext_tDTC_Type_Msg.tDTC_Type[ID].Count = 0x00U;
	}
#endif
}
void Diag_Ecu_Reset(void)
{
	EEPROM_Update_Mode = FALSE;
	Delay_Time_Set(TID_DIAG_ECU_RESET,DT_DIAG_ECU_RESET);
}

void Diag_Clear_DTC(void)
{	
	FLASH_Erase_Buf((FLADDR)DTCCODE_ADDRESS,BANK2);
	Clear_DTC_Struct();
}

U8 Diag_Read_Dtc(U8 index)
{
	SEG_XDATA U8 ret = 0U;
	if ( index == 0x01U)
	{
		if ( ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR] != 0xFFU )
		{
			ret = ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_INTERNAL_ERROR];
		}
		else
		{
			ret = FALSE;
		}
	}
	if (index == 0x02U)
	{
		if ( ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR] != 0xFFU )
		{
			ret = ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_SENSOR_ERROR];
		}
		else
		{
			ret = FALSE;
		}
	}
	if ( index == 0x03U )
	{
		if (  ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_OUTPUT_ERROR] != 0xFFU )
		{
			ret = ga_Ext_tDTC_Type_Msg.DTC_State[DTC_CODE_OUTPUT_ERROR];
		}
		else
		{
			ret = FALSE;
		}
	}

	return ret;
}

void Diag_Read_Sys(U8 ID, U8 *Buf,U8 Length)
{
	U8 ret = 0U;
	U16 ret16 = 0x0000;
	SEG_XDATA U8 Flash_Data[3U] = {0,};
	SEG_XDATA U8 temp_buf[HMC_SPEC_TOTAL_COUNT] = {0,};

	memset(Buf, (char)0x00 ,Length);

	if ( ID == 0x02U )
	{
		Buf[0] = USE_CASE_1BYTE;
		Buf[1] = USE_CASE_2BYTE;
		Buf[2] = USE_CASE_3BYTE;
		Buf[3] = USE_CASE_4BYTE;
	
		Flash_Data[0] = Optical_Axis_Data[0] + Optical_Axis_Data[6];
		Flash_Data[1] = Optical_Axis_Data[1] + Optical_Axis_Data[7];

		if (Flash_Data[0] < 0x80U) {
		Buf[HMC_SPEC_OPTIC_X_HIGH_PID] = 0x00U;
		} else {
			Buf[HMC_SPEC_OPTIC_X_HIGH_PID] = 0xFFU;
		}
		Buf[HMC_SPEC_OPTIC_X_LOW_PID]  = Flash_Data[0];

		if (Flash_Data[1] < 0x80U) {
		Buf[HMC_SPEC_OPTIC_Y_HIGH_PID] = 0x00U;
		} else {
			Buf[HMC_SPEC_OPTIC_Y_HIGH_PID] = 0xFFU;
		}
		Buf[HMC_SPEC_OPTIC_Y_LOW_PID]  = Flash_Data[1];

		if ( (Optical_Axis_Data[3U] == 0xFFU) && (Optical_Axis_Data[4U] == 0xFFU) )
		{
			if ( Optical_Axis_Data[5U] == 0xFFU )
			{
				//Default
				Optical_Axis_Data[3U] = 0x00U;
				Optical_Axis_Data[4U] = 0x00U;
			}
		}
		Buf[HMC_SPEC_RELATIVE_OPTIC_X_PID]  = Optical_Axis_Data[3U];
		Buf[HMC_SPEC_RELATIVE_OPTIC_Y_PID]  = Optical_Axis_Data[4U];

		Buf[HMC_SPEC_COUNTRY_CFG_PID] = tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data;
		Buf[HMC_SPEC_TEMPERATURE_PID] = Get_Temperature();//¿Âµµ °è»ê 

		ret16 = (U16)IlGetRxSAS_Angle();

		Buf[HMC_SPEC_SAS_ANGLE_HIGH_PID] = (U8)(ret16 >> 8U ) & 0xFFU;
		Buf[HMC_SPEC_SAS_ANGLE_LOW_PID] = (U8)ret16 & 0xFFU;

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_WT_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_LOW_PID] |= 0x01U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_WT_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_LOW_PID] |= 0x01U;}
		else{}

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_MC_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_LOW_PID] |= 0x02U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_MCU_MC_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_LOW_PID] |= 0x02U;}
		else{}

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FV_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_LOW_PID] |= 0x04U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FV_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_LOW_PID] |= 0x04U;}
		else{}

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FC_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_LOW_PID] |= 0x08U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_FC_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_LOW_PID] |= 0x08U;}
		else{}

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_ISPSTAT_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_LOW_PID] |= 0x10U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_ISPSTAT_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_LOW_PID] |= 0x10U;}
		else{}

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_COMM_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_LOW_PID] |= 0x20U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_COMM_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_LOW_PID] |= 0x20U;}
		else{}

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENOSR_INIT_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_LOW_PID] |= 0x40U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENOSR_INIT_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_LOW_PID] |= 0x40U;}
		else{}

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_VIDEO_OUT_CUT_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_LOW_PID] |= 0x80U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_VIDEO_OUT_CUT_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_LOW_PID] |= 0x80U;}
		else{}

		if      ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_VIDEO_OVER_VOL_ERR].Status == DTC_CURRENT_ERROR) {Buf[HMC_SPEC_DTC_CURRENT_HIGH_PID] |= 0x01U;}
		else if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_VIDEO_OVER_VOL_ERR].Status == DTC_CONFIRM_ERROR) {Buf[HMC_SPEC_DTC_CONFIRM_HIGH_PID] |= 0x01U;}
		else{}
	}
	else
	{
		SYS_Flash_Read(ID,Buf,Length);
	}
	ret = 0x04;
	memcpy(temp_buf,Buf,Length);
}

void Firmware_Update(void)
{
	Delay_Time_Set(TID_FIRME_UPDATE,DT_FIRME_UPDATE);
}

void Diag_Eeprom_Update(void)
{
	Isp_Off();
	EEPROM_Update_Mode = TRUE;
	MCU_F_ENABLE = 0x01U;

	EIE1 &= (U8)(~0x04U);                       // Enable ADC0 conversion complete int.
	EIE1 &= (U8)(~0x01U);
}

void Diag_Write_Transfer_Data(U8 *Buf)
{
	SEG_XDATA U32	 address = (U32)0x00U;
	static SEG_XDATA U32 cnt   = (U32)0x00U;
	static SEG_XDATA U32 erase_cnt   = (U32)0x00U;
	static SEG_XDATA U32 lotation = (U32)0x01U;
	static SEG_XDATA volatile U8 Buf1[256] = {0U,};
	static SEG_XDATA volatile U8 Buf2[256] = {0U,};
	static SEG_XDATA volatile U8 buf1_index = 253U;
	static SEG_XDATA volatile U8 buf2_index = 3U;
	static SEG_XDATA volatile U8 loop_index = 0U;
	SEG_XDATA U8 Write_Buf[256] = {0U,};

	if ((cnt % (U32)16) == (U32)0)
	{
		Flash_EraseSector(erase_cnt*(U32)0x1000U);	
		erase_cnt++;
		Wait_ms(100U);
	}
	if ( cnt == 0x00U)
	{
		memset(Buf1, (char)0xFFU ,256U);	
		memset(Buf2, (char)0xFFU ,256U);	
		memcpy(Buf1,&Buf[1U],253U);
	}
	else
	{
		if ( loop_index == 0U)
		{
			if ( buf1_index == 1U)
			{
				memcpy(&Buf1[buf1_index],&Buf[1U],253U);
				//memcpy(Buf1,Buf,256U);
				buf1_index = 254U;
				buf2_index = 2U;
				loop_index = 1U;
				lotation++;
			}
			else
			{
				memcpy(Buf2,&Buf[1U],253U);
				memcpy(&Buf1[buf1_index],Buf2,buf2_index);
				
				address = (cnt-lotation)*(U32)256;
				memcpy(Write_Buf,Buf1,256U);

				Flash_Write(address,Write_Buf,(U16)256);

				memset(Buf1, (char)0xFFU ,256U);	
				memcpy(Buf1,&Buf2[buf2_index],buf1_index);
				memset(Buf2, (char)0xFFU ,256U);	
				buf1_index-=3U;
				buf2_index+=3U;
			}
		}
		
		else if ( loop_index == 1U)
		{
			if ( buf1_index == 2U)
			{
				memcpy(&Buf1[buf1_index],(Buf+1U),253U);
				buf1_index = 255U;
				buf2_index = 1U;
				loop_index = 2U;
				lotation++;
			}
			else
			{	
				memcpy(Buf2,&Buf[1U],253U);
				memcpy(&Buf1[buf1_index],Buf2,buf2_index);
				
				address = (cnt-lotation)*(U32)256;
				memcpy(Write_Buf,Buf1,256U);

				Flash_Write(address,Write_Buf,(U16)256);

				memset(Buf1, (char)0xFFU ,256U);	
				memcpy(Buf1,&Buf2[buf2_index],buf1_index);
				memset(Buf2, (char)0xFFU ,256U);	
				buf1_index-=3U;
				buf2_index+=3U;
			}
				
		}
		else if ( loop_index == 2U )
		{
			if ( buf1_index == 0U)
			{
				memcpy(&Buf1[buf1_index],&Buf[1U],253U);
				buf1_index = 253U;
				buf2_index = 3U;
				loop_index = 0U;
				lotation++;
			}
			else
			{
				memcpy(Buf2,&Buf[1U],253U);
				memcpy(&Buf1[buf1_index],Buf2,buf2_index);
				
				address = (cnt-lotation)*(U32)256;
				memcpy(Write_Buf,Buf1,256U);

				Flash_Write(address,Write_Buf,(U16)256);

				memset(Buf1, (char)0xFFU ,256U);	
				memcpy(Buf1,&Buf2[buf2_index],buf1_index);
				memset(Buf2, (char)0xFFU ,256U);	
				buf1_index-=3U;
				buf2_index+=3U;
			}
		}
		else
		{
		}
		
	}
	cnt++;
}

void Diag_Sensor_Reset(void)
{
	Init_IspTask();
}

void Diag_Guid_OnOff(U8 OnOff)
{
	isp_guide_on_off(OnOff);
}

void Diag_Calibration_Mode_start(void)
{
	DIAG_CALIBRATION_MODE_ACTIVE(TRUE);
	
	memset(Optical_Axis_Data, (char)0xFFU ,9U);

	FLASH_Read_Buf(Optical_Axis_Data,(FLADDR)OPTIC_ADDRESS,9U,BANK2);

	if ( (Optical_Axis_Data[0] == 0xFFU) && (Optical_Axis_Data[1] == 0xFFU) )
	{
		if ( Optical_Axis_Data[2] == 0xFFU )
		{
			//Default
			Optical_Axis_Data[0] = 0x00U;
			Optical_Axis_Data[1] = 0x00U;
		}
	}

	if ( (Optical_Axis_Data[6] == 0xFFU) && (Optical_Axis_Data[7] == 0xFFU) )
	{
		if ( Optical_Axis_Data[8] == 0xFFU )
		{
			//Default
			Optical_Axis_Data[6] = 0x00U;
			Optical_Axis_Data[7] = 0x00U;
		}
	}

	//memcpy(Calibration_Data, &Optical_Axis_Data[6],3U);
}

void Diag_Calibration_Mode(U8 Ctrl_Para)
{
	SEG_XDATA volatile U8 i,Cs = 0U;
	SEG_XDATA volatile U8 X_Axis;
	SEG_XDATA volatile U8 Y_Axis;

	if (DIAG_CALIBRATION_MODE_STATE == TRUE)
	{
		switch (Ctrl_Para)
		{
			case CALIBRATION_MODE_START:
				Diag_Calibration_Mode_start();
				DIAG_OPTIC_OFFSET_SET_ACTIVE(TRUE);
				Diag_DTC_Set_Off();
				break;

			case CALIBRATION_MODE_STOP: // Save data & Mode Stop
				FLASH_Erase_Buf((FLADDR)OPTIC_ADDRESS, BANK2);
				FLASH_Write_Buf((FLADDR)OPTIC_ADDRESS, Optical_Axis_Data, 0x09U, BANK2);

				DIAG_OPTIC_OFFSET_SET_ACTIVE(TRUE);
				DIAG_CALIBRATION_MODE_ACTIVE(FALSE);
				Diag_DTC_Set_On();
				break;

			case CALIBRATION_MOVE_DEFAULT:
				Calibration_Data[0] = 0x00U;
				Calibration_Data[1] = 0x00U;

				break;

			case CALIBRATION_MOVE_LEFT:
				X_Axis = Optical_Axis_Data[0] + Optical_Axis_Data[6];

				if ((X_Axis <= OPTIC_X_LIMIT_MINUS) && (X_Axis >= 0x80U))
				{
					X_Axis = OPTIC_X_LIMIT_MINUS - Optical_Axis_Data[0];
					Calibration_Data[0] = X_Axis;
				}
				else
				{
					Calibration_Data[0] = Optical_Axis_Data[6] - 0x01U;
				}
				Calibration_Data[1] = Optical_Axis_Data[7];

				break;

			case CALIBRATION_MOVE_RIGHT:
				X_Axis = Optical_Axis_Data[0] + Optical_Axis_Data[6];

				if ((X_Axis >= OPTIC_X_LIMIT_PLUS) && (X_Axis < 0x80U))
				{
					X_Axis = OPTIC_X_LIMIT_PLUS - Optical_Axis_Data[0];
					Calibration_Data[0] = X_Axis;
				}
				else
				{
					Calibration_Data[0] = Optical_Axis_Data[6] + 0x01U;
				}
				Calibration_Data[1] = Optical_Axis_Data[7];

				break;

			case CALIBRATION_MOVE_UP:
				Y_Axis = Optical_Axis_Data[1] + Optical_Axis_Data[7];

				if ((Y_Axis >= OPTIC_Y_LIMIT_PLUS) && (Y_Axis < 0x80U))
				{
					Y_Axis = OPTIC_Y_LIMIT_PLUS - Optical_Axis_Data[1];
					Calibration_Data[1] = Y_Axis;
				}
				else
				{
					Calibration_Data[1] = Optical_Axis_Data[7] + 0x01U;
				}
				Calibration_Data[0] = Optical_Axis_Data[6];

				break;

			case CALIBRATION_MOVE_DOWN:				
				Y_Axis = Optical_Axis_Data[1] + Optical_Axis_Data[7];

				if ((Y_Axis <= OPTIC_Y_LIMIT_MINUS) && (Y_Axis >= 0x80U))
				{
					Y_Axis = OPTIC_Y_LIMIT_MINUS - Optical_Axis_Data[1];
					Calibration_Data[1] = Y_Axis;
				}
				else
				{
					Calibration_Data[1] = Optical_Axis_Data[7] - 0x01U;
				}
				Calibration_Data[0] = Optical_Axis_Data[6];

				break;

			default:

				break;
		}
		
		if ((Ctrl_Para >= CALIBRATION_MOVE_DEFAULT) && (Ctrl_Para <= CALIBRATION_MOVE_DOWN))
		{
			for (i = 0; i < 2U; i++)
			{
				Cs ^= Calibration_Data[0 + i];
			}
			Calibration_Data[2] = Cs;

			memcpy(&Optical_Axis_Data[6],&Calibration_Data[0],3U);

			DIAG_OPTIC_OFFSET_SET_ACTIVE(TRUE);
		}	
	}
	else
	{
		if (Ctrl_Para == CALIBRATION_MODE_START)
		{
			Diag_Calibration_Mode_start();
			Diag_DTC_Set_Off();
		}
	}
}

void Diag_Exit_Update(void)
{
	//MCU_Reset();
}

void Diag_DTC_Set_On(void)
{
	DTC_Set_Off = FALSE;
}

void Diag_DTC_Set_Off(void)
{
	DTC_Set_Off = TRUE;
}

void Flash_DTC_Write(void)
{
	SEG_XDATA volatile U8 ret,i,Cs = 0x00U;
	SEG_XDATA volatile U8 Write_Buf[sizeof(tMsg_Ext_DTC_Type)+1U] = {0,};
	SEG_XDATA volatile U8 Read_Buf[sizeof(tMsg_Ext_DTC_Type)+1U] = {0,};
	SEG_XDATA FLADDR Write_Address = 0x0000U;
	SEG_XDATA FLADDR Read_Address = 0x0000U;
	SEG_XDATA FLADDR temp_Address1 = 0x0000U;
	SEG_XDATA FLADDR temp_Address2 = 0x0000U;

	memset(Write_Buf,0x00U,sizeof(Write_Buf));
	memset(Read_Buf,0x00U,sizeof(Read_Buf));

	memcpy(Write_Buf,&ga_Ext_tDTC_Type_Msg,sizeof(tMsg_Ext_DTC_Type));

	for (i = 0U; i < sizeof(tMsg_Ext_DTC_Type); i++ )
	{
		Cs ^= Write_Buf[i];
	}
	Write_Buf[sizeof(tMsg_Ext_DTC_Type)] = Cs;

	FLASH_Read_Buf(Read_Buf,(FLADDR)DTCCODE_ADDRESS,1U,BANK2);
	FLASH_Erase_Buf((FLADDR)DTCCODE_ADDRESS,BANK2);

	if ( Read_Buf[0] == 0xFFU)
	{
		Read_Buf[0] = 0x00U;
	}

	temp_Address1 = (FLADDR)Read_Buf[0U]*(FLADDR)sizeof(tMsg_Ext_DTC_Type);
	temp_Address2 = temp_Address1 + (FLADDR)1U;
	Write_Address = (FLADDR)(DTCCODE_ADDRESS + temp_Address2);
	Read_Address = Write_Address;

	while(1)
	{
		FLASH_Write_Buf(Write_Address,Write_Buf,sizeof(tMsg_Ext_DTC_Type)+1U,BANK2);
		FLASH_Read_Buf(Read_Buf,Read_Address,sizeof(tMsg_Ext_DTC_Type)+1U,BANK2);

		ret = (U8)memcmp(&Read_Buf,&Write_Buf,sizeof(Write_Buf));	
		if ( ret != 0x00U)
		{
			temp_Address1 = (FLADDR)sizeof(tMsg_Ext_DTC_Type)+(FLADDR)1U;
			Write_Address += temp_Address1;
			Read_Address = Write_Address;
				
			if ( Write_Address >= (DTCCODE_ADDRESS+236U) )
			{
				Write_Address = DTCCODE_ADDRESS+1U;
				Read_Address  = DTCCODE_ADDRESS+1U;
			}
		}
		else
		{
			break;
		}
	}
}
/*---This function use for Read DTC data from flash--------------*/ 
/*
	*Read data from Flash, 
	*Process and check data integrity, 
	*Then copy data to variable ga_Ext_tDTC_Type_Msg if data is valid.
*/
void Flash_DTC_Read(void)
{
	SEG_XDATA volatile U8 i,Cs = 0x00U;

    // Khai báo mang volatile có kích thuoc là sizeof(tMsg_Ext_DTC_Type)+1U và khoi tao gia tri là 0
	SEG_XDATA volatile U8 Read_Buf[sizeof(tMsg_Ext_DTC_Type)+1U] = {0,};

    // Khai báo bien Read_Address và hai bien tam temp_Address1, temp_Address2 là bien cuc bo
	SEG_XDATA volatile FLADDR Read_Address = 0x0000U;
	SEG_XDATA FLADDR temp_Address1 = 0x0000U;
	SEG_XDATA FLADDR temp_Address2 = 0x0000U;
	//FLADDR : 0 ~ 0xFFFF (unsigned int) 
    // Ðat tat ca các byte trong mang Read_Buf va giá tri 0
	memset(Read_Buf, (char)0x00U ,sizeof(tMsg_Ext_DTC_Type)+1U);

    // Ðac mot byte tu dia chi DTCCODE_ADDRESS trong bo nho Flash vào mang Read_Buf
	//DTCCODE_ADDRESS = 0xFF00U;
	FLASH_Read_Buf(Read_Buf,(FLADDR)DTCCODE_ADDRESS,1U,BANK2);

    // Neu byte doc duoc là 0xFFU, gán giá tri 0 cho byte do
	if ( Read_Buf[0] == 0xFFU)
	{
		Read_Buf[0] = 0x00U;
	}

    // Tính toán dia chi temp_Address1 và temp_Address2

	temp_Address1 = (FLADDR)Read_Buf[0U]*(FLADDR)sizeof(tMsg_Ext_DTC_Type);
	temp_Address2 = temp_Address1 + (FLADDR)1U;
    // Tính toán ð?a ch? Read_Address
	Read_Address = (FLADDR)DTCCODE_ADDRESS + temp_Address2;
	// = fc25 
    // Ð?c m?t kh?i d? li?u t? ð?a ch? Read_Address trong b? nh? Flash vào m?ng Read_Buf
	FLASH_Read_Buf(Read_Buf,Read_Address,sizeof(Read_Buf),BANK2);

    // Tính toán giá tr? XOR c?a t?t c? các byte trong m?ng Read_Buf, k?t qu? lýu vào bi?n Cs
	for (i = 0U; i < sizeof(tMsg_Ext_DTC_Type); i++ )
	{
		Cs ^= Read_Buf[i];
	}

    // Ki?m tra xem giá tr? tính ðý?c t? XOR có b?ng giá tr? cu?i cùng trong m?ng Read_Buf hay không
	if ( Cs == Read_Buf[sizeof(tMsg_Ext_DTC_Type)])
	{
        // N?u b?ng, sao chép d? li?u t? m?ng Read_Buf vào bi?n ga_Ext_tDTC_Type_Msg
		memcpy(&ga_Ext_tDTC_Type_Msg,Read_Buf,sizeof(tMsg_Ext_DTC_Type));
	}
}
void Diag_Write_Sys(U8 *Write_Buf,U8 Length)
{
	SEG_XDATA volatile U8 i,Cs,Cs_size,len;
	SEG_XDATA volatile U8 Buf[SYS_WRITE_CODE_SIZE] = {0,};
	
	len = Length;
	Cs_size = SYS_WRITE_CODE_SIZE-1U;
	FLASH_Erase_Buf((FLADDR)SYSINFO_ADDRESS,BANK2);
	memset(Buf, (char)0x00U ,SYS_WRITE_CODE_SIZE);
	memcpy(&Buf[0U],Write_Buf,SYS_WRITE_CODE_SIZE);

	for (i = 0; i < Cs_size; i++)
	{
		Cs ^= Buf[i];
	}
	Buf[Cs_size] = Cs;
	FLASH_Write_Buf((FLADDR)SYSINFO_ADDRESS,Buf,SYS_WRITE_CODE_SIZE,BANK2);

}

void Diag_Extra_Data_Process(U8 *Write_Buf)
{
	SEG_XDATA volatile U8 i,Cs;
	SEG_XDATA volatile U8 Buf[6] = {0,};
	
	if ( Write_Buf[0] == OPTIC_SETTING )
	{
		FLASH_Erase_Buf((FLADDR)OPTIC_ADDRESS,BANK2);

		memset(Buf, (char)0x00U ,6U);
		memcpy(Buf,&Write_Buf[1U],2U);
		memcpy(&Buf[3],&Write_Buf[3U],2U);

		for (i = 0; i < 2U; i++)
		{
			Cs ^= Buf[i];
		}
		Buf[2U] = Cs;

		for (i = 3; i < 5U; i++)
		{
			Cs ^= Buf[i];
		}
		Buf[5U] = Cs;

		FLASH_Write_Buf((FLADDR)OPTIC_ADDRESS,Buf,0x06U,BANK2);
		Diag_Ecu_Reset();
	}
	else if ( Write_Buf[0] == ERASE_ONESPEC )
	{
		FLASH_Erase_Buf((FLADDR)ONESPEC_ADDRESS,BANK2);
		Diag_Ecu_Reset();
	}
	else if ( Write_Buf[0] == ERASE_FLASH_ALL )
	{
		Isp_Off();
		EEPROM_Update_Mode = TRUE;
		MCU_F_ENABLE = 0x01;
		Wait_ms(30U);

		for ( i = 0 ; i < 50U; i++)
		{
			Flash_EraseSector((U32)i*(U32)0x1000U);	
			WDT_Clear();
		}
	}
	else{}
}

void SYS_Flash_Read(U8 ID,U8 *Read_Buf,U8 Length)
{
	SEG_XDATA volatile U8 i,Cs;
	SEG_XDATA volatile U8 Buf[SYS_CODE_SIZE+1U] = {0,};

	memset(Buf, (char)0x00U ,Length);

	FLASH_Read_Buf(Buf,(FLADDR)SYSINFO_ADDRESS,sizeof(Buf),BANK2);

	for (i = 0; i < SYS_CODE_SIZE; i++)
	{
		Cs ^= Buf[i];
	}
	Cs = Buf[SYS_CODE_SIZE];
	if ( Cs == Buf[SYS_CODE_SIZE])
	{
		switch(ID)
		{
			case SYS_HMC_SPEC:
				memcpy(Read_Buf,&Buf[0U],Length);
			break;
			case SYS_PART_NUMBER:
				memcpy(Read_Buf,&Buf[SYS_PART_NUMBER_L],Length);
			break;
			case SYS_MANU_DATE:
				memcpy(Read_Buf,&Buf[SYS_MANU_DATE_L],Length);
			break;
			case SYS_HW_VERSION:
				memcpy(Read_Buf,&Buf[SYS_HW_VERSION_L],Length);
			break;
			case SYS_SW_VERSION:
				memcpy(Read_Buf,&Buf[SYS_SW_VERSION_L],Length);
			break;
			case SYS_CAN_VERSION:
				memcpy(Read_Buf,&Buf[SYS_CAN_VERSION_L],Length);
			break;
			default:
			break;
		}
	}
}

void Diag_Read_Address(U8 *Read_Buf, U32 DataAddr, U16 DataLen) //Version code read from access
{
	SEG_XDATA volatile U8 DataBuf[16] = {0,};
	SEG_XDATA volatile U8 CodeBank1 = 0x00U;
	SEG_XDATA volatile U8 CodeBank2 = 0x00U;
	SEG_XDATA volatile FLADDR Read_Address1 = 0x0000U;
	SEG_XDATA volatile FLADDR Read_Address2 = 0x0000U;
	SEG_XDATA volatile U16 Length1 = 0x0000U;
	SEG_XDATA volatile U16 Length2 = 0x0000U;

	//memset(CodeBuf, (char)0xFFU ,sizeof(CodeBuf));
	CodeBank1 = DataAddr/0x8000;
	CodeBank2 = (DataAddr + DataLen -1)/0x8000;
	
	if (CodeBank1 == 0x00U)
	{
		CodeBank1 = 0x01U;
		CodeBank2 = 0x01U;
	}

	if (CodeBank1 == CodeBank2)
	{
		Read_Address1 = DataAddr - ((CodeBank1 - 1) * 0x8000);
		Length1 = DataLen;
		CodeBank1 *= BANK1;
		FLASH_Read_Buf(DataBuf, (FLADDR)Read_Address1, Length1, CodeBank1);
	}
	else
	{
		Read_Address1 = DataAddr - ((CodeBank1 - 1) * 0x8000);
		Length1 = (CodeBank2 * 0x8000) - DataAddr;
		CodeBank1 *= BANK1;

		Read_Address2 = 0x8000U;
		Length2 = DataLen - Length1;
		CodeBank2 *= BANK1;

		FLASH_Read_Buf(DataBuf, (FLADDR)Read_Address1, Length1, CodeBank1);
		FLASH_Read_Buf((DataBuf + Length1), (FLADDR)Read_Address2, Length2, CodeBank2);
	}

	memcpy(Read_Buf, DataBuf, DataLen);
}

U16   Diag_Get_Time(void)
{
	U16 Cur_Time = 0x0000U;
	Cur_Time = Get_Time();
	return Cur_Time;
}

