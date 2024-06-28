#include "ddl.h"
#include "flash.h"

//#define RUN_IN_RAM 1    //need to config *.icf

static volatile uint32_t u32FlashTestFlag   = 0;

/*****************************
 * FLASH 中断服务函数
 *****************************/
 void FlashInt(void)
 {
    if (TRUE == Flash_GetIntFlag(flash_int0))
    {
        Flash_ClearIntFlag(flash_int0);
        u32FlashTestFlag |= 0x01;
        Flash_DisableIrq(flash_int0);
    }
    if (TRUE == Flash_GetIntFlag(flash_int1))
    {
        Flash_ClearIntFlag(flash_int1);
        u32FlashTestFlag |= 0x02;
        Flash_DisableIrq(flash_int1);
    }
      
 }
   
/******************************
 * FLASH 编程测试
 ******************************/
en_result_t FlashWriteTest(void)
{
    en_result_t       enResult = Error;
    uint32_t          u32Addr  = 0x3ff0;
    uint8_t           u8Data   = 0x11;
    uint16_t          u16Data  = 0x2222;
    uint32_t          u32Data  = 0x33333333;

    Flash_Init(FlashInt, 0);
    
    Flash_SectorErase(u32Addr);
    
    enResult = Flash_WriteByte(u32Addr, u8Data);
    if (Ok == enResult)
    {
        if(*((volatile uint8_t*)u32Addr) == u8Data)
        {
            enResult = Ok;
        }
        else
        {
            return enResult;
        }
    }
    else
    {
        enResult = Error;
        return enResult;
    }  

    u32Addr += 2;
    enResult = Flash_WriteHalfWord(u32Addr, u16Data);
    if (Ok == enResult)
    {
        if(*((volatile uint16_t*)u32Addr) == u16Data)
        {
            enResult = Ok;
        }
        else
        {
            return enResult;
        }
    }
    else
    {
        return enResult;
    }

    u32Addr += 6;
    enResult = Flash_WriteWord(u32Addr, u32Data);
    if (Ok == enResult)
    {
        if(*((volatile uint32_t*)u32Addr) == u32Data)
        {
            enResult = Ok;
        }
        else
        {
            return enResult;
        }
    }
    else
    {
        return enResult;
    }  
    
    return enResult;
}

/******************************
 * main主函数
 ******************************/
int32_t main(void)
{
    volatile uint8_t u8TestFlag = 0;
    
    if(Ok != FlashWriteTest())
    {
        u8TestFlag |= 0x01;
    }   
    
    while (1);
}

