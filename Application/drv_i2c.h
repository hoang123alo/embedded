#ifndef DRV_I2C_H
#define DRV_I2C_H


/* ----- Global Define -------------------------------*/
#define  WRITE          0x00           // SMBus WRITE command
#define  READ           0x01U           // SMBus READ command
#define  SMB_MTSTA      0xE0U           // (MT) start transmitted
#define  SMB_MTDB       0xC0U           // (MT) data byte transmitted
#define  SMB_MRDB       0x80U           // (MR) data byte received

#define  I2C_1BYTE_RW  	0x01U           // (MR) data byte received
#define  I2C_2BYTE_RW   0x02U           // (MR) data byte received

#if defined(APTINA_AP0100)
#define ISP_SLAVE_ADDRESS	0x90U
#endif

#if defined(APTINA_AP0100)
#define ISP_SLAVE_ADDRESS	0x90U
#endif
/*----------------------------------------------------*/


/* ----- Global Value --------------------------------*/

/*----------------------------------------------------*/


/* ----- Function ------------------------------------*/
void SMBUS0_Init (void);
void SMB_Write (void);
void SMB_Read (void);
U16	 i2c_master_read(U8 slave_addr, U16 addr, U8 Byte_Length);
void i2c_master_write(U8 slave_addr, U16 addr, U16 dat, U8 Byte_Length);

/*----------------------------------------------------*/


/* ----- INTERRUPT -----------------------------------*/
INTERRUPT_PROTO (TIMER3_ISR, INTERRUPT_TIMER3);
INTERRUPT_PROTO (SMBUS0_ISR, INTERRUPT_SMBUS0);
/*----------------------------------------------------*/

/* ----- Extern Function -----------------------------*/
extern SEG_XDATA volatile U8 g_I2C_Err_Cnt;
/*----------------------------------------------------*/

#endif


