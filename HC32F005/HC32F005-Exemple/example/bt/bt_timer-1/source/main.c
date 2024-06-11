#include "ddl.h"
#include "bt.h"
#include "lpm.h"
#include "gpio.h"

static volatile uint32_t u32BtTestFlag = 0;
static volatile uint32_t u32Cnt = 0;

/******************BT1中断服务函数********************/
void Bt1Int(void)
{
    if (TRUE == Bt_GetIntFlag(TIM0))
    {
        Bt_ClearIntFlag(TIM0);
        u32BtTestFlag = 0x02;
    }
}

/******************BT定时功能测试（重载模式）**********************/
void BtTimerTest(void)
{
    stc_bt_config_t   stcConfig;
    en_result_t       enResult = Error;
    uint16_t          u16ArrData = 0x8000;		//
    uint16_t          u16InitCntData = 0xC000;	//
    
    stcConfig.pfnTim0Cb = Bt1Int;
    //门控使能IO
	Gpio_SetFunc_TIM0_GATE_P35();
//    Gpio_SetFunc_TIM1_GATE_P25();
//    Gpio_SetFunc_TIM2_GATE_P02();
    stcConfig.enGateP = BtPositive;		//门控极性：高电平有效
    stcConfig.enGate  = BtGateEnable;	//门控使能：有效
    stcConfig.enPRS   = BtPCLKDiv16;	//预分频系数：16分频
    stcConfig.enTog   = BtTogDisable;	//反转输出使能：无效
    stcConfig.enCT    = BtTimer;		//定时/计数功能选择：定时
    stcConfig.enMD    = BtMode2;		//计数模式配置：自动重装载16位计数器/定时器
    //Bt初始化
    if (Ok != Bt_Init(TIM0, &stcConfig))
    {
        enResult = Error;
    }
    
    //TIM0中断使能
    Bt_ClearIntFlag(TIM0);
    Bt_EnableIrq(TIM0);
    EnableNvic(TIM0_IRQn, 3, TRUE);
    
    //设置重载值和计数值，启动计数
    Bt_ARRSet(TIM0, u16ArrData);
    Bt_Cnt16Set(TIM0, u16InitCntData);
    Bt_Run(TIM0);
    
	Gpio_InitIO(1,4,GpioDirOut);
	
    //此处进入中断……
    while(1)
    {
        //判断是否第二次进入中断
        if (0x02 == u32BtTestFlag)
        {
            u32BtTestFlag = u32BtTestFlag & (~0x02);
            if (0 == u32Cnt)
            {
				Gpio_SetIO(1,4,0);
                //Bt_Stop(TIM0);
                enResult = Ok;
                //break;
            }else if(1 == u32Cnt)
			{
				Gpio_SetIO(1,4,1);
                //Bt_Stop(TIM0);
                enResult = Ok;
                //break;
			}
            u32Cnt++;
        }
    }    
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
    
    //CLK INIT
    stc_clk_config_t stcClkCfg;
    stcClkCfg.enClkSrc  = ClkRCH;
    stcClkCfg.enHClkDiv = ClkDiv1;
    stcClkCfg.enPClkDiv = ClkDiv1;

    Clk_Init(&stcClkCfg);
    
    //打开GPIO、BT外设时钟
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
    Clk_SetPeripheralGate(ClkPeripheralBt, TRUE);
       
    BtTimerTest();
    
     
    //while (1);
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


