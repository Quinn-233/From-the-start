#include "pca.h"
#include "lpm.h"
#include "gpio.h"

static volatile uint32_t u32PcaTestFlag = 0;
static volatile uint16_t u16CcapData[8] = {0};

void PcaInt(void)
{
    if (TRUE == Pca_GetCntIntFlag())
    {
        Pca_ClearCntIntFlag();
        u32PcaTestFlag |= 0x20;
    }
    if (TRUE == Pca_GetIntFlag(Module0))
    {
        Pca_ClearIntFlag(Module0);
        u32PcaTestFlag |= 0x01;
    }
    if (TRUE == Pca_GetIntFlag(Module1))
    {
        Pca_ClearIntFlag(Module1);
        u32PcaTestFlag |= 0x02;
    }
    if (TRUE == Pca_GetIntFlag(Module2))
    {
        Pca_ClearIntFlag(Module2);
        u32PcaTestFlag |= 0x04;
    }
    if (TRUE == Pca_GetIntFlag(Module3))
    {
        Pca_ClearIntFlag(Module3);
        u32PcaTestFlag |= 0x08;
    }
    if (TRUE == Pca_GetIntFlag(Module4))
    {
        Pca_ClearIntFlag(Module4);
        u32PcaTestFlag |= 0x10;
    }
        
}

/*******************************************************************************
 * PCA PWM功能测试
 ******************************************************************************/
en_result_t PcaPwmTest(void)
{
    stc_pca_config_t stcConfig;
    stc_pca_capmodconfig_t stcModConfig;
    en_result_t      enResult = Ok;
    uint8_t          u8CcaplData = 0x20;
    uint8_t          u8CcaphData = 0x40;
    uint32_t         u32Cnt;
    
    Gpio_SetFunc_PCA_CH0_P34(0);	//配置成PCA输出
    /******************PCA 配置结构体定义*********************/
    stcConfig.enCIDL = IdleGoon; 		//休眠模式下启/停控制
    stcConfig.enWDTE = PCAWDTDisable;	//WDT功能控制
    stcConfig.enCPS  = PCAPCLKDiv32; 	//时钟分频及时钟源选择功能
    stcConfig.pfnPcaCb = PcaInt;		//中断服务回调函数
    /**************PCA 捕获模式配置结构体定义*****************/
    stcModConfig.enECOM = ECOMEnable;	//比较器功能控制：允许
    stcModConfig.enCAPP = CAPPDisable;	//正沿捕获控制：禁止
    stcModConfig.enCAPN = CAPNDisable;	//负沿捕获控制：禁止
    stcModConfig.enMAT  = MATDisable;	//匹配控制：禁止
    stcModConfig.enTOG  = TOGDisable;	//翻转控制：禁止
    stcModConfig.enPWM  = PCAPWMEnable;	//脉宽调制控制：允许
    
    
    if (Ok != Pca_Init(&stcConfig))
    {
        enResult = Error;
    }
    if (Ok != Pca_CapModConfig(Module0, &stcModConfig))
    {
        enResult = Error;
    }
    
    Pca_CapDataLSet(Module0, u8CcaplData);
    Pca_CapDataHSet(Module0, u8CcaphData);
    Pca_Run();

    //PWM波形输出……
    u32Cnt = 0x80000;
    while(u32Cnt--)
    {
        ;
    }
    
    return enResult;
}

/**
 ******************************************************************************
 ** \brief  Main function of project
 **
 ** \return uint32_t return value, if needed
 **
 ** This sample
 **
 ******************************************************************************/

int32_t main(void)
{
    volatile uint8_t u8TestFlag = 0;     

    //PCA、GPIO外设时钟开启
    Clk_SetPeripheralGate(ClkPeripheralPca, TRUE);
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
        
    if(Ok != PcaPwmTest())
    {
        u8TestFlag |= 0x10;
    } 
    
    while (1);
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


