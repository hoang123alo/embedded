/*_____ I N C L U D E __________________________________________*/
#include "compiler_defs.h"
#include "C8051F580_defs.h"
#include "Global_Define.h"
#include "stdio.h"
#include "drv_spi.h"
#include "drv_timer.h"

/*--------------------------------------------------------------*/
static SEG_XDATA volatile U8	gTemp	= 0x00U;

void Spi_Init (void)
{
	SEG_XDATA U8 SFRPAGE_save = SFRPAGE;

	SFRPAGE = ACTIVE_PAGE;

	SPI0CFG   = 0x40U;					// Enable the SPI as a Master
//	SPI0CFG   = 0x50U;					// Enable the SPI as a Master
//	SPI0CFG   = 0x70U;					// Enable the SPI as a Master
										// CKPHA = '0', CKPOL = '0'

	SPI0CN    = 0x0DU;					// 4-wire, single master mode
										// SPI0 enable

	SPI0CKR   = 0x00U;					//fSCK = SYSCLK/(2x(SPI0CKR+1)) 0x0BU = 1Mbps, 0x17 = 500Kbps

	SFRPAGE = SFRPAGE_save;
}

U8 spi_master_read(U8 addr)
{
	SEG_XDATA U8 dat = 0x00U; 
	SEG_XDATA U8 rd_addr = 0x00U;

	rd_addr = (U8)(addr & 0x3FU);
	rd_addr |= 0x80U;

	NSSMD0   = 0; // Activate Slave Select

	Flash_DataSet( rd_addr );
	Flash_Wait();
	SPIF     = 0U;
	SPI0DAT  = 0U; // Dummy write to output serial clock
	Flash_Wait();
	dat = (U8)SPI0DAT; // Read data

	SPIF	= 0U;
	NSSMD0  = 1U; // Deactivate Slave Select

	return dat;
}

void spi_master_write(U8 addr, U8 dat)
{
	SEG_XDATA U8	wr_addr = 0x00U;
	
	wr_addr = (U8)(addr & 0x3FU);

	NSSMD0   = 0U; // Activate Slave Select

	Flash_DataSet( wr_addr );
	Flash_Wait();
	Flash_DataSet( dat );
	Flash_Wait();

	SPIF	= 0U;
	NSSMD0  = 1U; // Deactivate Slave Select
}

void Flash_Wait( void )
{
	static SEG_XDATA volatile U8	reg_4val = 0x00U;

	while( 1 ){
		reg_4val = (U8)SPIF;
		if( reg_4val != 0x00U ){
			break;
		}
	}
}

void Flash_DataSet( U8 flash_data )
{
	SPIF     = 0U;
	SPI0DAT  = flash_data;
}

void Flash_Slave_Select( void )
{
	SPIF	= 0U;
	NSSMD0  = 1U;
}

void Flash_Slave_Command( U8 flash_cmd )
{
	NSSMD0   = 0U;                       // Activate Slave Select
	SPI0DAT  = flash_cmd;					// command
}

U8	Read_Guide_Value( U8 *Value )
{
	SEG_XDATA U8 rtn = 1U,i = 0U;

	NSSMD0  = 0U; // Activate Slave Select

	Flash_Slave_Command( (U8)EEPROM_CMD_RDDT );
	Flash_Wait();

	Flash_DataSet((U8)0x00U);
	Flash_Wait();

	Flash_DataSet((U8)0x00U);
	Flash_Wait();

	Flash_DataSet((U8)0x40U);
	Flash_Wait();

	for( i = 0; i < 6; i++)
	{
		Flash_DataSet( 0x00U );
		Flash_Wait();

		SPIF     = (U8)0;
		Value[i] = (U8)SPI0DAT; 
	}

	NSSMD0  = 1U; // Deactivate Slave Select

	return rtn;
}

U8	Check_Flash_ID( void )
{
	SEG_XDATA U8 rtn = 1U,i = 0U;

	// Send the command
	Flash_Slave_Command( (U8)EEPROM_CMD_REMS );
	Flash_Wait();
	
	Flash_DataSet( 0x00U );
	Flash_Wait();

	Flash_DataSet( 0x00U );
	Flash_Wait();

	Flash_DataSet( 0x00U );
	Flash_Wait();

	Flash_DataSet( 0x00U );
	Flash_Wait();

	SPIF     = 0U;
	gTemp = (U8)SPI0DAT;                 		// Read data before restoring SFR page

	if( gTemp != 0xC2U ){

		SPIF     = 0U;
		SPI0DAT  = 0U;                       // Dummy write to output serial clock
		Flash_Wait();
		// Send the command
		Flash_Slave_Command( (U8)EEPROM_CMD_REMS );
		Flash_Wait();
	
		Flash_DataSet( 0x00U );
		Flash_Wait();

		Flash_DataSet( 0x00U );
		Flash_Wait();

		Flash_DataSet( 0x00U );
		Flash_Wait();

		SPIF     = 0U;
		SPI0DAT  = 0U;                       // Dummy write to output serial clock
		Flash_Wait();
	gTemp = (U8)SPI0DAT;                 		// Read data before restoring SFR page

	if( gTemp != 0xEFU ){
		rtn = 0U;

		}else{
			// Read the value returned
			SPIF     = 0U;
			SPI0DAT  = 0U;                       // Dummy write to output serial clock
			Flash_Wait();

			Flash_Slave_Select();
			gTemp = (U8)SPI0DAT;                 	// Read data before restoring SFR page

			if( (gTemp != 0x11U) ){//2Mbit
				rtn = 0U;
			}
		}
		NSSMD0   = 1U;                       // Deactivate Slave Select
	}
	else
	{
		// Read the value returned
		SPI0DAT  = 0U;                       // Dummy write to output serial clock
		Flash_Wait();

		Flash_Slave_Select();
		gTemp = (U8)SPI0DAT;                 	// Read data before restoring SFR page

		if( (gTemp != 0x11U) ){//2Mbit
			rtn = 0U;
		}
		else
		{
			Flash_Slave_Command( (U8)EEPROM_CMD_RDSR );
			Flash_Wait();
			
			Flash_DataSet( 0x00U );
			Flash_Wait();
		
			SPIF     = 0U;
			gTemp = (U8)SPI0DAT;                 	// Read data before restoring SFR page

			NSSMD0   = 1U;                       // Deactivate Slave Select
		}
	}
	
	return rtn;
}
/*
U8	Check_Flash_ID( void )
{
	SEG_DATA S8 rtn = 1;

	rtn = 1;

	// Send the command
	Flash_Slave_Command( EEPROM_CMD_REMS );
	Flash_Wait();
	
	Flash_DataSet( 0x00U );
	Flash_Wait();

	Flash_DataSet( 0x00U );
	Flash_Wait();

	Flash_DataSet( 0x00U );
	Flash_Wait();

//	Flash_DataSet( 0x00U );
//	Flash_Wait();

	SPIF     = (U8)0;
	SPI0DAT  = (U8)0;                       // Dummy write to output serial clock
	Flash_Wait();
	gTemp = (U8)SPI0DAT;                 		// Read data before restoring SFR page

	//if( gTemp != 0xC2U ){
	if( gTemp != 0xEFU ){
		rtn = 0;
	}
	else
	{
		// Read the value returned
		//SPI0DAT  = (U8)0;                       // Dummy write to output serial clock
		//Flash_Wait();
		SPIF     = (U8)0;
		SPI0DAT  = (U8)0;                       // Dummy write to output serial clock
		Flash_Wait();

		Flash_Slave_Select();
		gTemp = (U8)SPI0DAT;                 	// Read data before restoring SFR page

///		if( (gTemp != 0x13U) ){	//8Mbit or 2Mbit
///			rtn = 0;
///		}
///		else if((gTemp != 0x11U) ){//8Mbit or 2Mbit

//		if( (gTemp != 0x13U) && (gTemp != 0x11U) ){	//8Mbit or 2Mbit
//			rtn = 0;
//		}
		if( (gTemp != 0x11U) ){//2Mbit
			rtn = 0;
		}
		
		else
		{
			Flash_Slave_Command( EEPROM_CMD_RDSR );
			Flash_Wait();
			
//			Flash_DataSet( 0x00U );
//			Flash_Wait();
			
			SPIF     = (U8)0;
			SPI0DAT  = (U8)0;                       // Dummy write to output serial clock
			Flash_Wait();
			gTemp = (U8)SPI0DAT;                 	// Read data before restoring SFR page

			SPI0DAT  = (U8)0;                       // Dummy write to output serial clock
			Flash_Wait();
			gTemp = (U8)SPI0DAT;                 	// Read data before restoring SFR page

			NSSMD0   = (U8)1;                       // Deactivate Slave Select
		}
		
	}

	NSSMD0   = (U8)1;                       // Deactivate Slave Select

	return rtn;
}


void	Flash_Read( U8 *Addr, U16 Len )
{
	SEG_DATA U16 lp;

	if( Flash_is_OK == 1)
	{
		// Send the command
		Flash_Slave_Command( EEPROM_CMD_FAST_READ );
		Flash_Wait();
		
		Flash_DataSet( Addr[0] );
		Flash_Wait();

		Flash_DataSet( Addr[1] );
		Flash_Wait();

		Flash_DataSet( Addr[2] );
		Flash_Wait();

		Flash_DataSet( 0x00U );
		Flash_Wait();

		SPIF     = (U8)0;

		// Read the value returned

		for( lp = 0x0000U; lp < Len; lp++ ){
			SPI0DAT  = (U8)0;					// Dummy write to output serial clock
			Flash_Wait();

			SPIF     = (U8)0;
			Value[i] = (U8)SPI0DAT;		// Read data before restoring SFR page			
		}
	
		NSSMD0   = (U8)1;						// Deactivate Slave Select
	}
}*/

void	Flash_Write( U32 Addr,U8 *Data, U16 Len )
{
	SEG_XDATA U16 lp = (U16)0x00U;

		// Send the command
		Flash_Slave_Command( (U8)EEPROM_CMD_WREN );
		Flash_Wait();

		Flash_Slave_Select();
	
		Wait_ms(10U);

		Flash_Slave_Command( (U8)EEPROM_CMD_PP );
		Flash_Wait();

		Flash_DataSet( ((U8)(Addr>>16U) & 0xFFU) );
		Flash_Wait();

		Flash_DataSet( ((U8)(Addr>>8U) & 0xFFU) );
		Flash_Wait();

		Flash_DataSet( (U8)(Addr & 0xFFU) );
		Flash_Wait();

		SPIF     = 0U;

		// Read the value returned
		for( lp = 0x0000U; lp < Len; lp++ )
		{
			Flash_DataSet( Data[lp] );
			Flash_Wait();

			SPIF     = 0U;
		}
	
		NSSMD0   = 1U;		// Deactivate Slave Select
	
		Wait_ms(10U);

		NSSMD0   = 0U;

		while( 1 )
		{
			Flash_DataSet( (U8)EEPROM_CMD_RDSR );
			Flash_Wait();

			SPIF     = 0U;
			SPI0DAT  = 0U;                    // Dummy write to output serial clock
			Flash_Wait();
			gTemp = (U8)SPI0DAT;                 // Read data before restoring SFR page

			if( (gTemp & (U8)EEPROM_STATUS_WIP) == 0x00U ){	// write in progress bit
				break;
			}
		}
	
		NSSMD0   = 1U;						// Deactivate Slave Select
	
		Wait_ms(10U);

		Flash_Slave_Command( (U8)EEPROM_CMD_WRDI );
		Flash_Wait();

		Flash_Slave_Select();

}


void	Flash_EraseSector( U32 Addr )
{
		// Send the command
		Flash_Slave_Command( (U8)EEPROM_CMD_WREN );
		Flash_Wait();

		Flash_Slave_Select();
	
		Wait_ms(10U);

		Flash_Slave_Command( (U8)EEPROM_CMD_SE );
		Flash_Wait();

		Flash_DataSet( ((U8)(Addr>>16U)&0xFFU) );
		Flash_Wait();

		Flash_DataSet( ((U8)(Addr>>8U)&0xFFU) );
		Flash_Wait();

		Flash_DataSet( (U8)(Addr&0xFFU) );
		Flash_Wait();

		Flash_Slave_Select();

		Wait_ms(10U);

		NSSMD0   = 0U;

		while( 1 )
		{
			SPI0DAT  = EEPROM_CMD_RDSR;			// status register read
			Flash_Wait();

			SPIF     = 0U;
			SPI0DAT  = 0U;                       // Dummy write to output serial clock
			Flash_Wait();
			gTemp = (U8)SPI0DAT;                 	// Read data before restoring SFR page

			if( (gTemp & (U8)EEPROM_STATUS_WIP) == 0x00U ){	// write in progress bit
				break;
			}
			Wait_ms(10U);
		}
	
		NSSMD0   = 1U;						// Deactivate Slave Select
	
		Wait_ms(10U);

		Flash_Slave_Command( (U8)EEPROM_CMD_WRDI );
		Flash_Wait();
		
		Flash_Slave_Select();
}

/* end of file */
