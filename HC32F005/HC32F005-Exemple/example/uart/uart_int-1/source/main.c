#include "ddl.h"
#include "uart.h"
#include "bt.h"
#include "lpm.h"
#include "gpio.h"

uint16_t timer=0;
uint32_t pclk=0;

uint8_t u8RxData[10];
uint8_t u8RxData1[2]={0x55,0x00};
uint8_t u8String[]="RusHHH!\n";
uint8_t u8Buff[100]="";

uint8_t step0[]="0\n";
uint8_t step1[]="1\n";
uint8_t step2[]="2\n";
uint8_t step3[]="3\n";

uint8_t i = 0;
uint8_t u8RxCnt=0; 
uint8_t u8RxFlg=0;
uint8_t CheckFlg=0;
uint8_t num=0;

/****************** 发送一个字节 **********************/
void Uart_SendByte(uint8_t u8Idx,uint8_t u8Data)
{
	Uart_SetTb8(u8Idx,Even,u8Data);	
	Uart_SendData(u8Idx,u8Data);				//库函数的发送一个字节
	while(Uart_GetStatus(u8Idx, UartTxEmpty) == TRUE);	//获取TXE的状态，一直等于FLASE=0，表示TX buffer非空
}
/***************** 发送一个字符串 **********************/
void Uart_SendString(uint8_t u8Idx,uint8_t *str)
{
	uint8_t k=0;
	do
	{
		Uart_SendByte(u8Idx,*(str+k));	//循环发送一个字节一个字节的发
		k++;
	}while(*(str+k)!='\0');//直至遇到字符串结束符 '\0'
}
/************ 接收中断回调函数 ************/
void RxIntCallback(void)
{
//	u8RxData[1]=M0P_UART1->SBUF;
//	u8Buff[u8RxCnt] = Uart_ReceiveData(UARTCH1);
	u8Buff[u8RxCnt] = M0P_UART1->SBUF;
	if(Uart_CheckEvenOrOdd(UARTCH1,Even,u8RxData[u8RxCnt])!=Ok)
	{
		CheckFlg = 1;//奇偶校验出错
	}
	u8RxCnt++;
//	Uart_SendByte(UARTCH1,Uart_ReceiveData(UARTCH1));		//接收回显	
	if(u8Buff[u8RxCnt-1] == '#')
	{
		u8Buff[u8RxCnt-1] = '\0';
		u8RxCnt = 0;
		u8RxFlg = 1;
//		Uart_SendString(UARTCH1,u8Buff);
	}	
}
/************ 错误中断回调函数 ************/
void ErrIntCallback(void)
{ 
	Uart_SendString(UARTCH1,"Error!\n");
}
/************* UART初始化函数 *************/
void UART_Config(uint16_t uart_baund)
{ 
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
    /***********************TXD、RXD引脚定义************************/
	Gpio_InitIO(2, 3, GpioDirOut);
	Gpio_InitIO(2, 4, GpioDirOut);
	
    Gpio_SetFunc_UART1TX_P23();
    Gpio_SetFunc_UART1RX_P24();
	/*************************外部时钟使能**************************/
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);	//模式0/2可以不使能
    Clk_SetPeripheralGate(ClkPeripheralUart1,TRUE);	//
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
    stcBaud.u32Baud = uart_baund;					//设置目标波特率
    stcBaud.u8Mode = UartMode3; 					//计算波特率需要模式参数
    pclk = Clk_GetPClkFreq();						//获得PCLK（4MHz）
    timer=Uart_SetBaudRate(UARTCH1,pclk,&stcBaud);	//计算波特率所需TIMER值

    stcBtConfig.enMD = BtMode2;		//自动重装载16位计数器/定时器
    stcBtConfig.enCT = BtTimer;		//定时模式
    Bt_Init(TIM1, &stcBtConfig);	//调用basetimer1设置函数产生波特率
    Bt_ARRSet(TIM1,timer);			//配置重载值ARR
    Bt_Cnt16Set(TIM1,timer);		//配置计数值CNT
    Bt_Run(TIM1);					//启动定时
	/***********************串口及接收中断配置***********************/
    Uart_Init(UARTCH1, &stcConfig);		//串口1初始化
    Uart_EnableIrq(UARTCH1,UartRxIrq);	//允许串口1接收中断
    Uart_ClrStatus(UARTCH1,UartRxFull);	//清串口1接收中断标志
    Uart_EnableFunc(UARTCH1,UartRx);	//串口1接收中断使能
}

int32_t main(void)
{  
    UART_Config(2400u);
	
    while(1)
	{
		if(u8RxFlg)
		{
			u8RxFlg = 0;
//			for(i = 0; i < sizeof(u8RxData); i++)
//			{
//				if(Uart_CheckEvenOrOdd(UARTCH1,Even,u8RxData[i])!=Ok)
//				{
//					CheckFlg = 1;//奇偶校验出错
//					break;
//				}
//			}	
			if(CheckFlg != 1)
			{
				Uart_SendString(UARTCH1,u8Buff);
//				Uart_SendString(UARTCH0,step3);
//				Uart_SetTb8(UARTCH1,Even,u8RxData[0]);
//				Uart_SendData(UARTCH1,u8RxData[0]);				
//				Uart_SetTb8(UARTCH1,Even,u8RxData[1]);
//				Uart_SendData(UARTCH1,u8RxData[1]);
			}
		}

		sprintf(u8Buff, "CheckFlg：%d\n", CheckFlg);
		Uart_SendString(UARTCH1,u8Buff);
		delay1ms(500);
		
//		Uart_SetTb8(UARTCH1,Even,u8RxData[0]);
//		Uart_SendData(UARTCH1,u8RxData[0]);
//		delay1ms(500);
		
//		Uart_SendByte(UARTCH1,u8String[0]);
//		delay1ms(500);
		
//		sprintf(u8Buff, "CLK：%d\n", pclk);
//		Uart_SendString(UARTCH1,u8Buff);
//		delay1ms(500);
		
//		Uart_SendString(UARTCH1,u8String);
//		delay1ms(500);
	}
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


