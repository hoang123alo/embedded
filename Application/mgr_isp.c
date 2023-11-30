/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_mcu.h"
#include "drv_timer.h"
#include "drv_i2c.h"
#include "drv_mem.h"
#include "drv_pca.h"
#include "drv_adc.h"
#include "mgr_isp.h"
#include "mgr_comm.h"
#include "mgr_diag.h"

/*--------------------------------------------------------------*/

static SEG_XDATA U8 	g_Overlay_Stauts_Loop_Check_Time;
static SEG_XDATA U8 	g_Guideline_OnOff = TRUE;
static SEG_XDATA U8 	g_Pre_Calibration_Mode = FALSE;
static SEG_XDATA U8 	Overlay_Guideline_flag = 0x00U;
static SEG_XDATA U8 	warning_msg_flag = 0x00U;
static SEG_XDATA U8 	Asix[3U] = {0x00U,0x00U,0x00U};
SEG_XDATA volatile tMsg_ISP_Function tISP_Func_Msg;
SEG_XDATA volatile U8	Optical_Axis_Data[9U] = {0x00U,0x00U,0x00U,0x00U,0x00U,0x00U,0x00U,0x00U,0x00U};

static void Overlay_Guideline(S16 steering_cur, S8 sign);
static U8 Sensor_ID_Check(U8 State);
void DoorbellCheck( void );
static void warning_msg(U8 hutype, U8 Lang);
void Comm_Error_Check(void);
static void Check_Isp_InitState(void);

void Init_IspTask(void)
{
	FLASH_Read_Buf(Optical_Axis_Data,(FLADDR)OPTIC_ADDRESS,9U,BANK2);
	if ( (Optical_Axis_Data[0U] == 0xFFU) && (Optical_Axis_Data[1U] == 0xFFU) )
	{
		if ( Optical_Axis_Data[2U] == 0xFFU )
		{
			//Default
			Optical_Axis_Data[0U] = 0x00U;
			Optical_Axis_Data[1U] = 0x00U;
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
	Flash_OneSpec_Read();

	g_Pre_Calibration_Mode = FALSE;
	DIAG_CALIBRATION_MODE_ACTIVE(FALSE);
	DIAG_OPTIC_OFFSET_SET_ACTIVE(FALSE);

	tISP_Func_Msg.IGNSw = IGN_SW_KEY_OFF;			// start value : 0x00
	tISP_Func_Msg.Pre_Gear = GEAR_INVALID;			// start value : 0x09
	tISP_Func_Msg.Cur_Gear = GEAR_INVALID;
	tISP_Func_Msg.Pre_RVM_CamOff = RVM_CAMERA_INVALID;	// start value : 0x03
	tISP_Func_Msg.Cur_RVM_CamOff = RVM_CAMERA_INVALID;	// start value : 0x03
	tISP_Func_Msg.Pre_RVM_SW = FCZC_RVM_SW_OFF;			// start value : 0x00
	tISP_Func_Msg.Cur_RVM_SW = FCZC_RVM_SW_OFF;			// start value : 0x00
	tISP_Func_Msg.Pre_SW_IND = RVM_SW_IND_INVALID;	// start value : 0x03
	tISP_Func_Msg.Cur_SW_IND = RVM_SW_IND_INVALID;
	tISP_Func_Msg.Pre_ViewMode = RVM_VIEW_INVALID;	// start value : 0x0F
	tISP_Func_Msg.Cur_ViewMode = RVM_VIEW_INVALID;
	tISP_Func_Msg.RVM_SW_Status = FCZC_RVM_SW_OFF;

	g_Overlay_Stauts_Loop_Check_Time = FALSE;

	Isp_InterInit();
}
/*
static U8 Sensor_ID_Check(U8 State)
{
	SEG_XDATA U16 sensor_Id = 0x00U;
	SEG_XDATA U8 ret = 0x00U;
	sensor_Id = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x00U,(U16)0x00,(U16)0x00);
	if(1)//sensor_Id == 0x0242U
	{
		ret = TRUE;
		if ( State == SENSOR_RUNNING )
		{
			DTC_ISP_CM_CLEAR;
		}
	}
	else
	{
		ret = FALSE;
		if ( State == SENSOR_RUNNING )
		{
			DTC_ISP_CM_ERROR;
		}

	}
	
	if ( g_I2C_Err_Cnt > 0x00U )
	{
		g_I2C_Err_Cnt = 0x00U ;
		ret = FALSE;
	}
	ret = TRUE;
	return ret;
}*/

void Isp_InterInit(void)
{	
	SEG_XDATA U16 count = 0x00U;
	SEG_XDATA U8 ret = 0x00U;

	Device_Init(APTINA_AP0100);

	Comm_Tx_RVM_View(RVM_VIEW_INVALID);
	Comm_Tx_RVM_SW_IND(RVM_SW_IND_INVALID);
	tISP_Func_Msg.Pre_ViewMode = RVM_VIEW_INVALID;
	tISP_Func_Msg.Pre_SW_IND = RVM_SW_IND_INVALID;

	Check_Isp_InitState();

	Asix[0] = Optical_Axis_Data[0] + Optical_Axis_Data[6];
	Asix[1] = Optical_Axis_Data[1] + Optical_Axis_Data[7];

	Sensor_XY_Offset_set(Asix[0],Asix[1]);
	//Wait_ms(10U);
	Fixed_Guidline_On(RVM_PARKING_ASSIST_VIEW);
	Wait_ms(10U);

	if ( tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data != 0xFFU )
	{
		warning_msg(tCAN_Rx_Msg[USE_HUTYPE].Pre_Data,tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data);
		Wait_ms(10U);
	}

	Overlay_Guideline((S16)0x00,(S8)0x00);
	//Comm_Tuning_Value_Type(tCAN_Rx_Msg[USE_4WDERR].Pre_Data, tCAN_Rx_Msg[USE_ENGVOL].Pre_Data); //ENG_ONESPEC
	Wait_ms(10U);
	Frame_Sync_Status();
	Delay_Time_Set(TID_OVERLAY_GUIDELINE,DT_OVERLAY_GUIDELINE);
	Delay_Time_Set(TID_FRAME_VALIDE,DT_FRAME_VALIDE);
	Delay_Time_Set(TID_FRAME_COUNT,DT_FRAME_COUNT);
	Delay_Time_Set(TID_I2C_COMM_CHECK,DT_I2C_COMM_CHECK);
	//DTC_ISP_IT_CLEAR;
}

void Operate_IspTask(void)
{
	SEG_XDATA U8 On_Time;
	SEG_XDATA U8 ret;

	if (/*(DIAG_CALIBRATION_MODE_STATE == TRUE) && */(DIAG_OPTIC_OFFSET_SET_STATE == TRUE))
	{
		DIAG_OPTIC_OFFSET_SET_ACTIVE(FALSE);

		Asix[0] = Optical_Axis_Data[0] + Optical_Axis_Data[6];
		Asix[1] = Optical_Axis_Data[1] + Optical_Axis_Data[7];

		Sensor_XY_Offset_set(Asix[0],Asix[1]);
	}

	if ((g_Pre_Calibration_Mode == FALSE) && (DIAG_CALIBRATION_MODE_STATE == TRUE))
	{
		g_Pre_Calibration_Mode = TRUE;
		warning_msg((U8)0x00,(U8)0x00);
		Wait_ms(10U);
	}
//uint5--------------------------------

	Comm_RearView_Data();
	if ((tISP_Func_Msg.Pre_ViewMode != tISP_Func_Msg.Cur_ViewMode) || (tISP_Func_Msg.Pre_SW_IND != tISP_Func_Msg.Cur_SW_IND))
	{
		Comm_Tx_RVM_View(tISP_Func_Msg.Cur_ViewMode);
		Comm_Tx_RVM_SW_IND(tISP_Func_Msg.Cur_SW_IND);
		if ((tISP_Func_Msg.Pre_ViewMode == RVM_DRIVING_ASSIST_VIEW) || (tISP_Func_Msg.Cur_ViewMode == RVM_DRIVING_ASSIST_VIEW))
		{
			Isp_View_Mode_Change(tISP_Func_Msg.Cur_ViewMode);
		}
		tISP_Func_Msg.Pre_ViewMode = tISP_Func_Msg.Cur_ViewMode;
		tISP_Func_Msg.Pre_SW_IND = tISP_Func_Msg.Cur_SW_IND;
	}

	if ( ga_Ext_tDTC_Type_Msg.tDTC_Type[DTC_SENSOR_COMM_ERR].Status != DTC_CURRENT_ERROR )
	{		
		if (tISP_Func_Msg.Pre_ViewMode == RVM_DRIVING_ASSIST_VIEW)
		{
			Isp_Driving_Assist_View();
		}
		else
		{
			Isp_Parking_Assist_View();
		}
	}
//-----------------------------------------------------------------------//
	//STATUS LOOP CHECK 
	On_Time = Delay_Time_Get(TID_CHECKSTATUS_LOOP);
	if ( On_Time == TRUE)
	{
		ret = CheckCommandStatusLoop();
		if ( ret == FALSE )
		{
			g_Overlay_Stauts_Loop_Check_Time = TRUE;
			DTC_ISP_IS_ERROR;
		}
		else if ( ret == TRUE)
		{
			g_Overlay_Stauts_Loop_Check_Time = TRUE;
			DTC_ISP_IS_CLEAR;
		}
		else
		{
			g_Overlay_Stauts_Loop_Check_Time = FALSE;
		}
	}
}
//uint5--------------------------------
void Isp_Parking_Assist_View(void)
{
	SEG_XDATA U8 Sas_Data[2] = {0x00U, };
	SEG_XDATA U8 HU_Type_Data = 0x00U;
	SEG_XDATA U8 Language_Data = 0x00U;
	SEG_XDATA U8 On_Time;
	SEG_XDATA U8 ret;
	static SEG_XDATA volatile U8 overlay_flag = 0x00U;


	if ( g_Guideline_OnOff == TRUE )
	{
		On_Time = Delay_Time_Get(TID_OVERLAY_GUIDELINE);
		if( On_Time == TRUE)
		{
			if (DIAG_CALIBRATION_MODE_STATE == FALSE)
			{
				ret = Comm_Sas_Data(Sas_Data);
				HU_Type_Data = tCAN_Rx_Msg[USE_HUTYPE].Pre_Data;
				Language_Data = tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data;
			}
			else
			{
				Sas_Data[0] = 0x00U;
				HU_Type_Data = 0x00U;
				Language_Data = 0x00U;
			}
			if (overlay_flag == 100U)
			{
				Fixed_Guidline_On(RVM_PARKING_ASSIST_VIEW);
				//Wait_ms(40U);
				overlay_flag++;
			}			
			else if (overlay_flag == 200U)
			{
				if ( tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data != 0xFFU )
				{
					warning_msg((U8)HU_Type_Data, (U8)Language_Data);
					//Wait_ms(40U);
				}
				overlay_flag = 0x00;
			}
			else
			{
				Overlay_Guideline((S16)Sas_Data[0],(S8)Sas_Data[1]);
				overlay_flag++;
			}
		}
	}
	else
	{
		Overlay_Guideline((S16)0x00,(S8)0x00);
	}

	if (DIAG_CALIBRATION_MODE_STATE == FALSE)
	{
		if ((tCAN_Rx_Msg[USE_LANGUAGEINFO].Is_Change == TRUE ) ||  (tCAN_Rx_Msg[USE_HUTYPE].Is_Change == TRUE ))
		{
			tCAN_Rx_Msg[USE_LANGUAGEINFO].Is_Change = FALSE;
			tCAN_Rx_Msg[USE_HUTYPE].Is_Change = FALSE;
			if( tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data != CAN_LAN_INVALID_ADDR )
			{
				Flash_OneSpec_Write();
			}
			warning_msg(tCAN_Rx_Msg[USE_HUTYPE].Pre_Data,tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data);
			Wait_ms(10U); // Need fixed delay
		}
		
		if (g_Pre_Calibration_Mode == TRUE)
		{
			g_Pre_Calibration_Mode = FALSE;
		}
	}
}
//uint5--------------------------------
void Isp_Driving_Assist_View(void)
{
	static SEG_XDATA volatile U8 overlay_flag = 0x00U;
	SEG_XDATA U8 On_Time;

	On_Time = Delay_Time_Get(TID_OVERLAY_GUIDELINE);
	if( On_Time == TRUE)
	{
		if (overlay_flag >= 100U)
		{
			Fixed_Guidline_On(RVM_DRIVING_ASSIST_VIEW);
			overlay_flag = 0U;
		}
		overlay_flag++;
	}
}
//uint5--------------------------------
void Isp_View_Mode_Change(volatile U8 ViewMode)
{
	static SEG_XDATA U16 read_data = 0U, chk_Buffer= 0U;
	U8 i = 0U;
	DoorbellCheck();
	//Wait_ms(100U);
	//Draw black Image in Layer0
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)EXTRA_BUFFER0_DATA,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)MEM_BLACK_IMAGE_ADDR,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_ENABLE_DATA,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)CMD_OVRL_LOAD_BUFFER,(U16)0x00);
	DoorbellCheck();

	// Disable OSD (warning msg, guideline) //On-Screen Display
	if (ViewMode == RVM_DRIVING_ASSIST_VIEW)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)OVR_LAYER1_DATA,(U16)0x00);
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)0x0000,(U16)0x00);
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)0x820AU,(U16)0x00);
		DoorbellCheck();
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)OVR_LAYER2_DATA,(U16)0x00);
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)0x0000,(U16)0x00);
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)0x820AU,(U16)0x00);
		DoorbellCheck();
		//read Layer status and retry
		
		chk_Buffer = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CHECKING_LAYER0_DATA,(U16)0x00,(U16)0x00);
		if (chk_Buffer != 0)
		{
			Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)OVR_LAYER1_DATA,(U16)0x00);
			Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)0x0000,(U16)0x00);
			Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)0x820AU,(U16)0x00);
			DoorbellCheck();
		}
		chk_Buffer = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CHECKING_LAYER1_DATA,(U16)0x00,(U16)0x00);
		if (chk_Buffer != 0)
		{
			Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)OVR_LAYER2_DATA,(U16)0x00);
			Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)0x0000,(U16)0x00);
			Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)0x820AU,(U16)0x00);
			DoorbellCheck();
		}
	}	

	// Draw fixed guideline
	if (ViewMode == RVM_DRIVING_ASSIST_VIEW)
	{
		Fixed_Guidline_On(RVM_DRIVING_ASSIST_VIEW);
	}
	else
	{
		Fixed_Guidline_On(RVM_PARKING_ASSIST_VIEW);
		if ( tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data != 0xFFU )
		{
			warning_msg(tCAN_Rx_Msg[USE_HUTYPE].Pre_Data,tCAN_Rx_Msg[USE_LANGUAGEINFO].Pre_Data);
		}
	}

	// STE load
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x098EU,(U16)0x7C00U,(U16)0x00);
	if (ViewMode == RVM_DRIVING_ASSIST_VIEW)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0xfc00U,(U16)0x0001U,(U16)0x00);
	}
	else
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0xfc00U,(U16)0x0000U,(U16)0x00);
	}
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8311U,(U16)0x00);

	DoorbellCheck();
	while(1)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8101U,(U16)0x00);
		Wait_ms(10U);
		read_data = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x00,(U16)0x00);
		if ( read_data != (U16)0x0009 )
		{
			break;
		}
		DoorbellCheck();
	}

	// Change config
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0xfc00U,(U16)0x2800U,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8100U,(U16)0x00);
	DoorbellCheck();
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0xfc00U,(U16)0x2800U,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8100U,(U16)0x00);
	DoorbellCheck();
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8101U,(U16)0x00);

	// Disable Layer0 (black image)
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)OVR_LAYER0_DATA,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)0x0000,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)0x820AU,(U16)0x00);
	DoorbellCheck();

	chk_Buffer = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CHECKING_LAYER0_DATA,(U16)0x00,(U16)0x00);
	if (chk_Buffer != 0)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)OVR_LAYER0_DATA,(U16)0x00);
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)0x0000,(U16)0x00);
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)0x820AU,(U16)0x00);
		DoorbellCheck();
	}

	//Wait_ms(100U);

	for (i = DTC_SENSOR_FV_ERR; i < DTC_CODE_COUNT; i++)
	{
		ga_Ext_tDTC_Type_Msg.tDTC_Type[i].Count = 0x00U;
	}
}

void V_STB_Check(void)
{
	SEG_XDATA U8 ret = FALSE;
	static SEG_XDATA U8 v_stb_error_count = 0x00U;
	static SEG_XDATA U8 v_stb_nomal_count = 0x00U;

	ret = (U8)AMP_V_STB;
	if ( ret == TRUE )
	{
		v_stb_nomal_count = 0x00U;
		v_stb_error_count++;
		if ( v_stb_error_count >= 20U)	//2s
		{
			DTC_VID_VO_ERROR;
			v_stb_error_count = 0x00U;
		}
	}
	else
	{
		v_stb_error_count = 0x00U;
		v_stb_nomal_count++;
		if ( v_stb_nomal_count >= 20U)	//2s
		{
			DTC_VID_VO_CLEAR;
			v_stb_nomal_count = 0x00U;
		}
	}
}

void Check_Isp_InitState(void)
{
	static SEG_XDATA U16 chk_data = 0x0000U;
	static SEG_XDATA U8 lpcn = 0x00U;
	//SEG_XDATA U8 rcv_data = FALSE;
	SEG_XDATA volatile U8 Ret_State = FALSE;
	lpcn = 0;
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)CMD_GET_STATE,(U16)0x00);
	while(1)
	{
		chk_data = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)0x00,(U16)0x00);
		if( (chk_data & 0x8000U) == 0x0000U )
		{
			chk_data = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)0x00,(U16)0x00);
			chk_data &= 0xFF00U;
			lpcn++;
			if ( (lpcn >= 15U) || (chk_data == SENSOR_STATE_STREAMING))
			{
				if ( lpcn >= 15U )
				{
					DTC_ISP_IT_ERROR;
				}
				else
				{
					DTC_ISP_IT_CLEAR;
				}		
				break;
			}
			Wait_ms(20U);
		}

	}
	lpcn = 0;
}

void Frame_Count_Check(void)
{
	static SEG_XDATA U16 Cur_FC_Data = (U16)0x00U;
	static SEG_XDATA U16 Pre_FC_Data = (U16)0x00U;

	Cur_FC_Data = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)AP0100_FRAME_COUNT_REGISTER,(U16)0x00,(U16)0x00);

	if( Pre_FC_Data == Cur_FC_Data )
	{
		DTC_ISP_FC_ERROR;
	}
	else
	{
		DTC_ISP_FC_CLEAR;
		Pre_FC_Data = Cur_FC_Data;
	}
}

void Frame_Valide_Check(void)
{
	SEG_XDATA U8 volatile Frame_value = 0U;
	Frame_value = Frame_Sync_Status();

	if ( (Frame_value >= 2U) && (Frame_value < 4U))
	{
		DTC_ISP_FV_CLEAR;
	}
	else 
	{
		DTC_ISP_FV_ERROR;
	}
}

void Video_Signal_Check(void)
{
	SEG_XDATA U8 ret = TRUE;
	static SEG_XDATA U8 video_error_count = 0x00U;
	static SEG_XDATA U8 video_nomal_count = 0x00U;

	// ADC
	ret = Get_Video_Signal();
	if ( ret == FALSE )
	{
		video_nomal_count = 0x00U;
		video_error_count++;
		if ( video_error_count >= 20U)	//2s
		{
			DTC_VID_OC_ERROR;
			video_error_count = 0x00U;
		}
	}
	else
	{
		video_error_count = 0x00U;
		video_nomal_count++;
		if ( video_nomal_count >= 20U)	//2s
		{
			DTC_VID_OC_CLEAR;
			video_nomal_count = 0x00U;
		}
	}
}
// thuc hien kiem tra thong qua giao tiep i2c .
// cho coi mot khoang thoi gian va lap lai qua trinh neu can thiet 
void DoorbellCheck( void )
{
	SEG_XDATA U8 On_Time = FALSE;
	SEG_DATA U16 chk_data = 0x8000U;
	SEG_DATA U8 lpcnt = 0x00U;

	Delay_Time_Set(TID_DOORBELL_CHECK, DT_DOORBELL_CHECK);
	do
	{
		On_Time = Delay_Time_Get(TID_DOORBELL_CHECK);
		if ( On_Time == TRUE)
		{
			lpcnt++;
			 // Wait_ms(10U);
			 // doc gia tri tu I2C_2BYTE_RW cua ISP_SLAVE
			chk_data = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x00,(U16)0x00);
			WDT_Clear();
		}
	}
	while ( ((chk_data & 0x8000U) == 0x8000U) && (lpcnt < (U8)10) );

	Delay_Time_Expire(TID_DOORBELL_CHECK);
}

static void Overlay_Guideline(S16 steering_cur, S8 sign)
{
	SEG_DATA volatile U16 read_data = 0x0000U;
	//SEG_XDATA S8  = 0;

	DoorbellCheck();

	if(sign == 0)
	{
		steering_cur = -steering_cur;
	}

	steering_cur = (S16)NORMAL_3M_GUIDE_LINE_ADDR + (S16)STEERING_ANGLE_LIMIT + steering_cur;

	if(Overlay_Guideline_flag == 0){
		//i2c_master_write( (U16)CMD_PARAM_POOL_0_ADDR,	(U16)GUIDELINE_BUFFER2_DATA );
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)GUIDELINE_BUFFER2_DATA,(U16)0x00);
		Overlay_Guideline_flag = 1;
	}
	else{
		//i2c_master_write( (U16)CMD_PARAM_POOL_0_ADDR,	(U16)GUIDELINE_BUFFER3_DATA );
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)GUIDELINE_BUFFER3_DATA,(U16)0x00);
		Overlay_Guideline_flag = 0;
	}	

	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)steering_cur,(U16)0x00);
	if ( g_Guideline_OnOff == TRUE)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_ENABLE_DATA,(U16)0x00);
	}
	else
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_DISABLE_DATA,(U16)0x00);
	}
	WDT_Clear();
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)CMD_OVRL_LOAD_BUFFER,(U16)0x00);	
	g_Overlay_Stauts_Loop_Check_Time = FALSE;
	//Delay_Time_Set(TID_CHECKSTATUS_LOOP,DT_CHECKSTATUS_LOOP);

	DoorbellCheck();
}

/***********************************
 **  Check for command status
 ***********************************/
U8 CheckCommandStatusLoop( void )
{
	static SEG_XDATA U16 chk_data = 0x0000;
	static SEG_XDATA U8 lpcnt = 0;
	SEG_XDATA U8 ret = TRUE;
	
	lpcnt++;
	//chk_data = i2c_master_read( (U16)CMD_REGISTER_ADDR );
	chk_data = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)0x00,(U16)0x00);
	if( (chk_data & 0x8000U) == 0x0000U )
	{
		ret = TRUE;
		lpcnt = 0x00U;
		Delay_Time_Expire(TID_CHECKSTATUS_LOOP);
		WDT_Clear();
	}	
	else
	{
		if ( lpcnt >= 5U )
		{
#ifndef DIAG_TEST_MCU_WD_ERR
		WDT_Clear();
#endif
			ret = FALSE;
			lpcnt = 0x00U;
			Delay_Time_Expire(TID_CHECKSTATUS_LOOP);
		}
		else
		{
			ret = 0x03U; 		//is confirmimg
		}
	}
	return ret;
}

void Guidline_Off(void)
{
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_DISABLE_DATA,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)CMD_OVRL_LOAD_BUFFER,(U16)0x00);	
	g_Guideline_OnOff = FALSE;
}

void Guidline_On(void)
{
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_ENABLE_DATA,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)CMD_OVRL_LOAD_BUFFER,(U16)0x00);	
	g_Guideline_OnOff = TRUE;
}

void Isp_Off(void)
{
	MCU_SENSOR_RESET = 0x00;
}

void Isp_On(void)
{
	Init_IspTask();
}

void Fixed_Guidline_On(const U8 ViewMode)
{
	static SEG_XDATA S8 overlay_buffer_flag = 0;
	DoorbellCheck();

	if ( overlay_buffer_flag == 0x00U)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)FIXED_GUIDELINE_BUFFER4_DATA,(U16)0x00);
		overlay_buffer_flag = 0x01U;
	}
	else
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)FIXED_GUIDELINE_BUFFER5_DATA,(U16)0x00);
		overlay_buffer_flag = 0x00U;
	}
	if (ViewMode == RVM_DRIVING_ASSIST_VIEW)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)MEM_FIXED_GUIDE_ADDR_D,(U16)0x00);
	}
	else
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)MEM_FIXED_GUIDE_ADDR,(U16)0x00);
	}
	if (g_Guideline_OnOff == TRUE)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_ENABLE_DATA,(U16)0x00);
	}
	else
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_DISABLE_DATA,(U16)0x00);
	}
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)CMD_OVRL_LOAD_BUFFER,(U16)0x00);
	
	DoorbellCheck();	
}

void Fixed_Guidline_Off(void)
{
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_DISABLE_DATA,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)CMD_OVRL_LOAD_BUFFER,(U16)0x00);
}

static void warning_msg(U8 hutype, U8 Lang)
{
	static SEG_XDATA U16 lan_start_addr = MEM_LAN_START_ADDR;
	static SEG_XDATA U16 Pre_Lang = 0xFFU ;
	SEG_DATA volatile U16 read_data = 0x0000U;
	SEG_XDATA U8 ret = FALSE;

	DoorbellCheck();

	if ( ((hutype >= 0x10U) && (hutype <= 0x21U)) || ((hutype >= 0x60U) && (hutype <= 0x68U)) || 
	      (hutype == 0x41U) || (hutype == 0x70U) || (hutype == 0xFAU) || (Lang > 0x1FU) )
	{	
		ret = FALSE;
	}
	else
	{
		ret = TRUE;
	}
	
	if ( Lang == 0xFFU )
	{
		ret = FALSE;
	}

	if(ret == TRUE)
	{
		if(Lang == CAN_LAN_INVALID_ADDR) 
		{
			Lang = Pre_Lang;		// Invalid address. previous value
		}
		else 
		{
			Lang += (U8)lan_start_addr;	// language id and flash address matching
		}
	}
	else 
	{
		Lang = MEM_LAN_INVALID_ADDR;		// Invalid address. blank value
	}

	Pre_Lang = Lang;

	if (warning_msg_flag == 0x00U)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)WARNING_BUFFER6_DATA,(U16)0x00);
		warning_msg_flag = 0x01U;
	}
	else
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_0_ADDR,(U16)WARNING_BUFFER1_DATA,(U16)0x00);
		warning_msg_flag = 0x00U;
	}
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_2_ADDR,(U16)Lang,(U16)0x00);
	if(ret == TRUE)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_PARAM_POOL_4_ADDR,(U16)OVRL_ENABLE_DATA,(U16)0x00);
	}
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)CMD_REGISTER_ADDR,(U16)CMD_OVRL_LOAD_BUFFER,(U16)0x00);

	DoorbellCheck();
}

static void Sensor_XY_Offset_set(U8 x, U8 y )
{
	SEG_DATA volatile U16 read_data = 0x0000U;
	U16 write_data = 0x0000U;

	write_data = (U16)(((U16)x << (U16)8) | (U16)y);
	
	DoorbellCheck();

	while(1)
	{
		Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8101U,(U16)0x00);
		Wait_ms(10U);
		read_data = Comm_I2C_Rx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x00,(U16)0x00);
		if ( read_data != (U16)0x0009 )
		{
			break;
		}
		DoorbellCheck();
	}

	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x098EU,(U16)AP0100_SENSOR_X_OFFSET,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)AP0100_SENSOR_X_OFFSET,(U16)write_data,(U16)0x00);

	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0xfc00U,(U16)0x2800U,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8100U,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0xfc00U,(U16)0x2800U,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8100U,(U16)0x00);
	Comm_I2C_Tx(ISP_SLAVE_ADDRESS,I2C_2BYTE_RW,(U16)0x0040U,(U16)0x8101U,(U16)0x00);

	DoorbellCheck();
}

void isp_guide_on_off(U8 Guide_On_Off)
{
	g_Guideline_OnOff = Guide_On_Off;
}