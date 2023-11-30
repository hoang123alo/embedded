/* ----- Global Define -------------------------------*/
#ifndef MGR_ISP_H
#define MGR_ISP_H

#include "mgr_comm.h"

#define LOAD_COMMAND_SIZE 				(4)
#define CLEAR_COMMAND_SIZE 				(3)

#define STEERING_ANGLE_LIMIT 		(0x0028U)

//G_SEL_DISP
#define GEAR_P						0x00U
#define GEAR_D						0x05U
#define GEAR_N						0x06U
#define GEAR_R						0x07U
#define GEAR_INVALID				0x09U
#define GEAR_INTERMEDIATE			0x0EU

//IGN SW
#define IGN_SW_KEY_OFF				0x00U
#define IGN_SW_ON					0x03U

//RVM CameraOff
#define RVM_CAMERA_DEFAULT			0x00U
#define RVM_CAMERA_OFF				0x01U
#define RVM_CAMERA_INVALID			0x03U

//FCZC RVM SW
#define FCZC_RVM_SW_OFF				0x00U
#define FCZC_RVM_SW_ON				0x01U

//RVM VIEW
#define RVM_VIEW_OFF				0x00U
#define RVM_PARKING_ASSIST_VIEW		0x01U
#define RVM_DRIVING_ASSIST_VIEW		0x02U
#define RVM_VIEW_INVALID			0x0FU

//RVM SW IND
#define RVM_SW_IND_OFF				0x00U
#define RVM_SW_IND_ON				0x01U
#define RVM_SW_IND_INVALID			0x03U

#define NORMAL_3M_GUIDE_LINE_ADDR  		(0x0001)

//RVM CONFIG
#define		 CMD_PARKING_CONFIG			(0x0000U)
#define		 CMD_DRIVING_CONFIG			(0x0001U)

#define		CHECKING_LAYER_DATA			(0x4F08U)
#define		CHECKING_LAYER0_DATA		(0x4F0AU)
#define		CHECKING_LAYER1_DATA		(0x4F0CU)
#define		CHECKING_LAYER2_DATA		(0x4F0EU)

#define		CMD_PARAM_POOL_0_ADDR		(0xFC00U)
#define		CMD_PARAM_POOL_1_ADDR		(0xFC01U)
#define		CMD_PARAM_POOL_2_ADDR		(0xFC02U)
#define		CMD_PARAM_POOL_3_ADDR		(0xFC03U)
#define		CMD_PARAM_POOL_4_ADDR		(0xFC04U)
#define		CMD_PARAM_POOL_5_ADDR		(0xFC04U)
#define		CMD_PARAM_POOL_6_ADDR		(0xFC06U)
#define		CMD_PARAM_POOL_7_ADDR		(0xFC07U)
#define		CMD_PARAM_POOL_8_ADDR		(0xFC08U)

#define		CMD_REGISTER_ADDR			(0x0040U)

#define		CMD_SET_STATE				(0x8100U)
#define		CMD_GET_STATE				(0x8101U)

#define		CMD_OVRL_LOAD_BUFFER		(0x8206U)
#define		CMD_OVRL_LOAD_STATUS		(0x8207U)

#define		OVRL_ENABLE_DATA			(0x0100U)
#define		OVRL_DISABLE_DATA			(0x0000U)

#define		OVR_LAYER0_DATA				(0x0000U)
#define		OVR_LAYER1_DATA				(0x0100U)
#define		OVR_LAYER2_DATA				(0x0200U)
#define		OVR_LAYER3_DATA				(0x0300U)

#define		BUFFER0_LAYER0_DATA			(0x0000U)
#define		BUFFER1_LAYER0_DATA			(0x0100U)
#define		BUFFER2_LAYER0_DATA			(0x0200U)
#define		BUFFER3_LAYER0_DATA			(0x0300U)
#define		BUFFER4_LAYER0_DATA			(0x0400U)
#define		BUFFER5_LAYER0_DATA			(0x0500U)
#define		BUFFER6_LAYER0_DATA			(0x0600U)

#define		BUFFER0_LAYER1_DATA			(0x0001U)
#define		BUFFER1_LAYER1_DATA			(0x0101U)
#define		BUFFER2_LAYER1_DATA			(0x0201U)
#define		BUFFER3_LAYER1_DATA			(0x0301U)
#define		BUFFER4_LAYER1_DATA			(0x0401U)
#define		BUFFER5_LAYER1_DATA			(0x0501U)
#define		BUFFER6_LAYER1_DATA			(0x0601U)

#define		BUFFER0_LAYER2_DATA			(0x0002U)
#define		BUFFER1_LAYER2_DATA			(0x0102U)
#define		BUFFER2_LAYER2_DATA			(0x0202U)
#define		BUFFER3_LAYER2_DATA			(0x0302U)
#define		BUFFER4_LAYER2_DATA			(0x0402U)
#define		BUFFER5_LAYER2_DATA			(0x0502U)
#define		BUFFER6_LAYER2_DATA			(0x0602U)

#define		BUFFER0_LAYER3_DATA			(0x0003U)
#define		BUFFER1_LAYER3_DATA			(0x0103U)
#define		BUFFER2_LAYER3_DATA			(0x0203U)
#define		BUFFER3_LAYER3_DATA			(0x0303U)
#define		BUFFER4_LAYER3_DATA			(0x0403U)
#define		BUFFER5_LAYER3_DATA			(0x0503U)
#define		BUFFER6_LAYER3_DATA			(0x0603U)

#define		EXTRA_BUFFER0_DATA			(BUFFER0_LAYER0_DATA)
#define		EXTRA_BUFFER1_DATA			(BUFFER1_LAYER0_DATA)
#define		EXTRA_BUFFER2_DATA			(BUFFER2_LAYER0_DATA)
#define		EXTRA_BUFFER3_DATA			(BUFFER3_LAYER0_DATA)
#define		EXTRA_BUFFER4_DATA			(BUFFER4_LAYER0_DATA)
#define		EXTRA_BUFFER5_DATA			(BUFFER5_LAYER0_DATA)
#define		EXTRA_BUFFER6_DATA			(BUFFER6_LAYER0_DATA)

#define		WARNING_BUFFER0_DATA		(BUFFER0_LAYER1_DATA)
#define		WARNING_BUFFER1_DATA		(BUFFER1_LAYER1_DATA)
#define		WARNING_BUFFER2_DATA		(BUFFER2_LAYER1_DATA)
#define		WARNING_BUFFER3_DATA		(BUFFER3_LAYER1_DATA)
#define		WARNING_BUFFER4_DATA		(BUFFER4_LAYER1_DATA)
#define		WARNING_BUFFER5_DATA		(BUFFER5_LAYER1_DATA)
#define		WARNING_BUFFER6_DATA		(BUFFER6_LAYER1_DATA)

#define		GUIDELINE_BUFFER0_DATA		(BUFFER0_LAYER2_DATA)
#define		GUIDELINE_BUFFER1_DATA		(BUFFER1_LAYER2_DATA)
#define		GUIDELINE_BUFFER2_DATA		(BUFFER2_LAYER2_DATA)
#define		GUIDELINE_BUFFER3_DATA		(BUFFER3_LAYER2_DATA)
#define		GUIDELINE_BUFFER4_DATA		(BUFFER4_LAYER2_DATA)
#define		GUIDELINE_BUFFER5_DATA		(BUFFER5_LAYER2_DATA)
#define		GUIDELINE_BUFFER6_DATA		(BUFFER6_LAYER2_DATA)

#define		FIXED_GUIDELINE_BUFFER0_DATA		(BUFFER0_LAYER3_DATA)
#define		FIXED_GUIDELINE_BUFFER1_DATA		(BUFFER1_LAYER3_DATA)
#define		FIXED_GUIDELINE_BUFFER2_DATA		(BUFFER2_LAYER3_DATA)
#define		FIXED_GUIDELINE_BUFFER3_DATA		(BUFFER3_LAYER3_DATA)
#define		FIXED_GUIDELINE_BUFFER4_DATA		(BUFFER4_LAYER3_DATA)
#define		FIXED_GUIDELINE_BUFFER5_DATA		(BUFFER5_LAYER3_DATA)
#define		FIXED_GUIDELINE_BUFFER6_DATA		(BUFFER6_LAYER3_DATA)

// Language Address in Flash
#define		MEM_FIXED_GUIDE_ADDR			(0x0074U)	// 0x006DU 0x0048U
#define		MEM_FIXED_GUIDE_ADDR_D			(0x0075U)
#define		MEM_BLACK_IMAGE_ADDR			(0x0076U)
#define		MEM_LAN_START_ADDR				(0x0053U)	// 0x004CU 0x0048U
#define		MEM_LAN_INVALID_ADDR			(0x0072U)	// 0x006CU
#define		CAN_LAN_OFF_ADDR				(0x0000U)
#define		CAN_LAN_INVALID_ADDR			(0x001FU)

#define AP0100_SENSOR_X_OFFSET 	0xc8A8U
#define AP0100_SENSOR_Y_OFFSET 	0xc8A9U

#define ASX344_SENSOR_X_OFFSET 	0xc860U
#define ASX344_SENSOR_Y_OFFSET 	0xc861U

#define AP0100_FRAME_COUNT_REGISTER	 	0x8006U
#define ASX344_FRAME_COUNT_REGISTER	 	0x8004U

#define SENSOR_INIT						0x01U
#define SENSOR_RUNNING				 	0x02U
#define SENSOR_STATE_STREAMING		 	0x3100U

/*----------------------------------------------------*/
/* ----- Function ------------------------------------*/
void	Init_IspTask(void);
void	ui_event_process(void);
void	steering_angle_display_guide_line(S8 display_flag);
void Sensor_XY_Offset_set(U8 x, U8 y );
U8		CheckCommandStatusLoop(void);

void Init_IspTask(void);
void Isp_InterInit(void);
void Operate_IspTask(void);
void Isp_Parking_Assist_View(void);
void Isp_Driving_Assist_View(void);
void Isp_View_Mode_Change(volatile U8 ViewMode);
void Isp_Off(void);
void Isp_On(void);

void Fixed_Guidline_On(const U8 ViewMode);
void Fixed_Guidline_Off(void);
void Guidline_On(void);
void Guidline_Off(void);
void isp_guide_on_off(U8 Guide_On_Off);
void V_STB_Check(void);
void Video_Signal_Check(void);
void Frame_Count_Check(void);
void Frame_Valide_Check(void);
void Check_Isp_InitState(void);
/*--------------------*/

/* ----- Global Value --------------------------------*/

/*----------------------------------------------------*/
typedef struct {
	U8 IGNSw;
	U8 Pre_Gear;
	U8 Cur_Gear;
	U8 Pre_RVM_CamOff;
	U8 Cur_RVM_CamOff;
	U8 Pre_RVM_SW;
	U8 Cur_RVM_SW;
	U8 Pre_SW_IND;
	U8 Cur_SW_IND;
	U8 Pre_ViewMode;
	U8 Cur_ViewMode;
	U8 RVM_SW_Status;
}tMsg_ISP_Function;

/* ----- Extern Function -----------------------------*/
extern SEG_XDATA volatile tMsg_CAN_SAS_Data_s tCAN_Rx_SAS_Msg;
extern SEG_XDATA volatile tMsg_CAN_Rx_Data_s tCAN_Rx_Msg[USE_CAN_COUNT];
extern SEG_XDATA volatile U8 g_I2C_Err_Cnt;  // Counter for the number of errors.
extern SEG_XDATA volatile tMsg_ISP_Function tISP_Func_Msg;
/*----------------------------------------------------*/

#endif


