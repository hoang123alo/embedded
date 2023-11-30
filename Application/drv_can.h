#include "mgr_comm.h"
#include "nm_basic.h"
/* ----- Global Define -------------------------------*/

/*----------------------------------------------------*/

/* ----- Function ------------------------------------*/
void VCan_Init(void);
void VCan_Start(void);
void VCan_Task(void);
void CAN_Rx_Data_Process(U8 ID,tMsg_CAN_Rx_Data_s *Rx_Data);
void CAN_Rx_Sas_Process(tMsg_CAN_SAS_Data_s *Rx_Data);
void CAN_Rx_Data_G_SEL_DISP_Process(tMsg_CAN_Rx_Data_s *Rx_Data);
void CAN_Rx_Data_IGN_SW_Process(tMsg_CAN_Rx_Data_s *Rx_Data);
void CAN_Rx_Data_RVM_CameraOff_Process(tMsg_CAN_Rx_Data_s *Rx_Data);
void CAN_Rx_Data_FCZC_RVM_SW_Process(tMsg_CAN_Rx_Data_s *Rx_Data);
void CAN_Rx_Data_Eng_Vol_Process(tMsg_CAN_Rx_Data_s *Rx_Data);
void CAN_Rx_Data_4WD_ERR_Process(tMsg_CAN_Rx_Data_s *Rx_Data);

void ISR_SAS(CanReceiveHandle rcvObject);
void ISR_HuType(CanReceiveHandle rcvObject);
void ISR_NaviOnOff(CanReceiveHandle rcvObject);
void ISR_CountryCfg(CanReceiveHandle rcvObject);
void ISR_LanguageInfo(CanReceiveHandle rcvObject);
void ISR_MdpsType(CanReceiveHandle rcvObject);
void ISR_RvmCameraOff(CanReceiveHandle rcvObject);
void ISR_IGNSw(CanReceiveHandle rcvObject);
void ISR_FczcRvmSw(CanReceiveHandle rcvObject);
void ISR_GSelpDisp(CanReceiveHandle rcvObject);
//void ISR_EngChr(CanReceiveHandle rcvObject);
void ISR_EngVol(CanReceiveHandle rcvObject);
void ISR_WD4Err(CanReceiveHandle rcvObject);

void ApplSAS1MsgTimeout(void);
void ApplHU_MON_PE_01MsgTimeout(void);
void ApplCGW4MsgTimeout(void);
void ApplCGW2MsgTimeout(void);
void ApplCLU15MsgTimeout(void);
void ApplMDPS11MsgTimeout(void);
void ApplHU_RVM_E_00TMsgTimeout(void);
void ApplCGW1MsgTimeout(void);
void ApplCGW3MsgTimeout(void);
void ApplCGW_PC5MsgTimeout(void);
void ApplWD411MsgTimeout(void);
void ApplEMS12MsgTimeout(void);

void ApplNmBasicSwitchTransceiverOn(NMBASIC_CHANNEL_APPLTYPE_ONLY);
void ApplNmBasicEnabledCom(NMBASIC_CHANNEL_APPLTYPE_ONLY);
void ApplNmBasicBusOffEnd(NMBASIC_CHANNEL_APPLTYPE_ONLY);
void ApplNmBasicBusOffRestart(NMBASIC_CHANNEL_APPLTYPE_ONLY);
void ApplNmBasicBusOffStart(NMBASIC_CHANNEL_APPLTYPE_ONLY);
void ApplNmBasicDisabledCom(NMBASIC_CHANNEL_APPLTYPE_ONLY);
void ApplNmBasicFirstBusOffSlow(NMBASIC_CHANNEL_APPLTYPE_ONLY);

/*----------------------------------------------------*/


/* ----- Global Value --------------------------------*/

/*----------------------------------------------------*/


/* ----- Extern Function -----------------------------*/

/*----------------------------------------------------*/











