/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "drv_mem.h"
#include "drv_timer.h"

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void FLASH_ResetTrap (void);
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
volatile U8 FLKEY1 = 0x00U;
volatile U8 FLKEY2 = 0x00U;
//-----------------------------------------------------------------------------
// FLASH Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FLASH_SetFlashKey
//-----------------------------------------------------------------------------
// Performs math to calculate the two flash-key codes stored in global memory.
// These codes will be passed as parameters to flash write/erase functions.
// Therefore, this should be called only immediately before one of these
// functions.
void FLASH_SetFlashKey(void)
{
   FLKEY1 = 0xA0U;
   FLKEY1 |= 0x05U;

   FLKEY2 = 0xF0U;
   FLKEY2 |= 0x01U;
}
//-----------------------------------------------------------------------------
// FLASH_clearFlashKey
//-----------------------------------------------------------------------------
// Cleras the global flash-key codes. To be called immediately after the keys
// are passed to a flash write/erase function.
void FLASH_ClearFlashKey(void)
{
   FLKEY1 = 0x00U;
   FLKEY2 = 0x00U;
}
//-----------------------------------------------------------------------------
// FLASH_ResetTrap
//-----------------------------------------------------------------------------
// Resets the device. To be placed immediately before flash write/erase routines
// In the case that a 'fall-through' occurs, this function will be executed
// before any flash writes/erases, causing the device to reset.
void FLASH_ResetTrap (void)
{
   SFRPAGE = 0x00U; // Set SFRPAGE to 0x00 for access to RSTSRC register
   RSTSRC  = 0x12U; // Initiate a software reset, Keep VDD Monitor enabled
}
//-----------------------------------------------------------------------------
// F560_FLASH_ByteWrite
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1) FLADDR addr - address of the byte to write to
//                    valid range is 0x0000 to 0x7BFF for 32K Flash devices
//                    valid range is 0x0000 to 0x3FFF for 16K Flash devices
//   2) U8 byte - byte to write to Flash.
//
// This routine writes <byte> to the linear FLASH address <addr>.
//
// This routine conforms to the recommendations in the C8051F56x data sheet
// 
// If the MCU is operating from the internal voltage regulator, the VDD
// monitor should be set threshold and enabled as a reset source only when
// writing or erasing Flash. Otherwise, it should be set to the low threshold.
//
// If the MCU is operating from an external voltage regulator powering VDD
// directly, the VDD monitor can be set to the high threshold permanently.
//-----------------------------------------------------------------------------
U8 FLASH_ByteWrite (FLADDR addr, U8 byte, U8 bank, U8 flkey1, U8 flkey2)
{
#ifndef __PST_PolySpace__
   bit EA_SAVE = EA;                   // Preserve EA
#else
   U8 EA_SAVE = (U8)EA;                      // Preserve EA
#endif
   U8 xdata * data pwrite;             // FLASH write pointer
   U8 ret = 0x00U;
   U8 i = 0U;
   U8 Save_Bank = PSBANK;			       	 // Preserve PSBANK
   U8 SFRPAGE_save = SFRPAGE;

   FLASH_ClearFlashKey();

   EA = 0U;                             // Disable interrupts
   EA = 0U;                                  // Disable interrupts

   if (!( (bank == BANK2) && ((addr >= 0xF800U) && (addr <= 0xFFFFU)) ))
   {
	   SFRPAGE = 0x00U;						// Set SFRPAGE to 0x00 for access to RSTSRC register
	   RSTSRC  = 0x12U;						// Initiate a software reset, Keep VDD Monitor enabled
   }

   FLSCL |= 0x02U;						     // FLEWT = 1

   SFRPAGE = ACTIVE_PAGE;
   PSBANK = bank;						     // COBANK
   	
   RSTSRC = 0x04U; //0x00U | 0x04U;          // 1. Disable VDD monitor as a reset source

   VDM0CN = 0xA0U;                      // 2. Enable VDD monitor and high threshold

   for (i = 0U; i < 20U; i++) {}        // 3. Wait for VDD monitor to stabilize

   if ( (VDM0CN & 0x40U) == 0x00U)               // 4. If the VDD voltage is not high
   {
      ret = 0U;                        //    enough don't write to Flash
   	  VDM0CN = 0x80U;         
	  for (i = 0U; i < 20U; i++) {}        // 3. Wait for VDD monitor to stabilize
  	  RSTSRC = 0x06U; //0x02U | 0x04U;
   }
   else
   {
	   ret = 1U;   
	   RSTSRC = 0x06U; //0x02U | 0x04U;      // 5. Safe to enable VDD Monitor as a 
	                                       //    reset source

	   pwrite = (U8 xdata *) addr;
	                                       // 6. Enable Flash Writes
	   PSCTL |= 0x01U;                       // PSWE = 1 which enables writes
	   PSCTL &= ~0x02U;                      // PSEE = 0

	   FLKEY  = flkey1;             		// Key Sequence 1
	   FLKEY  = flkey2;             		// Key Sequence 2

	   VDM0CN = 0xA0U;                      // 7. Enable VDD monitor and high threshold

	   RSTSRC = 0x06U; //0x02U | 0x04U;      // 8. Enable VDD monitor as a reset source

	   *pwrite = byte;                     // 9. Write the byte

	   RSTSRC = 0x04U; //0x00U | 0x04U;      // 10. Disable the VDD monitor as reset 
	                                       //     source
	   VDM0CN = 0x80U;                      // 11. Change VDD Monitor to low threshold
      for (i = 0U; i < 20U; i++) {}        // 3. Wait for VDD monitor to stabilize
	   RSTSRC = 0x06U; //0x02U | 0x04U;      // 12. Re-enable the VDD monitor as a 
	                                       //     reset source

	   PSCTL &= ~0x01U;                     // PSWE = 0 which disable writes
   }
   PSBANK = Save_Bank;
   SFRPAGE = SFRPAGE_save;

   FLSCL &= ~0x02U;					     // FLEWT = 0
	   EA = EA_SAVE;                       // Restore interrupts

   return ret;                               // Write completed successfully
}

//-----------------------------------------------------------------------------
// F560_FLASH_ByteRead
//-----------------------------------------------------------------------------
//
// Return Value :
//      U8 - byte read from Flash
// Parameters   :
//   1) FLADDR addr - address of the byte to read to
//                    valid range is 0x0000 to 0x7BFF for 32K Flash devices
//                    valid range is 0x0000 to 0x3FFF for 16K Flash devices
//
// This routine reads a <byte> from the linear FLASH address <addr>.
//-----------------------------------------------------------------------------

U8 FLASH_ByteRead (FLADDR addr, U8 bank)
{
#ifndef __PST_PolySpace__
   bit EA_SAVE = EA;                   // Preserve EA
#else
   U8 EA_SAVE = (U8)EA;                 // Preserve EA
#endif
   U8 code * data pread;               // FLASH read pointer
   U8 byte = 0x00U;
   U8 Save_Bank = PSBANK;				// Preserve PSBANK
   S8 SFRPAGE_SAVE = SFRPAGE;           // Preserve SFRPAGE

   EA = 0U;                             // Disable interrupts
   EA = 0U;                             // Disable interrupts
		
   SFRPAGE = ACTIVE_PAGE;

   PSBANK = bank;						// COBANK

   pread = (U8 code *) addr;

   byte = *pread;                      // Read the byte

   PSBANK = Save_Bank;
   SFRPAGE = SFRPAGE_SAVE;              // Restore SFRPAGE
   EA = EA_SAVE;                       // Restore interrupts

   return byte;
}

//-----------------------------------------------------------------------------
// F560_FLASH_PageErase
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1) FLADDR addr - address of any byte in the page to erase
//                    valid range is 0x0000 to 0x79FF for 32K Flash devices
//                    valid range is 0x0000 to 0x3DFF for 16K Flash devices
//
// This routine erases the FLASH page containing the linear FLASH address
// <addr>.  Note that the page of Flash containing the Lock Byte cannot be
// erased from application code.
//
//// This routine conforms to the recommendations in the C8051F56x data sheet
//
// If the MCU is operating from the internal voltage regulator, the VDD
// monitor should be set threshold and enabled as a reset source only when
// writing or erasing Flash. Otherwise, it should be set to the low threshold.
//
// If the MCU is operating from an external voltage regulator powering VDD
// directly, the VDD monitor can be set to the high threshold permanently.
//-----------------------------------------------------------------------------

U8 FLASH_PageErase (FLADDR addr,U8 bank, U8 flkey1, U8 flkey2)
{
#ifndef __PST_PolySpace__
   bit EA_SAVE = EA;                   // Preserve EA
#else
   U8 EA_SAVE = (U8)EA;
#endif                  // Preserve EA
   U8 xdata * data pwrite;             // FLASH write pointer
   U8 ret = 0x00U;
   U8 i = 0x00U;
   U8 Save_Bank = PSBANK;			    	// Preserve PSBANK
   U8 SFRPAGE_save = SFRPAGE;
   
   FLASH_ClearFlashKey();

   EA = 0U;                                 // Disable interrupts
   EA = 0U;

   if (!( (bank == BANK2) && ((addr >= 0xF800U) && (addr <= 0xFFFFU)) ))
   {
	   SFRPAGE = 0x00U; 					// Set SFRPAGE to 0x00 for access to RSTSRC register
	   RSTSRC  = 0x12U; 					// Initiate a software reset, Keep VDD Monitor enabled
   }

   FLSCL |= 0x02U;					    	// FLEWT = 1

   SFRPAGE = ACTIVE_PAGE;
   PSBANK = bank;					        // COBANK

   RSTSRC = 0x04U; // 0x00U | 0x04U;        // 1. Disable VDD monitor as a reset source

   VDM0CN = 0xA0U;                      // 2. Enable VDD monitor and high threshold

   for (i = 0U; i < 20U; i++) {}        // 3. Wait for VDD monitor to stabilize

   if ( (VDM0CN & 0x40U) == 0x00U )               // 4. If the VDD voltage is not high enough
   {
      ret = 0U;                        //    don't attempt to write to Flash
   	  VDM0CN = 0x80U;         
	  for (i = 0U; i < 20U; i++) {}        // 3. Wait for VDD monitor to stabilize
  	  RSTSRC = 0x06U; //0x02U | 0x04U;
   }
   else
   {	
   	  ret = 1U;
   
	   RSTSRC = 0x06U; //0x02U | 0x04U;     // 5. Safe to enable VDD Monitor as a reset 
	                                       //    source
	   pwrite = (U8 xdata *) addr;
	                                       // 6. Enable Flash Writes
	   PSCTL |= 0x03U;                      // PSWE = 1; PSEE = 1

	   FLKEY  = flkey1;            // Key Sequence 1
	   FLKEY  = flkey2;            // Key Sequence 2

	   VDM0CN = 0xA0U;                      // 7. Enable VDD monitor and high threshold
	   RSTSRC = 0x06U; //0x02U | 0x04U;     // 8. Enable VDD monitor as a reset source

	   *pwrite = 0U;                        // 9. Initiate page erase

	   RSTSRC = 0x04U; //0x00U | 0x04U;     // 10. Disable the VDD monitor as a reset	                                        // source
	   VDM0CN = 0x80U;                      // 11. Change VDD Monitor to low threshold

      for (i = 0U; i < 20U; i++) {}        // 3. Wait for VDD monitor to stabilize
	   RSTSRC = 0x06U; //0x02U | 0x04U;     // 12. Re-enable the VDD monitor as a reset 
	                                       //     source

	   PSCTL &= ~0x03U;                     // PSWE = 0; PSEE = 0
   }
   PSBANK = Save_Bank;					    // Restore PSBANK
   SFRPAGE = SFRPAGE_save;				    // Restore SFRPAGE

   FLSCL &= ~0x02U;							// FLEWT = 0
	   EA = EA_SAVE;                       // Restore interrupts

   return ret;
}


void FLASH_Write_Buf (FLADDR dest, U8 *src, U16 numbytes,U8 bank)
{
	FLADDR i = 0x0000U;
   SEG_XDATA U8 index = 0x00U;

   for (i = dest; i < (dest+numbytes); i++) 
   {
		FLASH_SetFlashKey();
		FLASH_ByteWriteWithKey (i,src[index], bank);
	  index++;
   }
}

void FLASH_Read_Buf (U8 *dest, FLADDR src, U16 numbytes,U8 bank)
{
	FLADDR i = 0x0000U;
   SEG_XDATA U8 index = 0x00U;

   for (i = 0; i < numbytes; i++) 
   {
		dest[index] = FLASH_ByteRead (src+i, bank);
	  index++;
   }
}

void FLASH_Erase_Buf (FLADDR dest,U8 bank)
{
	FLASH_SetFlashKey();
	FLASH_PageEraseWithKey (dest, bank);
}
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
