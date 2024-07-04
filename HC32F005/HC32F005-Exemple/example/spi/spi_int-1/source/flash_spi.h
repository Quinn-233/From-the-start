#include "hc32f005.h"
#include "ddl.h"
#include "spi.h"
#include "gpio.h"
#include "base_types.h"
#include "stdbool.h"

#define ZD25_FLASH_SIZE (1024*1024*16)

#define ZD25X10 0xEF10
#define ZD25Q20 0xEF11 
#define ZD25Q40 0xEF12
#define ZD25Q80 0xEF13
#define ZD25Q16 0xEF14
#define ZD25Q32 0xEF15
#define ZD25Q64 0xEF16
#define ZD25Q128 0xEF17

extern u16 SPI_FLASH_TYPE;
	
#define ZD25X_FLASH_ERASE_CHIP   0
#define ZD25X_FLASH_ERASE_SECTOR 1
#define ZD25X_FLASH_ERASE_BLOCK_32K  2
#define ZD25X_FLASH_ERASE_BLOCK_64K  3

#define ZD25X_FLASH_READ_STATUSREG1  0
#define ZD25X_FLASH_READ_STATUSREG2  1
#define ZD25X_FLASH_READ_STATUSREG3  2


#define ZD25X_WriteEnable  0x06
#define ZD25X_WriteDisable 0x04
#define ZD25X_ReadStatusReg 0x05
#define ZD25X_WriteStatusReg 0x01
#define ZD25X_ReadData   0x03
#define ZD25X_FastReadData 0x0B
#define ZD25X_FastReadDua1 0x3B
#define ZD25X_PageProgram 0x02
#define ZD25X_32K_BlockErase 0x52
#define ZD25X_64K_BlockErase 0xD8
#define ZD25X_SectorErase 0xD8
#define ZD25X_ChipErase 0xc7
#define ZD25X_PowerDown 0xB9
#define ZD25X_ReleasePowerDown 0xAB
#define ZD25X_DeviceID  0xAB
#define ZD25X_ManufactDeviceID 0x90
#define ZD25X_JedecDeviceID 0x9f



#define ZD25XFLASH_REG_BIT_BUSY (1<<0)
#define ZD25XFLASH_REG_BIT_WEL  (1<<1)
#define ZD25XFLASH_REG_BIT_BPX  (0x7<<2)
#define ZD25XFLASH_REG_BIT_TB   (1<<5)
#define ZD25XFLASH_REG_BIT_SRP  (1<<7)

void ZD25_Flash_Init(void);


u8 ZD25_Flash_Read_SR(void);

void ZD25_FLASH_Write_SR(u8 sr);

u16 ZD25_Flash_ReadID(void);

u32 ZD25_Flash_ReadJedecID(void);

void ZD25_Flash_Read(u32 ReadAddr,u8* pBuffer,u32 NumByteToRead);

void ZD25_Flash_FastRead(u32 ReadAddr, u8* pBuff,u32 NumByteToRead);

void ZD25X_Flash_PageProgram(u32 WriteAddr,u8* pBuffer,u16 NumByteToWrite);

void ZD25_FLASH_Erase(u8 Type,u32 EraseAddr);

void ZD25_Flash_Write_NoCheck(u32 WriteAddr,u8* pBuffer,u16 NumByteToWrite);

void ZD25_Flash_Write(u32 WriteAddr, u8* pBuffer,u16 NumByteToWrite);

