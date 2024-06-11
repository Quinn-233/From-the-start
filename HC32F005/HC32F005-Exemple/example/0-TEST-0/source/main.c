#include "ddl.h"
#include "uart.h"
#include "bt.h"
#include "lpm.h"
#include "gpio.h"

uint16_t timer=0;
uint32_t pclk=0;

uint8_t u8RxData[10];
uint8_t u8Data[]="RusHHH!\n";
uint8_t u8Buff[10]="";

uint8_t step0[]="0\n";
uint8_t step1[]="1\n";
uint8_t step2[]="2\n";
uint8_t step3[]="3\n";


/************************************************************
* UART 
*************************************************************/
#if 1
/************** 发送一个字节 *****************/
void Uart_SendByte(uint8_t u8Idx,uint8_t u8Data)
{
	Uart_SetTb8(u8Idx,Even,u8Data);	
	Uart_SendData(u8Idx,u8Data);				
	while(Uart_GetStatus(u8Idx, UartTxEmpty) == TRUE);
}
/************* 发送一个字符串*****************/
void Uart_SendString(uint8_t u8Idx,uint8_t *str)
{
	uint8_t k=0;
	do
	{
		Uart_SendByte(u8Idx,*(str+k));	
		k++;
	}while(*(str+k)!='\0');	//直至遇到字符串结束符 '\0'
}
/************ 接收中断回调函数 ***************/
void RxIntCallback(void)
{
	Uart_SetTb8(UARTCH0,Even,step1[0]);
	Uart_SendData(UARTCH0,step1[0]);
	if(TRUE == Uart_GetStatus(UARTCH0,UartRxFull))
	{
		Uart_ClrStatus(UARTCH0,UartRxFull);
		//u8RxData[1]=M0P_UART0->SBUF;
		Uart_SetTb8(UARTCH0,Even,step2[0]);
		Uart_SendData(UARTCH0,step2[0]);
	}
	
//	//先存储在数组中
//	Uart_ClrStatus(UARTCH0, UartRxFull);        //清掉接收中断标志位 置0，为下次继续接收做准备
//	u8RxData[u8RxCnt] = Uart_ReceiveData(UARTCH0);   	//接收数据字节，把缓存寄存器的值存储在数组中
//	Uart_SetTb8(UARTCH0,Even,u8RxData[u8RxCnt]);	//
//	Uart_SendData(UARTCH0,u8RxData[u8RxCnt]);		//接收回显
//	u8RxCnt++;			//依次存储
//	
//	//u8RxCnt=0;//置0，方便下次重复以上操作
//	if(Uart_GetStatus(UARTCH0,UartRxFull))  // TC 发送完成为真，发送完成一个字节
//	{
//		Uart_ClrStatus(UARTCH0,UartRxFull); //清掉发送中断标志位。置0
//		u8RxCnt=0;//置0，方便下次重复以上操作
//	}
}
/************ 错误中断回调函数 ***************/
void ErrIntCallback(void)
{ 
}
/*********** UART初始化配置函数 **************/
void Uart_config(void)
{ 
	/*------- 结构体重命名 --------*/
    stc_uart_config_t  stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
    stc_uart_multimode_t stcMulti;
    stc_uart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;
	/*-------- 结构体清零 ---------*/
    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);
	/*------- 未知 --------*/
//    Gpio_InitIO(T1_PORT,T1_PIN,GpioDirIn); 
//    Gpio_InitIO(0,3,GpioDirOut);
//    Gpio_SetIO(0,3,1);
	/*----- TXD、RXD引脚配置 -----*/
    //Gpio_InitIOExt(1,4,GpioDirOut,TRUE,FALSE,FALSE,FALSE);   
    //Gpio_InitIOExt(1,5,GpioDirIn,TRUE,FALSE,FALSE,FALSE); 
	Gpio_InitIO(1,4,GpioDirOut);   
    Gpio_InitIO(1,5,GpioDirIn);
	
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
	Uart_Init(UARTCH0, &stcConfig);			//串口0初始化
	/**************************配置波特率**************************/
    stcBaud.bDbaud = 0u;							//双倍波特率功能
    stcBaud.u32Baud = 9600u;						//设置目标波特率
    stcBaud.u8Mode = UartMode3; 					//计算波特率需要模式参数
    pclk = Clk_GetPClkFreq();						//获得PCLK（4MHz）
    timer=Uart_SetBaudRate(UARTCH0,pclk,&stcBaud);	//计算波特率所需TIMER值

    stcBtConfig.enMD = BtMode2;		//自动重装载16位计数器/定时器
    stcBtConfig.enCT = BtTimer;		//定时模式
    Bt_Init(TIM0, &stcBtConfig);	//调用basetimer1设置函数产生波特率
    Bt_ARRSet(TIM0,timer);			//配置重载值ARR
    Bt_Cnt16Set(TIM0,timer);		//配置计数值CNT
    Bt_Run(TIM0);					//启动定时
	/*************************接收中断配置*************************/
    Uart_EnableIrq(UARTCH0,UartRxIrq);	//允许串口0接收中断
    Uart_ClrStatus(UARTCH0,UartRxFull);	//清串口0接收中断标志
    Uart_EnableFunc(UARTCH0,UartRx);	//串口0接收中断使能
}
#endif

/************************************************************
* GPIO 
*************************************************************/
#if 1
	
#endif


int32_t main(void)
{  
	Uart_config();
    while(1)
	{
//		Uart_SetTb8(UARTCH0,Even,u8RxData[0]);
//		Uart_SendData(UARTCH0,step0[0]);
//		delay1ms(500);
		
		
		
		sprintf(u8Buff, "%u\n", pclk);
		Uart_SendString(UARTCH0,u8Buff);
		delay1ms(500);
	}
}
