#include "ddl.h"
#include "uart.h"
#include "bt.h"
#include "lpm.h"
#include "gpio.h"

#define     T1_PORT                 (3)
#define     T1_PIN                  (3)

uint8_t u8RxData[3]={0x55,0x52,0x59};
uint8_t u8RxFlg=0;
uint8_t CheckFlg=0;
/************************接收中断回调函数************************/
void RxIntCallback(void)
{
    u8RxData[1]=M0P_UART0->SBUF;
	u8RxFlg = 1;
}
/************************错误中断回调函数************************/
void ErrIntCallback(void)
{
  
}

int32_t main(void)
{  
    uint16_t timer=0;
    uint32_t pclk=0;
	/************************结构体重命名************************/
    stc_uart_config_t  stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
    stc_uart_multimode_t stcMulti;
    stc_uart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;
	/************************结构体************************/
    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);
	/************************.************************/
//    Gpio_InitIO(T1_PORT,T1_PIN,GpioDirIn); 
//    Gpio_InitIO(0,3,GpioDirOut);
//    Gpio_SetIO(0,3,1);
    /***********************TXD、RXD引脚定义************************/
    Gpio_InitIOExt(1,4,GpioDirOut,TRUE,FALSE,FALSE,FALSE);   
    Gpio_InitIOExt(1,5,GpioDirOut,TRUE,FALSE,FALSE,FALSE); 
	
    Gpio_SetFunc_UART0TX_P14();
    Gpio_SetFunc_UART0RX_P15();
	/*************************外部时钟使能**************************/
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);	//模式0/2可以不使能
    Clk_SetPeripheralGate(ClkPeripheralUart0,TRUE);	//
	/************************配置中断相关内容************************/
    stcUartIrqCb.pfnRxIrqCb = RxIntCallback;		//接收中断回调函数（写了，有）
    stcUartIrqCb.pfnTxIrqCb = NULL;					//发送中断回调函数（没写）
    stcUartIrqCb.pfnRxErrIrqCb = ErrIntCallback;	//错误中断回调函数（写了，没有）
    stcConfig.pstcIrqCb = &stcUartIrqCb;			//配置中断服务函数
    stcConfig.bTouchNvic = TRUE;					//允许中断
	/************************配置通讯相关内容************************/
    stcConfig.enRunMode = UartMode3;		//测试项，更改此处来转换4种模式测试
    stcMulti.enMulti_mode = UartNormal;		//测试项，更改此处来转换多主机模式，mode2/3才有多主机模式
    stcConfig.pstcMultiMode = &stcMulti;	//配置正常或多机工作模式
	/**************************配置波特率**************************/
    stcBaud.bDbaud = 0u;							//双倍波特率功能
    stcBaud.u32Baud = 9600u;						//更新波特率位置
    stcBaud.u8Mode = UartMode3; 					//计算波特率需要模式参数
    pclk = Clk_GetPClkFreq();						//获得PCLK
    timer=Uart_SetBaudRate(UARTCH0,pclk,&stcBaud);	//计算波特率所需TIMER值

    stcBtConfig.enMD = BtMode2;		//自动重装载16位计数器/定时器
    stcBtConfig.enCT = BtTimer;		//定时模式
    Bt_Init(TIM0, &stcBtConfig);	//调用basetimer1设置函数产生波特率
    Bt_ARRSet(TIM0,timer);			//配置重载值ARR
    Bt_Cnt16Set(TIM0,timer);		//配置计数值CNT
    Bt_Run(TIM0);					//启动定时
	/***********************串口及接收中断配置***********************/
    Uart_Init(UARTCH0, &stcConfig);		//串口0初始化
    Uart_EnableIrq(UARTCH0,UartRxIrq);	//允许串口0接收中断
    Uart_ClrStatus(UARTCH0,UartRxFull);	//清串口0接收中断标志
    Uart_EnableFunc(UARTCH0,UartRx);	//串口0接收中断使能
	
	Uart_SetTb8(UARTCH0,Even,u8RxData[0]);
	Uart_SendData(UARTCH0,u8RxData[0]);
	
    while(1)
	{
		CheckFlg = 0;
		if(u8RxFlg)
		{
			u8RxFlg = 0;
			if(Uart_CheckEvenOrOdd(UARTCH0,Even,u8RxData[1])!=Ok)
			{
				CheckFlg = 1;//奇偶校验出错
				Uart_SetTb8(UARTCH0,Even,u8RxData[1]);
				Uart_SendData(UARTCH0,u8RxData[1]);
			}
			else
			{			
				Uart_SetTb8(UARTCH0,Even,u8RxData[2]);
				Uart_SendData(UARTCH0,u8RxData[2]);
			}
		}
	}
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


