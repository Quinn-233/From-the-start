#include "ddl.h"
#include "uart.h"
#include "bt.h"
#include "lpm.h"
#include "gpio.h"

#define     T1_PORT                 (3)
#define     T1_PIN                  (3)

uint8_t u8RxData[2]={0x55,0x00};
uint8_t u8RxFlg=0;
uint8_t CheckFlg=0;
void RxIntCallback(void)
{
    u8RxData[1]=M0P_UART0->SBUF;
	u8RxFlg = 1;
}
void ErrIntCallback(void)
{
  
}
int32_t main(void)
{  
	
    uint16_t timer=0;
    uint32_t pclk=0;
	
    stc_uart_config_t  stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
    stc_uart_multimode_t stcMulti;
    stc_uart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;
    

    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);

    
    Gpio_InitIO(T1_PORT,T1_PIN,GpioDirIn); 
    Gpio_InitIO(0,3,GpioDirOut);
    Gpio_SetIO(0,3,1);
    
    Gpio_InitIOExt(1,4,GpioDirOut,TRUE,FALSE,FALSE,FALSE);   
    Gpio_InitIOExt(1,5,GpioDirOut,TRUE,FALSE,FALSE,FALSE); 
    
    //通道端口配置
    Gpio_SetFunc_UART0TX_P14();
    Gpio_SetFunc_UART0RX_P15();

    //外设时钟使能
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);//模式0/2可以不使能
    Clk_SetPeripheralGate(ClkPeripheralUart0,TRUE);

    stcUartIrqCb.pfnRxIrqCb = RxIntCallback;
    stcUartIrqCb.pfnTxIrqCb = NULL;
    stcUartIrqCb.pfnRxErrIrqCb = ErrIntCallback;
    stcConfig.pstcIrqCb = &stcUartIrqCb;
    stcConfig.bTouchNvic = TRUE;
  

    stcConfig.enRunMode = UartMode3;//测试项，更改此处来转换4种模式测试
   

    stcMulti.enMulti_mode = UartNormal;//测试项，更改此处来转换多主机模式，mode2/3才有多主机模式

    stcConfig.pstcMultiMode = &stcMulti;

    stcBaud.bDbaud = 0u;//双倍波特率功能
    stcBaud.u32Baud = 2400u;//更新波特率位置
    stcBaud.u8Mode = UartMode3; //计算波特率需要模式参数
    pclk = Clk_GetPClkFreq();
    timer=Uart_SetBaudRate(UARTCH0,pclk,&stcBaud);

    stcBtConfig.enMD = BtMode2;
    stcBtConfig.enCT = BtTimer;
    Bt_Init(TIM0, &stcBtConfig);//调用basetimer1设置函数产生波特率
    Bt_ARRSet(TIM0,timer);
    Bt_Cnt16Set(TIM0,timer);
    Bt_Run(TIM0);

    Uart_Init(UARTCH0, &stcConfig);
    Uart_EnableIrq(UARTCH0,UartRxIrq);
    Uart_ClrStatus(UARTCH0,UartRxFull);
    Uart_EnableFunc(UARTCH0,UartRx);
	
    while(1)
	{
#if 0
		CheckFlg = 0;
		if(u8RxFlg)
		{
			u8RxFlg = 0;
			if(Uart_CheckEvenOrOdd(UARTCH0,Even,u8RxData[1])!=Ok)
			{
				CheckFlg = 1;//奇偶校验出错
			}
			else
			{
				Uart_SetTb8(UARTCH0,Even,u8RxData[0]);
				Uart_SendData(UARTCH0,u8RxData[0]);				
				Uart_SetTb8(UARTCH0,Even,u8RxData[1]);
				Uart_SendData(UARTCH0,u8RxData[1]);
			}
		}
#endif

#if 0
		Gpio_InitIO(1,5,GpioDirOut);
		Gpio_SetIO(1,5,1);
#endif
	}
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


