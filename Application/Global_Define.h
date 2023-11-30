/*----------TYPE-------------------*/

#define CAMERA
#define SYSCLK 24000000
/*----------SENSOR-----------------*/

//	#define APTINA_126
//	#define APTINA_128
	#define APTINA_AP0100			0x03U
	#define APTINA_ASX344			0x04U
//	#define APTINA_ASX350AT


/*----------EEPROM-----------------*/
#define MXIC
//#define WINBOND

/*----------DEVICE-----------------*/
//#define UART
#define SPI
#define I2C							
#define ADC
#define WATCHDOG
#define PCA
#define CAN

#define EXINTERRUPT

/*----------I2C---------------*/	// Use Timer1
//#define I2C_SPEED_100KHZ			
#define I2C_SPEED_400KHZ		

/*----------WATCHDOG---------------*/
#define WATCHDOG_ENABLE
//#define WATCHDOG_32MS
#define WATCHDOG_400MS

/*----------VDDMONITOR-------------*/
#define VDDMONITOR_ENABLE
//#define VDDMONITOR_HIGH
#define VDDMONITOR_LOW

/*----------TIMER-------------*/
//#define TIMER0 					//Use Wait_ms()
#define TIMER1						//Use SMBUS0
#define TIMER2						//1ms interrupt1
#define TIMER3						//Use SMBUS0 low time out
//#define TIMER4					//1ms interrupt2
#define TIMER5						//Use PCA0 (Watchdog)

/*----------ETC..-----------------*/
#define TRUE 	(U8)1
#define FALSE 	(U8)0

#define HIGH 	(U8)1
#define LOW 	(U8)0

//CAN DATA
//#define MDPS11				
//#define LANGUAGEINFO		
//#define HUTYPE				
//#define GATEWAY				
//#define NAVIONOFF			
//#define G_SEL_DISP		    
//#define CAM_REARCAMERA_REQ	

//Operate Mode---------------------
#define OPERATE_STANDBY 	(U8)1
#define OPERATE_INTERINIT	(U8)2
#define OPERATE_IDLE 		(U8)3
#define OPERATE_RUNNING 	(U8)4

#define DTC_RECORD_WRITE
#define DTC_RECORD_CLEAR

//Define Error
#define NO_ERROR			(U8)0x00
#define ERROR_RX_COMM		(U8)0x01	//RX Error
#define ERROR_TX_COMM		(U8)0x02	//TX Error

//#define ERROR_TX_COMM		(U8)0x02	//RX Error

//BANK SET
#define BANK0				(U8)0x00
#define BANK1				(U8)0x10
#define BANK2				(U8)0x20
#define BANK3				(U8)0x30

//DIAGNOSTIC TEST
//#define DIAG_TEST_MCU_WD_ERR
//#define DIAG_TEST_MCU_MC_ERR
//#define DIAG_TEST_ISP_FV_ERR
//#define DIAG_TEST_ISP_FC_ERR
//#define DIAG_TEST_ISP_IS_ERR
//#define DIAG_TEST_ISP_CM_ERR
//#define DIAG_TEST_ISP_IT_ERR
//#define DIAG_TEST_VID_OC_ERR
//#define DIAG_TEST_VID_VO_ERR

