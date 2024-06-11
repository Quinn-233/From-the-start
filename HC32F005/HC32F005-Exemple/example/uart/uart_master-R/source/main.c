#include "uart.h"
#include "bt.h"
#include "gpio.h"

#define     T1_PORT                 (3)
#define     T1_PIN                  (3)

uint8_t u8TxData[10] = {0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00};//"HUADA MCU!";
int32_t main(void)
{  
    uint16_t timer,i=0;
    uint32_t pclk;
    stc_clk_config_t stcCfg;			//时钟初始化函数 重命名
    stc_uart_config_t  stcConfig;		//
    stc_uart_irq_cb_t stcUartIrqCb;		//
    stc_uart_multimode_t stcMulti;		//
    stc_uart_baud_config_t stcBaud;		//
    stc_bt_config_t stcBtConfig;

    en_uart_mmdorck_t enTb8;
	//清空
    DDL_ZERO_STRUCT(stcConfig);
    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);
    //时钟部分配置
    stcCfg.enClkSrc = ClkXTH;
    stcCfg.enHClkDiv = ClkDiv1;
    stcCfg.enPClkDiv = ClkDiv1;
    Clk_Init(&stcCfg);
    
    Gpio_InitIOExt(3,5,GpioDirOut,TRUE,FALSE,FALSE,FALSE); 
    Gpio_InitIOExt(3,6,GpioDirOut,TRUE,FALSE,FALSE,FALSE);
    
    //通道端口配置
    Gpio_SetFunc_UART1TX_P35();
    Gpio_SetFunc_UART1RX_P36();

    //外设时钟使能
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);
    Clk_SetPeripheralGate(ClkPeripheralUart1,TRUE);



    stcUartIrqCb.pfnRxIrqCb = NULL;
    stcUartIrqCb.pfnTxIrqCb = NULL;
    stcUartIrqCb.pfnRxErrIrqCb = NULL;
    stcConfig.pstcIrqCb = &stcUartIrqCb;
    stcConfig.bTouchNvic = FALSE;
#if 1	//多主机模式测试
    stcConfig.enRunMode = UartMode3;
    stcMulti.enMulti_mode = UartMulti;
    enTb8 = Addr;
    Uart_SetMMDOrCk(UARTCH1,enTb8);
    stcConfig.pstcMultiMode = &stcMulti;
    
    stcBaud.bDbaud = 0u;
    stcBaud.u32Baud = 2400u;
    stcBaud.u8Mode = UartMode3; //计算波特率需要模式参数
    pclk = Clk_GetPClkFreq();
    timer=Uart_SetBaudRate(UARTCH1,pclk,&stcBaud);
  
    stcBtConfig.enMD = BtMode2;
    stcBtConfig.enCT = BtTimer;
    Bt_Init(TIM1, &stcBtConfig);//调用basetimer1设置函数产生波特率
    Bt_ARRSet(TIM1,timer);
    Bt_Cnt16Set(TIM1,timer);
    Bt_Run(TIM1);
#endif
    Uart_Init(UARTCH1, &stcConfig);
    Uart_ClrStatus(UARTCH1,UartTxEmpty);//清所有中断请求
    Uart_EnableFunc(UARTCH1,UartRx);   
#if 1
    Uart_SendData(UARTCH1,0xC0);
    enTb8 = Data;
    Uart_SetMMDOrCk(UARTCH1,enTb8);
    Uart_ClrStatus(UARTCH1,UartTxEmpty);        
    while(i<10)
    {
        Uart_SendData(UARTCH1,u8TxData[i]);
        i++;
    }
#endif
    while(1);
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


