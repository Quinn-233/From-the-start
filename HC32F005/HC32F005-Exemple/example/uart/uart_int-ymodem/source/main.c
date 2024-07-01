#include "ddl.h"
#include "uart.h"
#include "bt.h"
#include "lpm.h"
#include "gpio.h"
#include "xyzmodem.h"

uint16_t timer=0;
uint32_t pclk=0;

uint8_t kan[150];
uint8_t ymodemBuff[1024];
uint8_t u8RxData[100];
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
uint8_t rx_flag=0;

/*****************************************
** UART
******************************************/
#if 1
/****** 接收中断回调函数 ******/
void RxIntCallback(void)
{
	if(!rx_flag){
		
		u8RxData[u8RxCnt] = Uart_ReceiveData(UARTCH1);
		u8RxData[u8RxCnt] = M0P_UART1->SBUF;

		u8RxCnt++;
//		Uart_SendByte(UARTCH1,Uart_ReceiveData(UARTCH1));		//接收回显	
		if(u8RxData[u8RxCnt-1] == '\n')
		{
			rx_flag = 1;
			u8RxData[u8RxCnt] = '\0';
			u8RxCnt = 0;
			u8RxFlg = 1;
//			Uart_SendString(UARTCH1,u8RxData);
		}	
	}
	
	if(ymodem_start_flag){ // 开启 ymodem 功能 所有数据都进入队列 在队列里进行处理
		//enQueue(M0P_UART1->SBUF);
		kan[u8RxCnt++]=M0P_UART1->SBUF;
//		Uart_SendByte(UARTCH1,M0P_UART1->SBUF);
	}
//	if (!ymodem_start_flag) // 未开启 ymodem ，进行判断是否开启
//	{
//		if(u8RxFlg == 1){
//			u8RxFlg = 0;
//			if((!strcmp(test_command,(char*)u8RxData)) && u8RxCnt<=20){
//				ymodem_start_flag = 1;
//				Uart_SendString(UARTCH1,(uint8_t*)"ok,please select image binary file\r\n");
//			} else if ((!strcmp(del_custom_flash_command,(char*)u8RxData)) && u8RxCnt<=20) {
//				ymodem_start_flag = 2;
//				Uart_SendString(UARTCH1,(uint8_t*)"ok,Deleting\r\n");
//			} else if ((!strcmp(mask_command,(char*)u8RxData)) && u8RxCnt<=20) {
//				ymodem_start_flag = 3;
//				Uart_SendString(UARTCH1,(uint8_t*)"ok,please select a mask file\r\n");
//			} else{
//				Uart_SendString(UARTCH1,(uint8_t*)"unrecognized command\r\n");
//			}
//		}
//	}else { // 开启 ymodem 功能 所有数据都进入队列 在队列里进行处理
//		enQueue(Uart_ReceiveData(UARTCH1));
//	}
	

}
/****** 错误中断回调函数 ******/
void ErrIntCallback(void)
{ 
	Uart_SendString(UARTCH1,"UART_Error!\n");
}
/****** UART初始化函数 ******/
void UART_Config(uint16_t uart_baund)
{ 
	/*----- 结构体重命名 -----*/
    stc_uart_config_t  stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
    stc_uart_multimode_t stcMulti;
    stc_uart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;
	/*----- 结构体 -----*/
    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);
    /*----- TXD、RXD引脚定义 -----*/
	Gpio_InitIO(2, 3, GpioDirOut);
	Gpio_InitIO(2, 4, GpioDirOut);
	
    Gpio_SetFunc_UART1TX_P23();
    Gpio_SetFunc_UART1RX_P24();
	/*----- 外部时钟使能 -----*/
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);	//模式0/2可以不使能
    Clk_SetPeripheralGate(ClkPeripheralUart1,TRUE);	//
	/*----- 配置中断相关内容 -----*/
    stcUartIrqCb.pfnRxIrqCb = RxIntCallback;		//接收中断回调函数（写了，有）
    stcUartIrqCb.pfnTxIrqCb = NULL;					//发送中断回调函数（没写）
//    stcUartIrqCb.pfnRxErrIrqCb = ErrIntCallback;	//错误中断回调函数（写了，没有）
	stcUartIrqCb.pfnRxErrIrqCb = NULL;	//错误中断回调函数（写了，没有）
    stcConfig.pstcIrqCb = &stcUartIrqCb;			//配置中断服务函数
    stcConfig.bTouchNvic = TRUE;					//允许中断
	/*----- 配置通讯相关内容 -----*/
    stcConfig.enRunMode = UartMode3;		//测试项，更改此处来转换4种模式测试
    stcMulti.enMulti_mode = UartNormal;		//测试项，更改此处来转换多主机模式，mode2/3才有多主机模式
    stcConfig.pstcMultiMode = &stcMulti;	//配置正常或多机工作模式
	/*----- 配置波特率 -----*/
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
	/*----- 串口及接收中断配置 -----*/
    Uart_Init(UARTCH1, &stcConfig);		//串口1初始化
    Uart_EnableIrq(UARTCH1,UartRxIrq);	//允许串口1接收中断
    Uart_ClrStatus(UARTCH1,UartRxFull);	//清串口1接收中断标志
    Uart_EnableFunc(UARTCH1,UartRx);	//串口1接收中断使能
}
#endif

/*****************************************
** UART-Ymodem
******************************************/
#if 0
void irq_uart_handle(){
   // NDMA 串口接收程序
   static unsigned char uart_ndma_irqsrc;
   uart_ndma_irqsrc = uart_ndmairq_get();
   if(uart_ndma_irqsrc)
   {
       if(reg_uart_status1&FLD_UART_RX_BUF_IRQ)
       {
#if UART_YMODEM_ENABLE
           if (!ymodem_start_flag) // 未开启 ymodem ，进行判断是否开启
           {
               u8 check=uart_ndma_read_byte();
               check_buff[(check_index%20)] = check;
               check_index++;
               if(check_index>=0xff)
                   check_index=0;
               if(check=='\n'){ // 一次传输结束，开始验证
                   if((!strcmp(test_command,(char*)check_buff))&&check_index<=20) {
                       ymodem_start_flag = 1;
                       myprint("ok,please select image binary file\r\n");
                   } else if ((!strcmp(del_custom_flash_command,(char*)check_buff))&&check_index<=20) {
                       ymodem_start_flag = 2;
                       myprint("ok,Deleting\r\n");
                   } else if ((!strcmp(mask_command,(char*)check_buff))&&check_index<=20) {
                       ymodem_start_flag = 3;
                       myprint("ok,please select a mask file\r\n");
                   } else{
                       myprint("unrecognized command\r\n");
                   }
                   check_index=0;
                   // 清空 check_buff
                   for (int i = 0; i < 20; ++i)
                   {
                       check_buff[i] = 0;
                   }
               }
               // u8 check=uart_ndma_read_byte();
               // deQueue(&check);
               // if(check=='S'){
               //  myprint("hello world\r\n");
               //  ymodem_start_flag=1;
               // }
           } else { // 开启 ymodem 功能 所有数据都进入队列 在队列里进行处理
               enQueue(uart_ndma_read_byte());
           }
#endif
       }
   }
  }

void main_loop (){
   if(ymodem_start_flag == 1){
#if EXTERNAL_FLASH_ENABLE
        wd_stop(); // 关闭看门狗
        ymodem_download(); // 下载出厂烧录区域的多媒体文件包 其中包括gif和bmp文件
        wd_start(); // 打开看门狗
       ymodem_start_flag=0;
#endif
    } else if (ymodem_start_flag == 2) {
        // Erase_appoint_flash(0x20000,384);
       // u32 erase_addr = 0x20000;
       // for (u8 i = 0; i < 96; i++) { // 擦除128K-512
       //     erase_addr = i * 0x1000;
       //     FLASH_SectorErase(erase_addr);//扇区4K擦除
       // }
#if EXTERNAL_FLASH_ENABLE
        FLASH_ChipErase(); // 擦除整个外部 flash 芯片
        myprint("\r\nDelete complete");
#endif
        ymodem_start_flag=0;
    } else if (ymodem_start_flag == 3) {
       wd_stop();
        ymodem_download_mask(); // 下载 9073 mask id
        wd_start();
        ymodem_start_flag=0;
   }
}
#endif

int32_t main(void)
{  
    UART_Config(9600u);
	
    while(1)
	{
#if 1
		if (!ymodem_start_flag) // 未开启 ymodem ，进行判断是否开启
		{
			if(u8RxFlg == 1){
				u8RxFlg = 0;
				if((!strcmp(test_command,(char*)u8RxData)) && u8RxCnt<=20){
					ymodem_start_flag = 1;
					Uart_SendString(UARTCH1,(uint8_t*)"ok,please select image binary file\r\n");
				} else if ((!strcmp(del_custom_flash_command,(char*)u8RxData)) && u8RxCnt<=20) {
					ymodem_start_flag = 2;
					Uart_SendString(UARTCH1,(uint8_t*)"ok,Deleting\r\n");
				} else if ((!strcmp(mask_command,(char*)u8RxData)) && u8RxCnt<=20) {
					ymodem_start_flag = 3;
					Uart_SendString(UARTCH1,(uint8_t*)"ok,please select a mask file\r\n");
				} else{
					Uart_SendString(UARTCH1,(uint8_t*)"unrecognized command\r\n");
				}
			}
		}
#endif
	
#if 0
		delay1ms(2000);
		Uart_SendByte(UARTCH1,'C');
		
		
		if(u8RxFlg == 1){
			u8RxFlg = 0;
			
			
			Uart_SendByte(UARTCH1,ACK);
		}
#endif
		
#if 1
		if(ymodem_start_flag == 1){
#if EXTERNAL_FLASH_ENABLE
			wd_stop(); // 关闭看门狗
			ymodem_download(); // 下载出厂烧录区域的多媒体文件包 其中包括gif和bmp文件
			wd_start(); // 打开看门狗
			ymodem_start_flag=0;
#endif
//			ymodem_start_flag=0;
			ymodem_download_mask(); // 下载 9073 mask id
//			Uart_SendString(UARTCH1,u8RxData);
//			delay1ms(2000);
//			Uart_SendByte(UARTCH1,'C');
			
		} else if (ymodem_start_flag == 2) {
//			Erase_appoint_flash(0x20000,384);
//		    u32 erase_addr = 0x20000;
//		    for (u8 i = 0; i < 96; i++) { // 擦除128K-512
//		         erase_addr = i * 0x1000;
//		         FLASH_SectorErase(erase_addr);//扇区4K擦除
//			}
		} else if (ymodem_start_flag == 3) {
//			wd_stop();
//			ymodem_download_mask(); // 下载 9073 mask id
//			wd_start();
//			ymodem_start_flag=0;
		}
#endif	
		
		
		
//		sprintf(u8Buff, "ymodem_start_flag：%d\n", ymodem_start_flag);
//		Uart_SendString(UARTCH1,u8Buff);
//		delay1ms(500);
		
//		Uart_SendString(UARTCH1,u8RxData);
//		delay1ms(500);
		
//		sprintf(u8Buff, "CheckFlg：%d\n", CheckFlg);
//		Uart_SendString(UARTCH1,u8Buff);
//		delay1ms(500);
		
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


