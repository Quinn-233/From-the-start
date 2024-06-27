#include "gpio.h"
/*
LED1 -> P32		KEY1 -> P02(BUTTON2)
LED2 -> P03		KEY2 -> P01(BUTTON1)
LED3 -> P34
LED4 -> P35

TXD -> P14
RXD -> P15
*/


int32_t main(void)
{
#if 1
    //GPIO输入
    //配置P33为输入，P33外接SW2按键
    Gpio_InitIO(3, 4, GpioDirIn);
    //如果按键SW2按下，为低电平，则跳出循环继续执行程序
    while (TRUE == Gpio_GetIO(3,4));   
   
    //GPIO输出
    //初始化外部GPIO P03为输出、上拉、开漏，P03端口外接LED3
    Gpio_InitIO(3, 2, GpioDirOut);
	Gpio_SetIO(3, 2, FALSE);
    //设置GPIO值（翻转）
    while (1)
    {
        //输出高电平，LED3亮
        Gpio_SetIO(3, 2, TRUE);
        delay1ms(1000);

        //输出低电平，LED3灭
        Gpio_SetIO(3, 2, FALSE);
        delay1ms(1000);
    }
#endif

#if 0
	//Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
	Gpio_InitIOExt(3, 2, GpioDirOut, TRUE, FALSE, TRUE, FALSE);
	//Gpio_InitIO(3, 2, GpioDirOut);
	Gpio_SetIO(3, 2, TRUE);
#endif
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


