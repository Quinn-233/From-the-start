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
        //(u32Cnt++)%2;
		//if((u32Cnt++) == 2) u32Cnt=0;
		Gpio_SetIO(3,2,0);
    }
}

/******************BT定时功能测试（重载模式）**********************/
void BtTimerTest(void)
{
    stc_bt_config_t   stcConfig;
    uint16_t          u16ArrData = 0x10000-31250;		//
    uint16_t          u16InitCntData = 0x10000-31250;	//
    
	Clk_SetPeripheralGate(ClkPeripheralBt, TRUE);
	
    stcConfig.pfnTim0Cb = Bt1Int;
    //门控使能IO
	Gpio_SetFunc_TIM0_GATE_P35();
//    Gpio_SetFunc_TIM1_GATE_P25();
//    Gpio_SetFunc_TIM2_GATE_P02();
	
	//Bt初始化
    stcConfig.enGateP = BtPositive;		//门控极性：高电平有效
    stcConfig.enGate  = BtGateEnable;	//门控使能：有效
    stcConfig.enPRS   = BtPCLKDiv64;	//预分频系数：64分频
    stcConfig.enTog   = BtTogDisable;	//反转输出使能：无效
    stcConfig.enCT    = BtTimer;		//定时/计数功能选择：定时
    stcConfig.enMD    = BtMode2;		//计数模式配置：自动重装载16位计数器/定时器
    Bt_Init(TIM0, &stcConfig);
    
    //TIM0中断使能
    Bt_ClearIntFlag(TIM0);
    Bt_EnableIrq(TIM0);
    EnableNvic(TIM0_IRQn, 3, TRUE);
    
    //设置重载值和计数初值，启动计数
    Bt_ARRSet(TIM0, u16ArrData);		//
    Bt_Cnt16Set(TIM0, u16InitCntData);	//16位初值设置
    Bt_Run(TIM0);   
}

int32_t main(void)
{    
    //CLK INIT
    stc_clk_config_t stcClkCfg;
    stcClkCfg.enClkSrc  = ClkRCH;
    stcClkCfg.enHClkDiv = ClkDiv1;
    stcClkCfg.enPClkDiv = ClkDiv1;
    Clk_Init(&stcClkCfg);
    
    //打开GPIO、BT外设时钟
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
    
	Gpio_InitIO(3,2,GpioDirOut);
	Gpio_SetIO(3,2,1);
	
#if 1
	Gpio_InitIO(3,5,GpioDirOut);
	Gpio_SetIO(3,5,1);
	
	BtTimerTest();
	
    while(1)
    {
//		if (0 == u32Cnt)
//        {
//			Gpio_SetIO(3,2,1);
//		}else if(1 == u32Cnt)
//		{
//			Gpio_SetIO(3,2,0);
//		}
		
	}
#endif
	
#if 0
	while(1)
    {
		Gpio_SetIO(3,2,1);
		delay1ms(1000);
		Gpio_SetIO(3,2,0);
		delay1ms(1000);
	}
#endif
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


