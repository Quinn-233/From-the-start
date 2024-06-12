#include "ddl.h"
#include "uart.h"
#include "bt.h"
#include "gpio.h"
#include "pca.h"
#include "adt.h"
#include "lpm.h"


uint16_t timer=0;
uint32_t pclk=0;
uint8_t u8RxData[10];
uint8_t u8Data[]="RusHHH!\n";
uint8_t u8Buff[10]="";
uint8_t step0[]="0\n";
uint8_t step1[]="1\n";
uint8_t step2[]="2\n";
uint8_t step3[]="3\n";

uint8_t num=1;

static volatile uint32_t u32Cnt = 0;

static volatile uint32_t u32PcaFlag = 0;
/************************************************************
* UART 
*************************************************************/
#if 1
/*------------- 发送一个字节 ----------------*/
void Uart_SendByte(uint8_t u8Idx,uint8_t u8Data)
{
	Uart_SetTb8(u8Idx,Even,u8Data);	
	Uart_SendData(u8Idx,u8Data);				
	while(Uart_GetStatus(u8Idx, UartTxEmpty) == TRUE);
}
/*------------ 发送一个字符串 ---------------*/
void Uart_SendString(uint8_t u8Idx,uint8_t *str)
{
	uint8_t k=0;
	do
	{
		Uart_SendByte(u8Idx,*(str+k));	
		k++;
	}while(*(str+k)!='\0');	//直至遇到字符串结束符 '\0'
}
/*----------- 接收中断回调函数 --------------*/
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
/*----------- 错误中断回调函数 --------------*/
void ErrIntCallback(void)
{ 
}
/*---------- UART初始化配置函数 -------------*/
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
/*--------- GPIO初始化配置函数 ------------*/
void Gpio_config(void)
{
	Gpio_InitIO(0,2,GpioDirIn);	//原理图为KEY1，丝印为BOTTOM2
    Gpio_InitIO(0,1,GpioDirIn);	//原理图为KEY2，丝印为BOTTOM1
	
	Gpio_InitIO(3,2,GpioDirOut);	//LED1
	Gpio_SetIO(3,2,0);
    Gpio_InitIO(0,3,GpioDirOut);	//LED2
	Gpio_SetIO(0,3,0);
	Gpio_InitIO(3,4,GpioDirOut);	//LED3
	Gpio_SetIO(3,4,0);
    Gpio_InitIO(3,5,GpioDirOut);	//LED4
	Gpio_SetIO(3,5,0);
}	
/*------------ 按键检测函数 ---------------*/
uint8_t Key_scale(void)
{
	uint8_t res=0x00;
	if(FALSE == Gpio_GetIO(0,2))
	{
		delay1ms(10);
		if(FALSE == Gpio_GetIO(0,2))
		{
			while(FALSE == Gpio_GetIO(0,2))	res|=0x01;
		}
			
	}
	if(FALSE == Gpio_GetIO(0,1))
	{
		delay1ms(10);
		if(FALSE == Gpio_GetIO(0,1))
		{
			while(FALSE == Gpio_GetIO(0,1))	res|=0x02;
		}
	}	
	return res;
}
/*----------- LED灯切换函数 ---------------*/
void LED_switch(void)
{
	switch(Key_scale())
		{
			case 0x01:
				//Gpio_SetIO(3,2,1);
				num++;
				if(num == 5) num=1;
				break;
			case 0x02:
				//Gpio_SetIO(0,3,1);
				num--;
				if(num == 0) num=4;
				break;
		}
		switch(num)
		{
			case 1:
				Gpio_SetIO(3,2,1);
				Gpio_SetIO(0,3,0);
				Gpio_SetIO(3,4,0);
				Gpio_SetIO(3,5,0);
				break;
			case 2:
				Gpio_SetIO(3,2,0);
				Gpio_SetIO(0,3,1);
				Gpio_SetIO(3,4,0);
				Gpio_SetIO(3,5,0);
				break;
			case 3:
				Gpio_SetIO(3,2,0);
				Gpio_SetIO(0,3,0);
				Gpio_SetIO(3,4,1);
				Gpio_SetIO(3,5,0);
				break;
			case 4:
				Gpio_SetIO(3,2,0);
				Gpio_SetIO(0,3,0);
				Gpio_SetIO(3,4,0);
				Gpio_SetIO(3,5,1);
				break;
		}
}
#endif
/************************************************************
* BaseTimer 
*************************************************************/
#if 1
/*----------- BT0中断服务函数 --------------*/
void Bt1Int(void)
{
    if (TRUE == Bt_GetIntFlag(TIM0))
    {
        Bt_ClearIntFlag(TIM0);
        //每0.1秒触发中断，一次高电平一次电平
		//所以周期为0.2秒
		if(u32Cnt == 0)
		{
			Gpio_SetIO(3,2,1);
			u32Cnt++;
		}else if(u32Cnt == 1)
		{
			Gpio_SetIO(3,2,0);
			u32Cnt--;
		}
    }
}
/*---------- BT0初始化配置函数 -------------*/
void Bt_config(void)
{
	stc_bt_config_t		stcConfig;	//
	stc_clk_config_t	stcClkCfg;	//
    uint16_t          	u16ArrData = 0x10000-25000;		//25000=(0.1*4000000)/16
    uint16_t          	u16InitCntData = 0x10000-25000;	//25000=(0.1*4000000)/16
    
	//CLK INIT
    stcClkCfg.enClkSrc  = ClkRCH;
    stcClkCfg.enHClkDiv = ClkDiv1;
    stcClkCfg.enPClkDiv = ClkDiv1;
    Clk_Init(&stcClkCfg);
	
    //打开GPIO、BT外设时钟
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
	Clk_SetPeripheralGate(ClkPeripheralBt, TRUE);
	Gpio_InitIO(3,2,GpioDirOut);
	Gpio_SetIO(3,2,0);
	
    stcConfig.pfnTim0Cb = Bt1Int;
    //门控使能IO
//	Gpio_SetFunc_TIM0_GATE_P35();
//  Gpio_SetFunc_TIM1_GATE_P25();
//  Gpio_SetFunc_TIM2_GATE_P02();
	
	//Bt初始化
    stcConfig.enGateP = BtPositive;		//门控极性：高电平有效
    stcConfig.enGate  = BtGateDisable;	//门控使能：无效
    stcConfig.enPRS   = BtPCLKDiv16;	//预分频系数：16分频
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
#endif
/************************************************************
* PCA-PWM 
*************************************************************/
#if 1
/*----------- PCA中断服务函数 --------------*/
void PcaInt(void)
{
    if (TRUE == Pca_GetCntIntFlag())		//PCA计数器中断标志获取
    {
        Pca_ClearCntIntFlag();
        u32PcaFlag |= 0x20;
    }
    if (TRUE == Pca_GetIntFlag(Module0))	//PCA中断标志获取
    {
        Pca_ClearIntFlag(Module0);
        u32PcaFlag |= 0x01;
    }
    if (TRUE == Pca_GetIntFlag(Module1))	//PCA中断标志获取
    {
        Pca_ClearIntFlag(Module1);
        u32PcaFlag |= 0x02;
    }
    if (TRUE == Pca_GetIntFlag(Module2))	//PCA中断标志获取
    {
        Pca_ClearIntFlag(Module2);
        u32PcaFlag |= 0x04;
    }
    if (TRUE == Pca_GetIntFlag(Module3))	//PCA中断标志获取
    {
        Pca_ClearIntFlag(Module3);
        u32PcaFlag |= 0x08;
    }
    if (TRUE == Pca_GetIntFlag(Module4))	//PCA中断标志获取
    {
        Pca_ClearIntFlag(Module4);
        u32PcaFlag |= 0x10;
    }    
}
/*---------- PCA初始化配置函数 -------------*/
en_result_t Pca_config(void)
{
    stc_pca_config_t stcConfig;
    stc_pca_capmodconfig_t stcModConfig;
    en_result_t      enResult = Ok;
    uint8_t          u8CcaplData = 0x20;
    uint8_t          u8CcaphData = 0x80;
    
	//PCA、GPIO外设时钟开启
    Clk_SetPeripheralGate(ClkPeripheralPca, TRUE);
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
	
    Gpio_SetFunc_PCA_CH0_P34(0);	//P34-PCA-CH0输出
    /******************PCA 配置结构体定义*********************/
    stcConfig.enCIDL = IdleGoon; 		//休眠模式下启/停控制
    stcConfig.enWDTE = PCAWDTDisable;	//WDT功能控制
    stcConfig.enCPS  = PCAPCLKDiv2; 	//时钟分频及时钟源选择功能
    stcConfig.pfnPcaCb = PcaInt;		//中断服务回调函数
	Pca_Init(&stcConfig);
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
	
    return enResult;
}
#endif
/************************************************************
* ADT-PWM 
*************************************************************/
#if 1
/*----------- PCA中断服务函数 --------------*/
void App_AdvTimerInit(uint16_t u16Period, uint16_t u16CHA_PWMDuty, uint16_t u16CHB_PWMDuty)
{
	en_adt_compare_t	enAdtCompareA;
    en_adt_compare_t	enAdtCompareB;

    stc_adt_basecnt_cfg_t     stcAdtBaseCntCfg;
    stc_adt_CHxX_port_cfg_t   stcAdtTIM6ACfg;
    stc_adt_CHxX_port_cfg_t   stcAdtTIM6BCfg;
    
    DDL_ZERO_STRUCT(stcAdtBaseCntCfg);
    DDL_ZERO_STRUCT(stcAdtTIM6ACfg);
    DDL_ZERO_STRUCT(stcAdtTIM6BCfg);
    
    Clk_SetPeripheralGate(ClkPeripheralAdt, TRUE);    //ADT外设时钟使能
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
	
//	Gpio_SetFunc_TIM6_CHA_P02(0);
//	Gpio_SetFunc_TIM6_CHB_P03(0);
	Gpio_InitIO(0,2,GpioDirOut);
	Gpio_InitIO(0,3,GpioDirOut);
	
    stcAdtBaseCntCfg.enCntMode = AdtSawtoothMode;                 //锯齿波模式
    stcAdtBaseCntCfg.enCntDir = AdtCntUp;
    stcAdtBaseCntCfg.enCntClkDiv = AdtClkPClk0;                 /* 不分频 */
    
    Adt_Init(AdTIM6, &stcAdtBaseCntCfg);                      //ADT载波、计数模式、时钟配置
    
    Adt_SetPeriod(AdTIM6, u16Period);                         //周期设置
    
    enAdtCompareA = AdtCompareA;
    Adt_SetCompareValue(AdTIM6, enAdtCompareA, u16CHA_PWMDuty);  //通用比较基准值寄存器A设置
    
    enAdtCompareB = AdtCompareB;
    Adt_SetCompareValue(AdTIM6, enAdtCompareB, u16CHB_PWMDuty);  //通用比较基准值寄存器B设置
    
    stcAdtTIM6ACfg.enCap = AdtCHxCompareOutput;            //比较输出
    stcAdtTIM6ACfg.bOutEn = TRUE;                          //CHA输出使能
    stcAdtTIM6ACfg.enPerc = AdtCHxPeriodLow;               //计数值与周期匹配时CHA电平保持不变
    stcAdtTIM6ACfg.enCmpc = AdtCHxCompareHigh;             //计数值与比较值A匹配时，CHA电平翻转
    stcAdtTIM6ACfg.enStaStp = AdtCHxStateSelSS;            //CHA起始结束电平由STACA与STPCA控制
    stcAdtTIM6ACfg.enStaOut = AdtCHxPortOutLow;            //CHA起始电平为低
    stcAdtTIM6ACfg.enStpOut = AdtCHxPortOutLow;            //CHA结束电平为低
    Adt_CHxXPortConfig(AdTIM6, AdtCHxA, &stcAdtTIM6ACfg);   //端口CHA配置
    
    stcAdtTIM6BCfg.enCap = AdtCHxCompareOutput;
    stcAdtTIM6BCfg.bOutEn = TRUE;
    stcAdtTIM6BCfg.enPerc = AdtCHxPeriodHigh;
    stcAdtTIM6BCfg.enCmpc = AdtCHxCompareLow;
    stcAdtTIM6BCfg.enStaStp = AdtCHxStateSelSS;
    stcAdtTIM6BCfg.enStaOut = AdtCHxPortOutHigh;
    stcAdtTIM6BCfg.enStpOut = AdtCHxPortOutHigh;
        
    Adt_CHxXPortConfig(AdTIM6, AdtCHxB, &stcAdtTIM6BCfg);    //端口CHB配置
	
	Adt_StartCount(AdTIM6); //AdvTimer6运行
}
#endif

int32_t main(void)
{  
	
//	Uart_config();
//	Gpio_config();
//	Bt_config();
//	Pca_config();
	App_AdvTimerInit(0xC000, 0x4000, 0x8000);
    while(1)
	{	
		//LED_switch();	//KEY、LED测试
		
//		Uart_SetTb8(UARTCH0,Even,u8RxData[0]);
//		Uart_SendData(UARTCH0,step0[0]);
//		delay1ms(500);
		
//		sprintf(u8Buff, "%u\n", pclk);
//		Uart_SendString(UARTCH0,u8Buff);
//		delay1ms(500);
	}
}
