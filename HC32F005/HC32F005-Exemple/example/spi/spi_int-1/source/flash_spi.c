#include "flash_spi.h"

#if 1
void SPI_Delay(uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}

void SPI_Config(void)
{
	Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
	Gpio_InitIOExt(SPI_PORT_CSN, SPI_PIN_CSN, GpioDirOut, 0, 0, 0, 0);
	Gpio_SetIO(SPI_PORT_CSN, SPI_PIN_CSN, 1);
	Gpio_InitIOExt(SPI_PORT_SCK, SPI_PIN_SCK, GpioDirOut, 0, 0, 0, 0);
	Gpio_SetIO(SPI_PORT_SCK, SPI_PIN_SCK, 0);
	Gpio_InitIOExt(SPI_PORT_MISO, SPI_PIN_MISO, GpioDirIn, 0, 0, 0, 0);
//	Gpio_SetIO(SPI_PORT_MISO, SPI_PIN_MISO, 0);
	Gpio_InitIOExt(SPI_PORT_MOSI, SPI_PIN_MOSI, GpioDirOut, 0, 0, 0, 0);
	Gpio_SetIO(SPI_PORT_MOSI, SPI_PIN_MOSI, 0);
}

static void SPI_SendCMD(uint8_t CMD)
{
	uint8_t bit_ctr;
	uint8_t buff = CMD;
	for(bit_ctr=0; bit_ctr<8; bit_ctr++) 
	{
		// SPI-MOSI发数据
		if(buff & 0x80) MOSI_H; 		        
		else MOSI_L;
		SCK_H; 
		SPI_Delay(0xff);
		buff = buff << 1;           
		SCK_L; 
		SPI_Delay(0xff);           		 
	}
}

uint8_t SPI_WritePage(uint8_t *TxData, uint64_t Addr, uint16_t sendlen)
{		
	uint8_t bit_ctr, i, res;
	uint8_t buff[3];
	
	buff[0] = (Addr&0xff0000) >> 16;
	buff[1] = (Addr&0x00ff00) >> 8;
	buff[2] = (Addr&0x0000ff) >> 0;
	
	if(sendlen <= 256){
		CSN_L;
		SPI_SendCMD(ZD25X_WriteEnable);
		CSN_H;
		
		CSN_L;
		SPI_SendCMD(ZD25X_PageProgram);
		for(i=0; i<3; i++){
			for(bit_ctr=0; bit_ctr<8; bit_ctr++){
				if(buff[i] & 0x80) MOSI_H; 		        
				else MOSI_L;
				SCK_H; 
				SPI_Delay(0xff);
				buff[i] = buff[i] << 1;           
				SCK_L; 
				SPI_Delay(0xff);           		 
			}
		}
	
		for(i=0; i<sendlen; i++){
			for(bit_ctr=0; bit_ctr<8; bit_ctr++) 
			{
				if(*(TxData+i) & 0x80) MOSI_H; 		        
				else MOSI_L;
				SCK_H; 
				SPI_Delay(0xff);
				*(TxData+i) = (*(TxData+i) << 1);           
				SCK_L; 
				SPI_Delay(0xff);           		 
			}
		}
		CSN_H;
		
		CSN_L;
		SPI_SendCMD(ZD25X_WriteDisable);
		CSN_H;
		
		res = 0;
	}else{
		res = -1;
	}

	return res;
}

/********** 读取flash数据 ***********
**
**  *RxData
************************************/
void SPI_ReadByte(uint8_t *RxData, uint64_t Addr, uint8_t readlen)
{		
	uint8_t bit_ctr, i;
	uint8_t buff[3];

	buff[0] = (Addr&0xff0000) >> 16;
	buff[1] = (Addr&0x00ff00) >> 8;
	buff[2] = (Addr&0x0000ff) >> 0;
	
	CSN_L;
	
	SPI_SendCMD(ZD25X_ReadData);
	for(i=0; i<3; i++){
		for(bit_ctr=0; bit_ctr<8; bit_ctr++){
			if(buff[i] & 0x80) MOSI_H; 		        
			else MOSI_L;
			SCK_H; 
			SPI_Delay(0xff);
			buff[i] = buff[i] << 1;           
			SCK_L; 
			SPI_Delay(0xff);           		 
		}
	}
	for(i=0; i<readlen; i++){
		for(bit_ctr=0; bit_ctr<8; bit_ctr++) 
		{
			SCK_H; 
			SPI_Delay(0xff);         
			if(READ_MISO){
				*(RxData+i) = *(RxData+i) << 1;
				*(RxData+i) = (*(RxData+i) | 0x01); 
			}
			SCK_L; 
			SPI_Delay(0xff);
		}
	}
	
	CSN_H;
}
#endif




#if 0
//SPI_HandleTypeDef ZD25_Handle;
//SPI句柄结构变量声明

//Flash初始化
//void Flash_Init()
//{
//	 SPIx_Init(&ZD25_Handle);
//	 //初始化SPI接口
//}

u8 SPI_ReadWriteByte()
{
	
}

void ZD25_SPI_FLASH_CSL(void)
{
	Spi_SetCS(FALSE);
}

void ZD25_SPI_FLASH_CSH(void)
{
	Spi_SetCS(TRUE);
}

//Flash读写接口函数        （写入的数据）
u8 ZD25_Flash_ReadWriteByte(u8 TxData)
{
	//读写一个字节的数据
	return SPI_ReadWriteByte(&ZD25_Handle,TxData);
	 
}


u8 ZD25_Flash_Read_SR(void)
{
     u8 byte = 0 ;
     ZD25_SPI_FLASH_CSL();
     //使能
	ZD25_Flash_ReadWriteByte(ZD25X_ReadStatusReg);
	//发送读取状态寄存器命令（0x05）
     byte = ZD25_Flash_ReadWriteByte(0xFF);
     //读取一个字节
     ZD25_SPI_FLASH_CSH();
     //取消是能
     return byte;
}


//没用到
void ZD25_FLASH_Write_SR(u8 sr)
{
   ZD25_SPI_FLASH_CSL();
   //使能器件
   ZD25_Flash_ReadWriteByte(ZD25X_WriteStatusReg);
   //发送写取状态寄存器命令（0x01）
   ZD25_Flash_ReadWriteByte(sr);
   //写入一个字节
   ZD25_SPI_FLASH_CSH();
   //取消片选
}


void ZD25_FLASH_Write_Enable(void)
{
   ZD25_SPI_FLASH_CSL();
   //使能器件
   ZD25_Flash_ReadWriteByte(ZD25X_WriteEnable);
   //发送写取状态寄存器命令
   ZD25_SPI_FLASH_CSH();
   //取消选中的FLASH
}

//**没用到**
void ZD25_FLASH_Write_Disable(void)
{
    ZD25_SPI_FLASH_CSL();
    //使能器件
    ZD25_Flash_ReadWriteByte(ZD25X_WriteDisable); 
    //发送写禁止指令（0x04）
    ZD25_SPI_FLASH_CSH();
    //取消使能
}


u16 ZD25_Flash_ReadID(void)
{
   u16 Temp = 0;
   
   ZD25_SPI_FLASH_CSL();
  //选中FLASH
 ZD25_Flash_ReadWriteByte(ZD25X_ManufactDeviceID);	//发送读取ID命令 （0x90）；
   ZD25_Flash_ReadWriteByte(0x00);
   ZD25_Flash_ReadWriteByte(0x00);
   ZD25_Flash_ReadWriteByte(0x00);
   //发送24位地址
   Temp |= ZD25_Flash_ReadWriteByte(0xFF)<<8;
   Temp |= ZD25_Flash_ReadWriteByte(0xFF);
   //返回16位地址
   ZD25_SPI_FLASH_CSH();
  //取消选中FLash
   return Temp;
}   

u32 ZD25_Flash_ReadJedecID(void)
//读取芯片JedecID
{
	 u32 Temp = 0;
	
	ZD25_SPI_FLASH_CSL();
	//选中FLASH
	ZD25_Flash_ReadWriteByte(ZD25X_JedecDeviceID);//发送读取ID命令  （0x9f）
	Temp |= ZD25_Flash_ReadWriteByte(0xFF)<<16;
	Temp |= ZD25_Flash_ReadWriteByte(0xFF)<<8;
	Temp |= ZD25_Flash_ReadWriteByte(0xFF);
	//读取24位数据（刚开始8位左移16或上 8位移动
	//8 再或上 8位 ）
	//取反、bai左移、按位与、按位异或、du按位或）
	//优先级由zhi高到低的顺序排列为取反 > 左移 
	//> 按位与 > 按位异或dao > 按位或。
	ZD25_SPI_FLASH_CSH();
	//取消选中FLASH
	
	return Temp;
}


void ZD25_Flash_Read(u32 ReadAddr,u8* pBuffer,u32 NumByteToRead)
{
	 u16 i;
	 
   ZD25_SPI_FLASH_CSL();
   //使能器件
   
   ZD25_Flash_ReadWriteByte(ZD25X_ReadData);
   //发送读命令
   ZD25_Flash_ReadWriteByte((u8)((ReadAddr) >> 16));
   ZD25_Flash_ReadWriteByte((u8)((ReadAddr)>>8));
   ZD25_Flash_ReadWriteByte((u8)ReadAddr);
 //发送要读取的32位地址
   for(i=0;i<NumByteToRead;i++)
   {
          pBuffer[i] = ZD25_Flash_ReadWriteByte(0xff);
          //循环读数
	 }
   ZD25_SPI_FLASH_CSH();
 //禁止

   while((ZD25_Flash_ReadSR()&ZD25XFLASH_REG_BIT_BUSY) ==0x01 );
   //如果器件忙，等待
 }

void ZD25_Flash_FastRead(u32 ReadAddr,u8* pBuffer,u32 NumByteToRead)
{
    u16 i;
   
    ZD25_SPI_FLASH_CSL();
//使能

    ZD25_Flash_ReadWriteByte(ZD25X_FastReadData);
 //发送高速读命令0x0b，3字节地址   （0x0b）
    ZD25_Flash_ReadWriteByte(((ReadAddr)>>16));
    ZD25_Flash_ReadWriteByte(((ReadAddr)>>8));
    ZD25_Flash_ReadWriteByte(ReadAddr);
    //发送32bit地址
    ZD25_Flash_ReadWriteByte(0xFF);
//产生8个空闲时钟周期脉冲
    for(i=0;i<NumByteToRead;i++)
    {
       pBuffer[i] = ZD25_Flash_ReadWriteByte(0xFF);
       //循环度数
    }
    ZD25_SPI_FLASH_CSH();


    while(ZD25_Flash_ReadSR()&ZD25XFLASH_REG_BIT_BUSY);//器件忙等待
}


void ZD25X_Flash_PageProgram(u32 WriteAddr,u8* pBuffer,u16 NumByteToWrite)
//页编程（写入的地址，写数据存储的缓存区，写入的值的长度）
{
    u32 i;
    
    ZD25_FLASH_Write_Enable();
	 //发送写使能命令 


    ZD25_SPI_FLASH_CSL();
    //发送低速读写命令0xAD，3字节地址，写入的字节

    ZD25_Flash_ReadWriteByte(ZD25X_PageProgram);
    ZD25_Flash_ReadWriteByte(((WriteAddr)>>16));
    ZD25_Flash_ReadWriteByte(((WriteAddr)>>8));
   
    ZD25_Flash_ReadWriteByte(WriteAddr);
//一共发送32bit地址

    for(i=0; i<NumByteToWrite;i++)
    //发送其他字节
    {

       ZD25_Flash_ReadWriteByte(pBuffer[i]);
       //编程
		}
       ZD25_SPI_FLASH_CSH();
   //取消

      while(ZD25_Flash_ReadSR()&ZD25XFLASH_REG_BIT_BUSY);//如果器件忙，等待
}


void ZD25_FLASH_Erase(u8 Type,u32 EraseAddr)
//FLASH的擦除操作（擦除类型，擦除首地址）
{
	
	ZD25_FLASH_Write_Enable();
	//发送写使能命令
	switch(Type)
	{
		case ZD25X_FLASH_ERASE_CHIP:
		//整片擦除
		ZD25_SPI_FLASH_CSL();
		//选中使能FLASH
		ZD25_Flash_ReadWriteByte(ZD25X_ChipErase);
		//发送擦除整片命令（0xc7）
		ZD25_SPI_FLASH_CSH();
		//取消选中flash
		break;
		case ZD25X_FLASH_ERASE_SECTOR:
		//4K扇区擦除
		ZD25_SPI_FLASH_CSL();
	    //使能	
	    ZD25_Flash_ReadWriteByte(ZD25X_SectorErase);
	    //发送4K扇区擦除命令(0xd8)	
        ZD25_Flash_ReadWriteByte(((EraseAddr)>>16));
	    //擦除前16位
        ZD25_Flash_ReadWriteByte(((EraseAddr)>>8));
        //擦除前8位
        ZD25_Flash_ReadWriteByte(EraseAddr);
        //擦除完	
		ZD25_SPI_FLASH_CSH();
		//取消
		break;
		case ZD25X_FLASH_ERASE_BLOCK_32K:
		//32K块擦除
		ZD25_SPI_FLASH_CSL();
		//使能
		ZD25_Flash_ReadWriteByte(ZD25X_32K_BlockErase);
		//发送擦除命令（0x52）
		ZD25_Flash_ReadWriteByte(((EraseAddr)>>16));
		ZD25_Flash_ReadWriteByte(((EraseAddr)>>8));
		ZD25_Flash_ReadWriteByte(EraseAddr);
		ZD25_SPI_FLASH_CSH();
		break;
		case ZD25X_FLASH_ERASE_BLOCK_64K:
		//64K块擦除
		ZD25_SPI_FLASH_CSL();
	    ZD25_Flash_ReadWriteByte(ZD25X_64K_BlockErase);
		ZD25_Flash_ReadWriteByte(((EraseAddr)>>16));
		ZD25_Flash_ReadWriteByte(((EraseAddr)>>8));
		ZD25_Flash_ReadWriteByte(EraseAddr);
		ZD25_SPI_FLASH_CSH();
		break;
		default:break;
	}
	while(ZD25_Flash_ReadSR()&ZD25XFLASH_REG_BIT_BUSY);//如果器件忙等待
}

//未用到
void ZD25_Flash_Write_NoCheck(u32 WriteAddr, u8* pBuffer, u16 NumByteToWrite)
//无检验写SPI 在指定的位置开始写入指定长度数据
{
	u16 pageremain;
	
	pageremain = 256-WriteAddr%256;
	//单页剩余数字
	if(NumByteToWrite <= pageremain)
		pageremain = NumByteToWrite;
		//不大于256字节
	
	while(1)
	{
		 ZD25X_Flash_PageProgram (WriteAddr,pBuffer,pageremain);
		 if(NumByteToWrite==pageremain)
			 break;
			 //写入结束
		 else//NumByteToWrite>pageremain
		 {
			 pBuffer+=pageremain;
			 WriteAddr+=pageremain;
			 NumByteToWrite-=pageremain;
			 //减去已经写入了的字节数
			 if(NumByteToWrite>256)
				 pageremain = 256;
				 //一次可以写入256个字节
			 else
				 pageremain=NumByteToWrite;
				 //不够256个字节
		 }
	};
}


//没用到

u8 ZD25SPI_FLASH_BUFFER[4096];
//写SPI FLASH，在指定地址开始写入指定长度的数据
void ZD25_Flash_Write(u32 WriteAddr,u8* pBuffer,u16 NumByteToWrite)
#if 0
{
	 u32 secpos;
	 //扇区地址
	 u16 secoff;
	 //扇区内偏移
	 u16 secremain;
	 //扇区剩余空间
	 u16 i;
	 u8 * SPI_FLASH_BUF;
	 
	 SPI_FLASH_BUF = ZD25SPI_FLASH_BUFFER;
	 //可以通过申请内存的方式实现
	 secpos = WriteAddr/4096;
	 //扇区地址
	 secpos = WriteAddr%4096;
	 //扇区内的偏移
	 secremain = 4096-secoff;
	 //扇区内剩余空间的大小
	 
	  if(NumByteToWrite <= secremain)
			secremain = NumByteToWrite;
			//不大于4096个字节
		while(1)
		{
			 ZD25_Flash_FastRead(secpos*4096,SPI_FLASH_BUF,4096);//读出整个扇区的内容
			for(i=0;i<secremain;i++)
			//校验数据
			{
				if(SPI_FLASH_BUF[secoff+i] != 0XFF)
					break;
					//需要擦除
			}
			if(i<secremain)//需要擦除
			{
				ZD25_FLASH_Erase(ZD25X_FLASH_ERASE_SECTOR,secpos*4096);//擦除这个扇区
				for(i=1;i<secremain;i++)
				//复制
				{
					SPI_FLASH_BUF[i+secoff]=pBuffer[i];
				}
				ZD25_Flash_Write_NoCheck(secpos*4096,SPI_FLASH_BUF,4096);//写入整个扇区
			}
			else
			{
				 ZD25_Flash_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的，直接写入扇区剩余区间
			}
			
			if(NumByteToWrite == secremain)
			{
				break;//写入结束了
			}
			else
			//写入未结束
			{
				secpos++;
				//扇区地址增1
				secoff=0;
				//偏移位置为0
				
				pBuffer+=secremain;
				//指针偏移
				WriteAddr+=secremain;
				//写地址偏移
				NumByteToWrite-=secremain;
				//字节数递减
				if(NumByteToWrite>4096)
					secremain = 4096;
					//下一个扇区还是写不完
				else
					secremain=NumByteToWrite;
					//下一个扇区可以写完
			}
		}
}
#endif
#endif
