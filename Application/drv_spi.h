#ifndef DRV_SPI_H
#define DRV_SPI_H

// Target MCU Commands
#define TGT_CMD_ENTER_BL_MODE    (0x90U)
#define TGT_CMD_GET_INFO         (0x91U)
#define TGT_CMD_SET_FLASH_KEYS   (0x92U)
#define TGT_CMD_SET_ADDR         (0x93U)
#define TGT_CMD_ERASE_PAGE       (0x94U)
#define TGT_CMD_WRITE_FLASH      (0x95U)
#define TGT_CMD_GET_PAGE_CRC     (0x96U)
#define TGT_CMD_WRITE_SIGNATURE  (0x97U)
#define TGT_CMD_SW_RESET         (0x98U)
#define TGT_CMD_SW_INFO          (0x9FU)

// EEPROM
#define  EEPROM_CAPACITY			(1048576)	// EEPROM Capacity (bytes)
#define  EEPROM_PAGE_SIZE			(256)		// EEPROM Page Size (bytes)
#define  EEPROM_SECTOR_SIZE			(4096)		// EEPROM Sector Size (bytes)
#define  EEPROM_BLOCK_SIZE			(65536)		// EEPROM Block Size (bytes)
#define  EEPROM_CMD_WREN			(0x06U)		// write enable
#define  EEPROM_CMD_RDDT			(0x03U)		// read data
#define  EEPROM_CMD_WRDI			(0x04U)		// write disable
#define  EEPROM_CMD_RDSR			(0x05U)		// status register read
#define  EEPROM_CMD_FAST_READ		(0x0BU)		// +AD1,AD2,AD3+Dummy fast read data
#define  EEPROM_CMD_REMS			(0x90U)		// +Dummy,Dummy,ADD read electronic manufacturer & device ID)
#define  EEPROM_CMD_SE				(0x20U)		// +AD1,AD2,AD3 sector erase (4KB) 0~255 sector
#define  EEPROM_CMD_PP				(0x02U)		// +AD1,AD2,AD3 page program (256 byte)
#define  EEPROM_STATUS_WIP			(0x01U)		// write in progress bit

// Reserved space first byte
#define RESERVED_SPACE_ADDR			(31*1024)	// 0x7C00U

void Flash_Wait(void);
void Flash_DataSet(U8 flash_data);
void Flash_Slave_Select(void);
void Flash_Slave_Command(U8 flash_cmd);
void Update_Flash_Proc(U8 Mode, U8 Buf);
void Flash_EraseSector( U32 Addr );
void Spi_Init (void);
U8	Check_Flash_ID( void );
U8 spi_master_read(U8 addr);
void spi_master_write(U8 addr, U8 dat);
void	Flash_Write( U32 Addr,U8 *Data, U16 Len );
U8	Read_Guide_Value( U8 *Value );
#endif


