#include "drv_can.h"

void Can_Init(void)
{
	U8 SFRPAGE_save = SFRPAGE;
	SFRPAGE  = CAN0_PAGE;               // All CAN register are on page 0x0C

	CAN0CN |= 0x01;                     // Start Intialization mode

	//---------Initialize general CAN peripheral settings

	CAN0CN |= 0x4E;                     // Enable Error and Module
	                                   // Enable access to bit timing register

	// See the CAN Bit Timing Spreadsheet for how to calculate this value
	//   CAN0BT = 0x1402;                    // Based on 24 Mhz CAN clock, set the
	                                   // CAN bit rate to 1 Mbps
	CAN0BT  = 0x3A02U; //bit14-12:
	                 //010 - TSEG1-1, (the time segment before the sample point) 
	                 //bit11-8:
	                 //0011 - TSEG2-1, (the time segment after the sample point)
	                 //bit7-6:
	                 //00 - SJW-1, ((Re)synchronisation jump width)
	                 //bit5-0:
	                 //00101 - BRP-1, (boud rate prescaler)

	//---------Initialize settings for Transmit Message Object 1

	// Command Mask Register
	CAN0IF1CM = 0x00F0;                 // Write Operation
	                                   // Transfer ID Mask, MDir, MXtd
	                                   // Transfer ID, Dir, Xtd, MsgVal
	                                   // Transfer Control Bits
	                                   // Don't set TxRqst or transfer data

	// Mask Registers
	CAN0IF1M1 = 0x0000;                 // Mask Bits 15-0 not used for filtering
	CAN0IF1M2 = 0x5FFC;                 // Ignore Extended Identifier for
	                                   // filtering
	                                   // Used Direction bit for filtering
	                                   // Use ID bits 28-18 for filtering

	// Message Control Registers
	CAN0IF1MC = 0x0080 | MESSAGE_SIZE;  // Disable Transmit Interrupt
	                                   // Message Object is a Single Message
	                                   // Message Size set by #define
	// Arbitration Registers
	CAN0IF1A1 = 0x0000;                 // 11-bit ID, so lower 16-bits not used

	// Arbitration Registers
	CAN0IF1A2 = 0xA000 | (MSG_ID_TX_BL_CMD << 2);   // Set MsgVal to valid
	                                               // Set Direction to write
	                                               // Set 11-bit Identifier

	CAN0IF1CR = MO_TX_BL_CMD;           // Start command request

	while (CAN0IF1CRH & 0x80) {}        // Poll on Busy bit

	//---------Initialize settings for Transmit Message Object 2

	// Can use the same settings for this transmit object, so no need reinitalize
	// the first five CAN registers again

	// Command Mask Register
	// CAN0IF1CM = 0x00F0;

	// Mask Registers
	// CAN0IF1M1 = 0x0000;
	// CAN0IF1M2 = 0x5FFC;

	// Message Control Registers
	// CAN0IF1MC = 0x0080 | MESSAGE_SIZE;

	// Arbitration Registers
	// CAN0IF1A1 = 0x0000;

	// Arbitration Registers
	CAN0IF1A2 = 0xA000 | (MSG_ID_TX_BL_WRITE8 << 2);   // Set MsgVal to valid
	                                                  // Set Direction to write
	                                                  // Set 11-bit Identifier

	CAN0IF1CR = MO_TX_BL_WRITE8;        // Start command request

	while (CAN0IF1CRH & 0x80) {}        // Poll on Busy bit

	//---------Initialize settings for Receive Message Object

	// Can use the same settings for Receive object, so no need reinitalize the
	// first four CAN registers again

	// Command Mask Register
	//   CAN0IF1CM = 0x00F0;

	// Mask Registers
	//  CAN0IF1M1 = 0x0000;
	//  CAN0IF1M2 = 0x5FFC;

	// Arbitration Registers
	//  CAN0IF1A1 = 0x0000;


	// Arbitration Registers
	CAN0IF1A2 = 0xA000 | (MSG_ID_TX_SAS << 2);   // Set MsgVal to valid
	                                               // Set Object Direction to read
	                                               // Set 11-bit Identifier

	CAN0IF1CR = MO_TX_SAS_ID;              // Start command request

	while (CAN0IF1CRH & 0x80) {}           // Poll on Busy bit

	//---------Initialize settings for Receive Message Object

	// Can use the same settings for Receive object, so no need reinitalize the
	// first four CAN registers again

	// Command Mask Register
	//   CAN0IF1CM = 0x00F0;

	// Mask Registers
	//  CAN0IF1M1 = 0x0000;
	//  CAN0IF1M2 = 0x5FFC;

	// Arbitration Registers
	//  CAN0IF1A1 = 0x0000;


	// Arbitration Registers
	CAN0IF1A2 = 0xA000 | (MSG_ID_TX_CLU << 2);   // Set MsgVal to valid
	                                               // Set Object Direction to read
	                                               // Set 11-bit Identifier

	CAN0IF1CR = MO_TX_CLU_ID;              // Start command request

	while (CAN0IF1CRH & 0x80) {}           // Poll on Busy bit
	//---------Initialize settings for Receive Message Object

	// Can use the same settings for Receive object, so no need reinitalize the
	// first four CAN registers again

	// Command Mask Register
	//   CAN0IF1CM = 0x00F0;

	// Mask Registers
	//  CAN0IF1M1 = 0x0000;
	//  CAN0IF1M2 = 0x5FFC;

	// Arbitration Registers
	//  CAN0IF1A1 = 0x0000;


	// Arbitration Registers
	CAN0IF1A2 = 0x8000 | (MSG_ID_RX_BL_RSP << 2);   // Set MsgVal to valid
	                                               // Set Object Direction to read
	                                               // Set 11-bit Identifier

	CAN0IF1CR = MO_RX_BL_RSP;              // Start command request

	while (CAN0IF1CRH & 0x80) {}           // Poll on Busy bit

	//diagnostic
	// CAN TX

	CAN0IF1A2 = 0xA000U | (MSG_ID_FUNCTION_REQ << 2U);  // Set MsgVal to valid

	CAN0IF1CR = MO_FUNCTION_REQ;                // Start command request

	while (CAN0IF1CRH & 0x80) {}           // Poll on Busy bit


	CAN0IF1A2 = 0xA000U | (MSG_ID_PGS_PHYS_ID << 2U);  // Set MsgVal to valid

	CAN0IF1CR = MO_PGS_PHYS_ID;                // Start command request

	while (CAN0IF1CRH & 0x80) {}           // Poll on Busy bit

	CAN0IF1MC = 0x1480U;
	// Arbitration Registers
	CAN0IF1A2 = 0x8000 | (MSG_ID_RECEIVE_ID << 2);   // Set MsgVal to valid
	                                               // Set Object Direction to read
	                                               // Set 11-bit Identifier
	CAN0IF1CR = MO_RECEIVE_ID;              // Start command request

	while (CAN0IF1CRH & 0x80) {}           // Poll on Busy bit
	//--------- CAN Initalization is complete

	CAN0CN &= ~0x41;                    // Return to Normal Mode and disable
	                                   // access to bit timing register


	//CAN0CN |= 0x80U;//Test Mode Enable
	//CAN0TST = 0x10U;//Loop Back Mode |= 0x10U;     //bit1:

	EIE2 |= 0x02;                       // Enable CAN interupts

	SFRPAGE = SFRPAGE_save;
}