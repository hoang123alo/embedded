#include "compiler_defs.h"
#include "C8051F580_defs.h"            // SFR declarations
#include "drv_i2c.h"
#include "drv_timer.h"
#include "mgr_diag.h"
#include "global_define.h"

static SEG_XDATA volatile U8   number_wd = 0U;   
static SEG_XDATA volatile U8   number_rd = 0U;

static SEG_XDATA volatile U8 SMB_DATA_IN[4] = {0x00U,0x00U,0x00U,0x00U};
static SEG_XDATA volatile U8 SMB_DATA_OUT[4] = {0x00U,0x00U,0x00U,0x00U};

static SEG_XDATA volatile U8 TARGET = 0x00U;  // Target SMBus slave address
static SEG_XDATA volatile U8 SMB_BUSY = 0U;    // Software flag to indicate when the
                                       // SMB_Read() or SMB_Write() functions
                                       // have claimed the SMBus
static SEG_XDATA volatile U8 SMB_RW = 0x00U;  // Software flag to indicate the
                                       // direction of the current transfer
static SEG_XDATA volatile U8 NUM_ERRORS = 0U;  // Counter for the number of errors.

SEG_XDATA volatile U8 g_I2C_Err_Cnt = 0x00U;  // Counter for the number of errors.

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
#ifndef __PST_PolySpace__
INTERRUPT_PROTO (TIMER3_ISR, INTERRUPT_TIMER3);
INTERRUPT_PROTO (SMBUS0_ISR, INTERRUPT_SMBUS0);
#else
void TIMER3_ISR(void);
void SMBUS0_ISR(void);
#endif

void SMB_Write (void);
void SMB_Read (void);

void SMBUS0_Init (void)
{
   SEG_XDATA U8 i = 0U;

   SEG_XDATA U8 SFRPAGE_save = SFRPAGE;
   SFRPAGE = ACTIVE_PAGE;

   SMB0CF = (U8)0x5D;                     // Use Timer1 overflows as SMBus clock
                                       // source;
                                       // Disable slave mode;
                                       // Enable setup & hold time
                                       // extensions;
                                       // Enable SMBus Free timeout detect;
                                       // Enable SCL low timeout detect;

   SMB0CF |= (U8)0x80;                    // Enable SMBus;
	
   EIE1 |= (U8)0x01;                     // Enable the SMBus interrupt

   SFRPAGE = SFRPAGE_save;


/*------Global Value Init------*/

	for (i = 0; i < 4U; i++)
	{
		SMB_DATA_IN[i] = 0x00U;
		SMB_DATA_OUT[i] = 0x00U;
	}
	number_wd 	= 0x00U;   
	number_rd 	= 0x00U;
	
	TARGET 		= 0x00U;
	SMB_BUSY 	= 0x00U;
	SMB_RW 		= 0x00U;
	NUM_ERRORS 	= 0x00U;

}

U16 i2c_master_read(U8 slave_addr, U16 addr, U8 Byte_Length)
{
	SEG_XDATA U16 dat = (U16)0x00U;
	SEG_XDATA U8 i = 0U;

	number_wd = 1;   
	number_rd = 1;

	NUM_ERRORS = 0;                     	// Error counter

	//SMB_DATA_OUT[0] = (U8)((addr>>8)&0x00FFU);
	if ( Byte_Length == I2C_2BYTE_RW )
	{
		number_wd = 2;   
		number_rd = 2;
		SMB_DATA_OUT[0] = (U8)((addr>>8U)&0x00FFU);
		SMB_DATA_OUT[1] = (U8)(addr & 0x00FFU);
	}
	else
	{
		number_wd = 1;   
		number_rd = 1;
		SMB_DATA_OUT[0] = (U8)(addr & 0x00FFU);
	}


	TARGET = slave_addr;             	// Target the Slave for next SMBus transfer
	SMB_Write();                     		// Initiate SMBus write

	// SMBus Read Sequence
	TARGET = slave_addr;            	// Target the Slave for next SMBus transfer
	SMB_Read();								// Initiate SMBus read

	dat = (U16)SMB_DATA_IN[0];
	if ( Byte_Length == I2C_2BYTE_RW )
	{
		dat = (dat<<8U) | (U16)SMB_DATA_IN[1];
	}

 	for (i = 0; i < 4U; i++)
	{
		SMB_DATA_IN[i] = 0x00U;
	}

	if( NUM_ERRORS > 0U ){
		NUM_ERRORS = 0U;
		dat = 0x0000U; //correct for prevent rolling F and 0 in count error
	}

	return dat;
}


void i2c_master_write(U8 slave_addr, U16 addr, U16 dat, U8 Byte_Length)
{
	SEG_XDATA volatile U8   addr_num = 0U; 
	NUM_ERRORS = 0U;                     	// Error counter
	
	if ( Byte_Length == I2C_2BYTE_RW )
	{
		number_wd = 4U;   
		number_rd = 0U;

		SMB_DATA_OUT[0] = (U8)((addr>>8U)&0x00FFU);
		SMB_DATA_OUT[1] = (U8)(addr & 0x00FFU);
		SMB_DATA_OUT[2] = (U8)((dat>>8U)&0x00FFU);
		SMB_DATA_OUT[3] = (U8)(dat & 0x00FFU);
	}
	else
	{
		number_wd = 2U;   
		number_rd = 0U;
		SMB_DATA_OUT[0] = (U8)(addr & 0x00FFU);
		SMB_DATA_OUT[1] = (U8)(dat & 0x00FFU);
	}
	TARGET = slave_addr;             	// Target the Slave for next SMBus transfer
	SMB_Write();                     		// Initiate SMBus write

	if( NUM_ERRORS > 0U ){
		NUM_ERRORS = 0U;
	}
}


//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Timer3 Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
//
// A Timer3 interrupt indicates an SMBus SCL low timeout.
// The SMBus is disabled and re-enabled here
//
//-----------------------------------------------------------------------------
#ifndef __PST_PolySpace__
INTERRUPT(TIMER3_ISR, INTERRUPT_TIMER3)
#else
void TIMER3_ISR(void)	// Frame Sync
#endif
{
	SMB0CF &= (U8)(~0x80U);                    	// Disable SMBus
	SMB0CF |= (U8)0x80U;                     		// Re-enable SMBus
	TMR3CN &= (U8)(~0x80U);                    	// Clear Timer3 interrupt-pending flag
	STA = (U8)0x00;
	SMB_BUSY = 0U;                       		// Free SMBus
}

//-----------------------------------------------------------------------------
// SMBus Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
//
// SMBus ISR state machine
// - Master only implementation - no slave or arbitration states defined
// - All incoming data is written to global variable array <SMB_DATA_IN>
// - All outgoing data is read from global variable array <SMB_DATA_OUT>
//
//-----------------------------------------------------------------------------
#ifndef __PST_PolySpace__
INTERRUPT(SMBUS0_ISR, INTERRUPT_SMBUS0)
#else
void SMBUS0_ISR(void)	// Frame Sync
#endif
{
	static SEG_XDATA U8 status = 0x00U;
	static SEG_XDATA U8 rec_byte_counter = 0U;
	static SEG_XDATA U8 sent_byte_counter = 0U;

	SEG_XDATA U8 FAIL = 0U;	// Used by the ISR to flag failed transfers
	
	status = (U8)ARBLOST;

	if (status == 0x00U)	// Check for errors
	{
		// Normal operation
		status = (U8)SMB0CN;
		status &=  (U8)0xF0;

		switch (status)	// Status vector
		{
			// Master Transmitter/Receiver: START condition transmitted.
			case SMB_MTSTA:
				SMB0DAT = (U8)(TARGET | SMB_RW);	// Load address of the target slave
				STA = (U8)0x00;				// Manually clear START bit
				rec_byte_counter = 1U;		// Reset the counter
				sent_byte_counter = 1U;		// Reset the counter
				break;

			// Master Transmitter: Data byte transmitted
			case SMB_MTDB:
				status = (U8)ACK;
				if (status != 0x00U)	// Slave ACK?
				{
					if (SMB_RW == WRITE)	// If this transfer is a WRITE,
					{
						if (sent_byte_counter <= number_wd)
						{
							// send data byte
							SMB0DAT = SMB_DATA_OUT[(sent_byte_counter-1U)];
							sent_byte_counter++;
						}
						else
						{
							STO = (U8)0x01;	// Set STO to terminate transfer
							SMB_BUSY = 0U;	// And free SMBus interface
						}
					}
				}
				else // If slave NACK,
				{
					STO = (U8)0x01;	// Send STOP condition, followed
					STA = (U8)0x01;	// By a START
					NUM_ERRORS++;	// Indicate error
				}
				break;

         // Master Receiver: byte received
         case SMB_MRDB:
            if (rec_byte_counter < number_rd)
            {
               SMB_DATA_IN[(rec_byte_counter-1U)] = (U8)SMB0DAT;	// Store received byte
               ACK = (U8)0x01;										// Send ACK to indicate byte received
               rec_byte_counter++;								// Increment the byte counter
            }
            else
            {
               SMB_DATA_IN[(rec_byte_counter-1U)] = (U8)SMB0DAT;	// Store received byte
               ACK = (U8)0x00;										// Send NACK to indicate last byte of this transfer
               STO = (U8)0x01;										// Send STOP to terminate transfer

			   SMB_BUSY = 0U;									// Free SMBus interface
            }
            break;

         default:
            FAIL = 1U;	// Indicate failed transfer and handle at end of ISR
            break;

      } // end switch
   }
   else
   {
      // ARBLOST = 1, error occurred... abort transmission
      FAIL = 1U;
   } // end ARBLOST if

   if (FAIL == 1U)	// If the transfer failed,
   {
      SMB0CF &= (U8)(~0x80U);	// Reset communication
      SMB0CF |= (U8)0x80U;
      STA = (U8)0x00U;
      STO = (U8)0x00U;
      ACK = (U8)0x00U;

      SMB_BUSY = 0U;				// Free SMBus

      FAIL = 0U;

      NUM_ERRORS++;				// Indicate an error occurred

      //DTC_ISP_CM_ERROR;
   }

   SI = (U8)0x00;	// Clear interrupt flag
}

//-----------------------------------------------------------------------------
// Support Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SMB_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Writes a single byte to the slave with address specified by the <TARGET>
// variable.writeSpiFlash
// Calling sequence:
// 1) Write target slave address to the <TARGET> variable
// 2) Write outgoing data to the <SMB_DATA_OUT> variable array
// 3) Call SMB_Write()
//
//-----------------------------------------------------------------------------
void SMB_Write (void)
{
   SEG_XDATA U8 SFRPAGE_save = SFRPAGE;
   SEG_XDATA U8 On_Time = FALSE;
   SFRPAGE = ACTIVE_PAGE;

   SMB_BUSY = 1U;                       // Claim SMBus (set to busy)
   SMB_RW = 0x00U;                         // Mark this transfer as a WRITE
   STA = 0x01U;                            // Start transfer

	Delay_Time_Set(TID_I2C_BUSY_CHECK,DT_I2C_BUSY_CHECK);
	while (SMB_BUSY == 1U)
	{
		On_Time = Delay_Time_Get(TID_I2C_BUSY_CHECK);
		if ( On_Time == TRUE)
		{
			SMB_BUSY = 0U;
			g_I2C_Err_Cnt++;
		}
	}
	Delay_Time_Expire(TID_I2C_BUSY_CHECK);
   SFRPAGE = SFRPAGE_save;
}


//-----------------------------------------------------------------------------
// SMB_Read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Reads a single byte from the slave with address specified by the <TARGET>
// variable.
// Calling sequence:
// 1) Write target slave address to the <TARGET> variable
// 2) Call SMB_Write()
// 3) Read input data from <SMB_DATA_IN> variable array
//
//-----------------------------------------------------------------------------
void SMB_Read (void)
{
   SEG_XDATA U8 On_Time = FALSE;
   SEG_XDATA U8 SFRPAGE_save = SFRPAGE;
   SFRPAGE = ACTIVE_PAGE;

   SMB_BUSY = 1U;                       // Claim SMBus (set to busy)
   SMB_RW = 0x01U;                         // Mark this transfer as a READ

   STA = (U8)0x01;                            // Start transfer

	Delay_Time_Set(TID_I2C_BUSY_CHECK,DT_I2C_BUSY_CHECK);
	while (SMB_BUSY == 1U)
	{
		On_Time = Delay_Time_Get(TID_I2C_BUSY_CHECK);
		if ( On_Time == TRUE)
		{
			SMB_BUSY = 0U;
			g_I2C_Err_Cnt++;
		}
	}
	Delay_Time_Expire(TID_I2C_BUSY_CHECK);

   SFRPAGE = SFRPAGE_save;
}
