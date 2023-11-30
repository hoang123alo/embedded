/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_can.h"
#include "drv_timer.h"
#include "drv_mcu.h"
#include "drv_pca.h"        /* MISRA-C Rule 160504 */
#include "mgr_comm.h"
#include "mgr_isp.h"
#include "mgr_diag.h"
#include "desc.h"
#include "nm_basic.h"
#include "il_inc.h"
#include "math.h"
#include "v_def.h"
#include "can_def.h"
/*--------------------------------------------------------------*/
static SEG_XDATA U8 g_HuType_Interrupt			= FALSE;
static SEG_XDATA U8 g_LanguageInfo_Interrupt 	= FALSE;
static SEG_XDATA U8 g_SAS_Interrupt				= FALSE;
static SEG_XDATA U8 g_SAS_Timeout				= FALSE;
static SEG_XDATA U8 g_Sel_Disp_Interrupt		= FALSE;
static SEG_XDATA U8 g_Sel_Disp_Timeout			= FALSE;
static SEG_XDATA U8 g_IGN_SW_Interrupt		   	= FALSE;
static SEG_XDATA U8 g_IGN_SW_Timeout			= FALSE;
static SEG_XDATA U8 g_RVM_CameraOff_Interrupt	= FALSE;
static SEG_XDATA U8 g_FCZC_RVM_SW_Interrupt		= FALSE;
static SEG_XDATA U8 g_FCZC_RVM_SW_Timeout		= FALSE;
static SEG_XDATA U8 g_Eng_Vol_Interrupt			= FALSE;
static SEG_XDATA U8 g_Eng_Vol_Timeout			= FALSE;
static SEG_XDATA U8 g_4WD_Err_Interrupt			= FALSE;
static SEG_XDATA U8 g_4WD_Err_Timeout			= FALSE;
extern SEG_XDATA tMsg_Ext_DTC_Type ga_Ext_tDTC_Type_Msg;

static void Is_Change(tMsg_CAN_Rx_Data_s *Rx_Datan);
static void Is_Chattering(tMsg_CAN_Rx_Data_s *Rx_Datan);

U8 gTuning_Value[4][6] =
{
	37, 19, 25,  37	, 19, 25, // ENG 2.2, 2WD
	37, 25, 80,  36	, 25, 80, // ENG 3.3, 2WD
	37, 19, 25,  37	, 19, 25, // ENG 2.2, 4WD(AWD)
	35, 25, 77,  35	, 25, 77  // ENG 3.3, 4WD(AWD)
};

void VCan_Init(void)
{
	EIE2 |= (U8)0x02;
	EA = 0U;
	EA = 0U;

	CanInitPowerOn();
	WDT_Clear();
	IlInitPowerOn();
	WDT_Clear();
	TpInitPowerOn();
	WDT_Clear();
	NmBasicInitPowerOn();
	WDT_Clear();
	DescInitPowerOn(0);

	EA = 1U;
}

void VCan_Start(void)
{
	NmBasicStart();
	WDT_Clear();
	IlTxStart();
	WDT_Clear();
	IlRxStart();
	WDT_Clear();
}

void VCan_Task(void)
{
	IlRxTask();
	WDT_Clear();
	IlTxTask();
	WDT_Clear();
	TpRxTask();
	WDT_Clear();
	DescTask();
	WDT_Clear();
	TpTxTask();
	WDT_Clear();
	NmBasicTask();
	WDT_Clear();
}

static void Is_Chattering(tMsg_CAN_Rx_Data_s *Rx_Datan)
{
	SEG_XDATA U8 On_Time;
	if ( Rx_Datan->Cur_Data != Rx_Datan->Pre_Data )
	{
		if ( Rx_Datan->Chatter_Time == (U16)0 )
		{
			Delay_Time_Set(TID_GEAR_CHATTERING,DT_GEAR_CHATTERING);
			Rx_Datan->Chatter_Time = 0x01U;
			Rx_Datan->Mid_Data = Rx_Datan->Cur_Data;
		}
		else
		{
			On_Time = Delay_Time_Get(TID_GEAR_CHATTERING);
			if ( On_Time == TRUE)
			{
				Rx_Datan->Chatter_Time++;
				if ( Rx_Datan->Chatter_Time >= (U16)8)
				{
					Rx_Datan->Chatter_Time = (U16)0;
					Rx_Datan->Mid_Data     = (U16)0;					
					Rx_Datan->Pre_Data     = Rx_Datan->Cur_Data;
					Rx_Datan->Is_Change	   = TRUE;
					Rx_Datan->Cur_Data	   = (U16)0;
					Delay_Time_Expire(TID_GEAR_CHATTERING);	
				}
			}
		}
	}
	else
	{
		Rx_Datan->Chatter_Time = (U16)0;
		Rx_Datan->Mid_Data     = (U16)0;					
		Rx_Datan->Cur_Data	   = (U16)0;
		Delay_Time_Expire(TID_GEAR_CHATTERING);					
	}
}

static void Is_Change(tMsg_CAN_Rx_Data_s *Rx_Datan)
{
	if ( Rx_Datan->Cur_Data != Rx_Datan->Pre_Data )
	{
		if ( Rx_Datan->Change_Count == 0x00U )
		{
			Rx_Datan->Mid_Data = Rx_Datan->Cur_Data;					
			Rx_Datan->Change_Count++;
		}
		else
		{
			if ( Rx_Datan->Cur_Data == Rx_Datan->Mid_Data )					
			{
				Rx_Datan->Change_Count++;
			}
			else
			{
				Rx_Datan->Change_Count = 0x00U;
			}
		}
		
		if ( Rx_Datan->Change_Count >= 3U )
		{
			Rx_Datan->Pre_Data 		= Rx_Datan->Cur_Data;
			Rx_Datan->Is_Change 	= TRUE;
			Rx_Datan->Change_Count 	= 0x00;
			Rx_Datan->Mid_Data 		= 0xFFU;
		}
	}
}

void CAN_Rx_Data_G_SEL_DISP_Process(tMsg_CAN_Rx_Data_s *Rx_Data)
{
	if ( g_Sel_Disp_Timeout == TRUE)
	{
		Rx_Data->Chatter_Time = (U16)0;
		Rx_Data->Mid_Data     = (U16)0;					
		Rx_Data->Pre_Data     = GEAR_P;
		Rx_Data->Is_Change	  = TRUE;
		Rx_Data->Cur_Data	  = (U16)0;
	}
	else
	{
		Rx_Data->Time_Out = FALSE;
		if ( g_Sel_Disp_Interrupt == TRUE)
		{
			g_Sel_Disp_Interrupt = FALSE;
			Rx_Data->Cur_Data = IlGetRxG_SEL_DISP();
			Is_Chattering(Rx_Data);
		}
	}
}

void CAN_Rx_Data_IGN_SW_Process(tMsg_CAN_Rx_Data_s *Rx_Data)
{
	if ( g_IGN_SW_Timeout == TRUE)
	{
		Rx_Data->Chatter_Time = (U16)0;
		Rx_Data->Mid_Data     = (U16)0;					
		Rx_Data->Pre_Data     = IGN_SW_KEY_OFF;
		Rx_Data->Is_Change	  = TRUE;
		Rx_Data->Cur_Data	  = (U16)0;
	}
	else
	{
		Rx_Data->Time_Out = FALSE;
		if ( g_IGN_SW_Interrupt == TRUE)
		{
			g_IGN_SW_Interrupt = FALSE;
			Rx_Data->Cur_Data = IlGetRxCF_Gway_IGNSw();
			if ( Rx_Data->Cur_Data != Rx_Data->Pre_Data )
			{
				Rx_Data->Pre_Data =  Rx_Data->Cur_Data;
				Rx_Data->Is_Change 	= TRUE;
			}
		}
	}
}

void CAN_Rx_Data_RVM_CameraOff_Process(tMsg_CAN_Rx_Data_s *Rx_Data)
{
	if ( g_RVM_CameraOff_Interrupt == TRUE)
	{
		g_RVM_CameraOff_Interrupt = FALSE;
		Rx_Data->Cur_Data = IlGetRxHU_RVM_CameraOff();
		if ( Rx_Data->Cur_Data != Rx_Data->Pre_Data )
		{
			Rx_Data->Pre_Data =  Rx_Data->Cur_Data;
			Rx_Data->Is_Change 	= TRUE;
		}
	}
}

void CAN_Rx_Data_FCZC_RVM_SW_Process(tMsg_CAN_Rx_Data_s *Rx_Data)
{
	if ( g_FCZC_RVM_SW_Timeout == TRUE)
	{
		Rx_Data->Chatter_Time = (U16)0;
		Rx_Data->Mid_Data     = (U16)0;					
		Rx_Data->Pre_Data     = FCZC_RVM_SW_OFF;
		Rx_Data->Is_Change	  = TRUE;
		Rx_Data->Cur_Data	  = (U16)0;
	}
	else
	{
		Rx_Data->Time_Out = FALSE;
		if ( g_FCZC_RVM_SW_Interrupt == TRUE)
		{
			g_FCZC_RVM_SW_Interrupt = FALSE;
			Rx_Data->Cur_Data = IlGetRxCF_CK_FCZC_RVM_SW();
			if ( Rx_Data->Cur_Data != Rx_Data->Pre_Data )
			{
				Rx_Data->Pre_Data =  Rx_Data->Cur_Data;
				Rx_Data->Is_Change 	= TRUE;
			}
		}
	}
}

void CAN_Rx_Data_4WD_ERR_Process(tMsg_CAN_Rx_Data_s *Rx_Data)
{
	if ( g_4WD_Err_Timeout == TRUE)
	{
		if ( Rx_Data->Pre_Data == TYPE_4WD )
		{
			Rx_Data->Is_Change	  = TRUE;
		}
		Rx_Data->Chatter_Time = (U16)0;
		Rx_Data->Mid_Data     = (U16)0;					
		Rx_Data->Pre_Data     = (U16)0; // TYPE_2WD
		Rx_Data->Cur_Data	  = (U16)0;
	}
	else
	{
		if ( g_4WD_Err_Interrupt == TRUE)
		{
			g_4WD_Err_Interrupt = FALSE;
			Rx_Data->Cur_Data = (U8)IlGetRxWD4_ERR();

			if ( Rx_Data->Change_Count >= 10U )
			{
				Rx_Data->Pre_Data 		= TYPE_4WD;
				Rx_Data->Is_Change 		= TRUE;
				Rx_Data->Change_Count 	= 0x00;
				Rx_Data->Mid_Data 		= 0xFFU;
			}
			else
			{
				Rx_Data->Change_Count++;
			}
#if 0	
			if ( Rx_Data->Cur_Data != Rx_Data->Pre_Data )
			{
				if ( Rx_Data->Change_Count == 0x00U )
				{
					Rx_Data->Mid_Data = Rx_Data->Cur_Data;					
					Rx_Data->Change_Count++;
				}
				else
				{
					if ( Rx_Data->Cur_Data == Rx_Data->Mid_Data )					
					{
						Rx_Data->Change_Count++;
					}
					else
					{
						Rx_Data->Change_Count = 0x00U;
					}
				}
	
				if ( Rx_Data->Change_Count >= 10U )
				{
				//	Rx_Data->Pre_Data 		= Rx_Data->Cur_Data;
					Rx_Data->Pre_Data 		= TYPE_4WD;
					Rx_Data->Is_Change 	= TRUE;
					Rx_Data->Change_Count 	= 0x00;
					Rx_Data->Mid_Data 		= 0xFFU;
				}
			}
#endif
		}
	}
}

void CAN_Rx_Data_Eng_Vol_Process(tMsg_CAN_Rx_Data_s *Rx_Data)
{
	if ( g_Eng_Vol_Timeout == TRUE)
	{
		if ( Rx_Data->Pre_Data == TYPE_ENGVOL33 )
		{
			Rx_Data->Is_Change	  = TRUE;
		}
		Rx_Data->Chatter_Time = (U16)0;
		Rx_Data->Mid_Data     = (U16)0;					
		Rx_Data->Pre_Data     = (U16)0;
		Rx_Data->Cur_Data	  = (U16)0;
	}
	else
	{
		if ( g_Eng_Vol_Interrupt == TRUE)
		{
			g_Eng_Vol_Interrupt = FALSE;
			Rx_Data->Cur_Data = (U8)IlGetRxENG_VOL();
	
			if ( Rx_Data->Cur_Data != Rx_Data->Pre_Data )
			{
				if ( Rx_Data->Change_Count == 0x00U )
				{
					Rx_Data->Mid_Data = Rx_Data->Cur_Data;					
					Rx_Data->Change_Count++;
				}
				else
				{
					if ( Rx_Data->Cur_Data == Rx_Data->Mid_Data )					
					{
						Rx_Data->Change_Count++;
					}
					else
					{
						Rx_Data->Change_Count = 0x00U;
					}
				}
	
				if ( Rx_Data->Change_Count >= 10U )
				{
					Rx_Data->Pre_Data 		= Rx_Data->Cur_Data;
					Rx_Data->Is_Change 	= TRUE;
					Rx_Data->Change_Count 	= 0x00;
					Rx_Data->Mid_Data 		= 0xFFU;
				}
			}
		}
	}
}

void CAN_Rx_Data_Process(U8 ID,tMsg_CAN_Rx_Data_s *Rx_Data)
{
	switch(ID)
	{
		case USE_MDPS11:
		break;
		case USE_LANGUAGEINFO:
			if ( g_LanguageInfo_Interrupt == TRUE)
			{
				g_LanguageInfo_Interrupt = FALSE;
				Rx_Data->Cur_Data = (U8)IlGetRxCF_Clu_LanguageInfo();
				if ( Rx_Data->Cur_Data != 0xFFU)
				{
					Is_Change(Rx_Data);
				}		
			}
		break;
		case USE_HUTYPE:
			if ( g_HuType_Interrupt == TRUE)
			{
				g_HuType_Interrupt = FALSE;
				Rx_Data->Cur_Data = (U8)IlGetRxHU_TYPE();
				Is_Change(Rx_Data);
			}
		break;
		case USE_GATEWAY:
		break;
		case USE_NAVIONOFF:
		break;

		default:
		break;
	}
}

void CAN_Rx_Sas_Process(tMsg_CAN_SAS_Data_s *Rx_Data)
{
	SEG_XDATA U8	Cur_sign = 0x00U;
	SEG_XDATA U16	iStAng_in = (U16)0x00U;
	SEG_XDATA U16	steering_cur = (U16)0x00U;
	static SEG_XDATA U16	iStAng_CAN = (U16)0x00U;
	
	g_SAS_Timeout = IlGetSAS_AngleRxTimeout();
	if ( g_SAS_Timeout == TRUE )
	{
		if ( Rx_Data->Time_Out == FALSE )
		{
			Rx_Data->Time_Out = TRUE;
			Rx_Data->Is_Change = TRUE;
			Rx_Data->Angle = 0x0000U;//  SAS = 0 
			Rx_Data->Angle_Pre_Sign = 1U;
		}		
	}
	else
	{
		Rx_Data->Time_Out = FALSE;
		// nhan du lieu sas
		if ( g_SAS_Interrupt == TRUE)
		{
			g_SAS_Interrupt = FALSE;
			iStAng_CAN = IlGetRxSAS_Angle(); 
	// tinh huong goc lai 
			if( iStAng_CAN <= 0x7FFFU){
				Cur_sign  = 1;	// +°¢µµ
				iStAng_in = iStAng_CAN;
			}else{
				Cur_sign  = 0;	// -°¢µµ
				iStAng_in = ((0xFFFFU - iStAng_CAN) + 0x0001U);
			}
		
			if ( iStAng_in == 0x7FFFU)
			{
				iStAng_in = 0;
			}	
		// ap dung goc lai hien tai khi SAS thay doi ten 16 
			if ( (U16)abs((S16)iStAng_in - (S16)Rx_Data->Pre_Data) > (U16)16)       //0.5:68 0.4:53 0.3:41
			{
				steering_cur = iStAng_in / (U16)10;
				if (Cur_sign == 1U) 
				{
					steering_cur = (U16)gTuning_Value[Rx_Data->Tuning_Type][1]*steering_cur;
					steering_cur = steering_cur/((U16)gTuning_Value[Rx_Data->Tuning_Type][2]+(U16)200);
				}
				else
				{
					steering_cur = (U16)gTuning_Value[Rx_Data->Tuning_Type][4]*steering_cur;
					steering_cur = steering_cur/((U16)gTuning_Value[Rx_Data->Tuning_Type][5]+(U16)200);
				}

				Rx_Data->Pre_Data = iStAng_in;
				Rx_Data->Pre_Sign = Cur_sign;
			}	 
			else
			{
				steering_cur = Rx_Data->Pre_Data / (U16)10;
				if (Cur_sign == 1U) 
				{
					steering_cur = (U16)gTuning_Value[Rx_Data->Tuning_Type][1]*steering_cur;
					steering_cur = steering_cur/((U16)gTuning_Value[Rx_Data->Tuning_Type][2]+(U16)200);
				}
				else
				{
					steering_cur = (U16)gTuning_Value[Rx_Data->Tuning_Type][4]*steering_cur;
					steering_cur = steering_cur/((U16)gTuning_Value[Rx_Data->Tuning_Type][5]+(U16)200);
				}
			}
			// kiem tra giói han goc lai 
			if ( Cur_sign == 1U)
			{
				if ( steering_cur > gTuning_Value[Rx_Data->Tuning_Type][0])
				{
		    		steering_cur = gTuning_Value[Rx_Data->Tuning_Type][0];
				}
			} 
			else
			{
				if ( steering_cur > gTuning_Value[Rx_Data->Tuning_Type][3])
				{
		    		steering_cur = gTuning_Value[Rx_Data->Tuning_Type][3];
				}

    		} 
			// neu goc lai thay doi , cap nhat gia tri va trang thai
			if ((steering_cur != Rx_Data->Angle) || (Cur_sign != Rx_Data->Angle_Pre_Sign))
			{
				Rx_Data->Angle = (U8)steering_cur;
				Rx_Data->Angle_Pre_Sign = Cur_sign;
				Rx_Data->Is_Change = TRUE;
			}
		}
	}
}

void ApplNmBasicSwitchTransceiverOn(NMBASIC_CHANNEL_APPLTYPE_ONLY)
{
}
void ApplNmBasicEnabledCom(NMBASIC_CHANNEL_APPLTYPE_ONLY)
{
#if defined (VGEN_ENABLE_IL_VECTOR)
	IlRxStart();
	IlTxStart();
#endif

}
void ApplNmBasicBusOffEnd(NMBASIC_CHANNEL_APPLTYPE_ONLY)
{
#if defined (VGEN_ENABLE_IL_VECTOR)
//#if (no timeout detection during BusOff )
	IlRxStart();
	IlTxStart();
//#endif
#endif
}
void ApplNmBasicBusOffRestart(NMBASIC_CHANNEL_APPLTYPE_ONLY)
{

}
void ApplNmBasicBusOffStart(NMBASIC_CHANNEL_APPLTYPE_ONLY)
{
#if defined (VGEN_ENABLE_IL_VECTOR)
//#if (no timeout detection during BusOff )
	CanCancelTransmit(0);
	IlRxStop();
	IlTxStop();
//#endif
#endif
}
void ApplNmBasicDisabledCom(NMBASIC_CHANNEL_APPLTYPE_ONLY)
{
#if defined (VGEN_ENABLE_IL_VECTOR)
	IlRxStop();
	IlTxStop();
#endif	
}
void ApplNmBasicFirstBusOffSlow(NMBASIC_CHANNEL_APPLTYPE_ONLY)
{
	U8 lFlgCanStatus;
	
	lFlgCanStatus = (CanGetStatus() & kCanTxOn);
	CanOffline();
	CanPartOffline(C_SEND_GRP_NONE);
	if ( lFlgCanStatus == kCanTxOn)
	{
		CanOnline();
	}
}

void ApplIlFatalError(vuint8 errorNumber)
{
	errorNumber = 0U;
}

vuint8 ApplNmBasicSwitchTransceiverOff(NMBASIC_CHANNEL_APPLTYPE_ONLY)
{
	vuint8 ret = 0U;
	return ret;
}

void TP_API_CALLBACK_TYPE ApplTpFatalError(canuint8 errorNumber)
{
	errorNumber = 0U;
}

void ISR_SAS(CanReceiveHandle rcvObject)
{
	g_SAS_Interrupt = TRUE;
	g_SAS_Timeout = FALSE;
}

/* CODE CATEGORY 1 START */
void ISR_HuType(CanReceiveHandle rcvObject)
{
	g_HuType_Interrupt = TRUE;
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_NaviOnOff(CanReceiveHandle rcvObject)
{
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_CountryCfg(CanReceiveHandle rcvObject)
{
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_LanguageInfo(CanReceiveHandle rcvObject)
{
	g_LanguageInfo_Interrupt = TRUE;
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_MdpsType(CanReceiveHandle rcvObject)
{
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_RVMCameraOff(CanReceiveHandle rcvObject)
{
	g_RVM_CameraOff_Interrupt = TRUE;
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_IGNSw(CanReceiveHandle rcvObject)
{
	g_IGN_SW_Interrupt = TRUE;
	g_IGN_SW_Timeout = FALSE;
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_FczcRvmSw(CanReceiveHandle rcvObject)
{
	g_FCZC_RVM_SW_Interrupt = TRUE;
	g_FCZC_RVM_SW_Timeout = FALSE;
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_GSelpDisp(CanReceiveHandle rcvObject)
{
	g_Sel_Disp_Interrupt = TRUE;
	g_Sel_Disp_Timeout = FALSE;
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_EngVol(CanReceiveHandle rcvObject)
{
	g_Eng_Vol_Interrupt = TRUE;
	g_Eng_Vol_Timeout = FALSE;
}
/* CODE CATEGORY 1 END */

/* CODE CATEGORY 1 START */
void ISR_WD4Err(CanReceiveHandle rcvObject)
{
	g_4WD_Err_Interrupt = TRUE;
	g_4WD_Err_Timeout = FALSE;
}
/* CODE CATEGORY 1 END */


void ApplSAS1MsgTimeout(void)
{
	g_SAS_Timeout = TRUE;
}

void ApplHU_MON_PE_01MsgTimeout(void){}
void ApplCGW4MsgTimeout(void){}
void ApplCGW2MsgTimeout(void){}
void ApplCLU15MsgTimeout(void){}
void ApplMDPS11MsgTimeout(void){}
void ApplHU_RVM_E_00TMsgTimeout(void){}

void ApplCGW1MsgTimeout(void)
{
	g_IGN_SW_Timeout = TRUE;
}

void ApplCGW6MsgTimeout(void)
{
	g_FCZC_RVM_SW_Timeout = TRUE;
}

void ApplCGW_PC5MsgTimeout(void)
{
	g_Sel_Disp_Timeout = TRUE;
}

void ApplWD411MsgTimeout(void)
{
	g_4WD_Err_Timeout = TRUE;
}

void ApplEMS12MsgTimeout(void)
{
	g_Eng_Vol_Timeout = TRUE;
}

