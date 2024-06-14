#include "hc32f005.h"
#include "main.h"
#include "systick.h"
#include "key.h"
#include "com.h"
#include "disk.h"
#include "string_lights.h"
#include "light_event.h"
#include "light_effects.h"
#include "light_sdata.h"
#include "light_vm.h"

uint16_t timer=0;
uint32_t pclk=0;
/*----------- 接收中断回调函数 --------------*/
void RxCallback(void)
{
}
/*----------- 错误中断回调函数 --------------*/
void ErrCallback(void)
{ 
}
/*---------- UART初始化配置函数 -------------*/
void Uart_config(uint32_t Ubaud)
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
	/*----- TXD、RXD引脚配置 -----*/
    //Gpio_InitIOExt(3,5,GpioDirOut,TRUE,FALSE,FALSE,FALSE);   
	//Gpio_InitIOExt(3,6,GpioDirIn,TRUE,FALSE,FALSE,FALSE); 
	Gpio_InitIO(3,5,GpioDirOut);   
    Gpio_InitIO(3,6,GpioDirIn);
	
    Gpio_SetFunc_UART0TX_P35();
    Gpio_SetFunc_UART0RX_P36();
	/*************************外部时钟使能**************************/
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);	//模式0/2可以不使能
    Clk_SetPeripheralGate(ClkPeripheralUart0,TRUE);	//
	/************************配置中断相关内容************************/
    stcUartIrqCb.pfnRxIrqCb = RxCallback;		//接收中断回调函数（写了，有）
    stcUartIrqCb.pfnTxIrqCb = NULL;					//发送中断回调函数（没写）
    stcUartIrqCb.pfnRxErrIrqCb = ErrCallback;	//错误中断回调函数（写了，没有）
    stcConfig.pstcIrqCb = &stcUartIrqCb;			//配置中断服务函数
    stcConfig.bTouchNvic = TRUE;					//允许中断
	/************************配置通讯相关内容************************/
    stcConfig.enRunMode = UartMode3;		//测试项，更改此处来转换4种模式测试
    stcMulti.enMulti_mode = UartNormal;		//测试项，更改此处来转换多主机模式，mode2/3才有多主机模式
    stcConfig.pstcMultiMode = &stcMulti;	//配置正常或多机工作模式
	Uart_Init(UARTCH0, &stcConfig);			//串口0初始化
	/**************************配置波特率**************************/
    stcBaud.bDbaud = 0u;							//双倍波特率功能
    stcBaud.u32Baud = Ubaud;						//设置目标波特率
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

static void system_led_init(void)
{
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE); // 使能gpio时钟
    SYS_LED_INIT();
    SYS_LED_OFF();
}

static void system_led_handle(void)
{
    static u8 cnt = 1;
    static u32 led_time = 0;

    if (sys_time_exceed_us(led_time, 500 * 1000)) {
        led_time = SYS_TICK_GET_VAL();
        SYS_LED_SETIO(cnt % 2);
        cnt++;
    }
}

static void bsp_init(void)
{
    key_init();
    //com_init();
    //disk_init();
    system_led_init();
    strlights_gpio_init();
}
void delay_ms(u16 time)
{
    u16 i = 0;
    while (time--) {
        i = 3871; //自己定义
        while (i--) ;
    }
}
// 默认颜色定义
//const pixel_u LS9073_default_color[9] = {
//    { .value = 0x0000ff00 }, // r: 255 g: 0 b: 0 bgrw
//    { .value = 0x00be0000 },
//    { .value = 0xbe000000 },
//    { .value = 0x4baaff00 },
//    { .value = 0x00beff00 },
//    { .value = 0x5abe0000 },
//    { .value = 0x5a00ff00 },
//    { .value = 0x0064ff00 },
//    { .value = 0x15be0000 },
//};
const pixel_u LS9073_default_color[] = {
    // r: 255 g: 0 b: 0  顺序：bgrw
    { .value = 0x0000ff00 }, // r g b w gb rb rg rgb rw gw bw wgb wrb wrg wrgb 灭
    { .value = 0x00ff0000 },
    { .value = 0xff000000 },
    { .value = 0x000000ff },
    { .value = 0xffff0000 },
    { .value = 0xff00ff00 },
    { .value = 0x00ffff00 },
    { .value = 0xffffff00 },
    { .value = 0x0000ffff },
    { .value = 0x00ff00ff },
    { .value = 0xff0000ff },
    { .value = 0xffff00ff },
    { .value = 0xff00ffff },
    { .value = 0x00ffffff },
    { .value = 0xffffffff },
    { .value = 0x00000000 },
};

uint8_t addr_test = 0;
uint8_t colors = 0;
//#define BURN_ID (0x7800)
//#define LIGHT_UP_ID (0x0000|0x555)
//#define testid 0x0000
#define BURN_ID 1 //9bit efuse
//#define BURN_ID (NUMBER_ID <<11) //20 bit id
#define LIGHT_UP_ID (BURN_ID <<11 |0x3ff) // 点亮id
//extern uint16_t new_id[200];
extern uint16_t whole_id[];
uint8_t VGS_test = 0x0f;
uint8_t CLO_test = 0;
int32_t main(void)
{
    uint8_t key_flag = 0;
    sysclk_init();
    systick_init();
    bsp_init();
	Uart_config(9600u);
    //ligeff_init();
	uint16_t cnt = 0;
    uint16_t flag = 0;
	uint16_t flag1 = 0;
	uint16_t flag2 = 0;
	uint16_t flag3 = 0;
    uint8_t color_number = 0;
	for (uint16_t i = 0; i < 200; i++) { //LS9073_LIGHT_NUMBER
		LS9073_color[i].value = 0; //显存清除
//        LS9073_color[i].value = 0xffffffff;
    }
//初始化
delay_ms(800);
	LS9073_send_fun_full_receipt_flexible(0, LS9073_CURR_W_BIT_LENGTH_DATA, LS9073_PARAMETER_VGS, LS9073_CURR_W_BIT_LENGTH, LS9073_PARAMETER_VGS_EN, LS9073_PARAMETER_A_LOAD_EN,LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
    LS9073_send_fun_full_receipt_memory(0, 0, 0, 0);//全收显存
    LS9073_send_fun_display_switch_memory();
	delay_ms(1);
	LS9073_send_fun_full_receipt_flexible(0xAAAA, LS9073_CURR_W_BIT_LENGTH_DATA, LS9073_PARAMETER_VGS, LS9073_CURR_W_BIT_LENGTH, LS9073_PARAMETER_VGS_EN,LS9073_PARAMETER_A_LOAD_EN,LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
	delay_ms(1);
//	LS9073_send_fun_loading_id();//加载id
//	delay_ms(1);
//	LS9073_send_fun_reset_known_id(0X8BFF,0);
//	LS9073_send_fun_reset_known_id(0x3ff,0);
//	LS9073_send_fun_reset_id();

	delay_ms(1);
//	LS9073_send_fun_serial_vgs();
	delay_ms(1);

    while (1) {
        systick_time_sync();
        system_led_handle();
        //flash_manage_loop();
        key_scan();
        //ligevt_process();
		Uart_SendData(UARTCH0,VGS_test);
        key_status_e key1_status = key_status_get(KEY1);
        key_status_e key2_status = key_status_get(KEY2);
        if (key2_status == KEY_STATUS_SON) {//效果键
            key_flag = 1;
        }
        if (key1_status == KEY_STATUS_SON) {
            key_flag = 1;
           	VGS_test++; // 修改vgs测电流
			if(VGS_test>0x0f)
				VGS_test=0;
        }
		if (key2_status == KEY_STATUS_SON) {
            key_flag = 1;
           	CLO_test++; // 修改vgs测电流
			if(CLO_test>4) CLO_test=0;
        }
        if (key_flag) {
            key_flag = 0;
			if(CLO_test<3)
			{
				LS9073_send_fun_full_receipt_flexible(	0xAAAA, LS9073_CURR_W_BIT_LENGTH_DATA, VGS_test,\
														4, LS9073_PARAMETER_VGS_EN,LS9073_PARAMETER_A_LOAD_EN,\
														LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
				switch(CLO_test){
					case 0:
						LS9073_send_fun_full_receipt_memory(0xff,0x00,0x00,0x00);
						break;
					case 1:
						LS9073_send_fun_full_receipt_memory(0x00,0xff,0x00,0x00);
						break;
					case 2:
						LS9073_send_fun_full_receipt_memory(0x00,0x00,0xff,0x00);
						break;
					case 3:
						LS9073_send_fun_full_receipt_memory(0xff,0xff,0xff,0xff);
						break;
				}
				LS9073_send_fun_display_switch_memory();
			}else
			{
				LS9073_send_fun_full_receipt_flexible(	0xAAAA, LS9073_CURR_W_BIT_LENGTH_DATA, VGS_test,\
														3, LS9073_PARAMETER_VGS_EN,LS9073_PARAMETER_A_LOAD_EN,\
														LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
				LS9073_send_fun_full_receipt_memory(0xff,0xff,0xff,0x00);
				LS9073_send_fun_display_switch_memory();
			}
        }

////全收显存功能测试
//		cnt++;
//		if(cnt>=60000){
//			//全收参数
//			cnt=0;
//			LS9073_send_fun_full_receipt_flexible(0xAAAA, LS9073_CURR_W_BIT_LENGTH_DATA, LS9073_PARAMETER_VGS, LS9073_CURR_W_BIT_LENGTH, LS9073_PARAMETER_VGS_EN,LS9073_PARAMETER_A_LOAD_EN,LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
////			LS9073_send_fun_full_receipt();
//		    //全收显存
////			LS9073_send_fun_full_receipt_memory(0xff,0xff,0xff,0xff);
//		    LS9073_send_fun_full_receipt_memory(LS9073_default_color[flag].rgb.r,LS9073_default_color[flag].rgb.g,LS9073_default_color[flag].rgb.b,LS9073_default_color[flag].rgb.w);
//		    //显示切换
//		    LS9073_send_fun_display_switch_memory();
//		    flag++;
//		    if(flag >= 16)//16个颜色
//			    flag = 0;
//		}

//////串收流水灯
//		cnt++;
//		if (cnt >= 20000) {
//			cnt = 0;
////			LS9073_send_fun_full_receipt_flexible(0, LS9073_CURR_W_BIT_LENGTH_DATA, LS9073_PARAMETER_VGS, LS9073_CURR_W_BIT_LENGTH, LS9073_PARAMETER_VGS_EN, LS9073_PARAMETER_A_LOAD_EN,LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
////			LS9073_send_fun_full_receipt_memory(0, 0, 0, 0);//全收显存
////			LS9073_send_fun_display_switch_memory();
////			LS9073_send_fun_full_receipt_flexible(0xAAAA, LS9073_CURR_W_BIT_LENGTH_DATA, LS9073_PARAMETER_VGS, LS9073_CURR_W_BIT_LENGTH, LS9073_PARAMETER_VGS_EN,LS9073_PARAMETER_A_LOAD_EN,LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
////			LS9073_send_fun_loading_id();//加载id
////			LS9073_send_fun_reset_id();
////			LS9073_color[0x41].value = LS9073_default_color[0].value;
////			LS9073_color[0x42].value = LS9073_default_color[1].value;
////			LS9073_color[0x43].value = LS9073_default_color[2].value;
//			if (flag < LS9073_LIGHT_NUMBER) {
//
//				LS9073_color[flag].value = LS9073_default_color[color_number].value;
////				if (flag == 66)
////					LS9073_color[flag].value = LS9073_default_color[2].value;
//			} else {
//				LS9073_color[flag - LS9073_LIGHT_NUMBER].value = LS9073_default_color[15].value;
//			}
//			//串收显存
////			LS9073_color[35].value = 0xff000000;
//			LS9073_send_fun_serial_receipt_memory();
////			for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i++) {
////          //定点刷新函数
////					LS9073_send_fun_fixed_point_refresh(i);
////			}
////			//显示切换
////      LS9073_send_fun_display_switch_memory();
//			flag++;
//			if (flag >= LS9073_LIGHT_NUMBER * 2) {
//				flag = 0;
//				color_number++;
//				if (color_number >= 16) {
//					color_number = 0;
//				}
//			}
//		}
//
////定点点亮流水灯
//      cnt++;
//      if(cnt>=5000){
//          cnt=0;
////		  //全收参数
////		  LS9073_send_fun_full_receipt_flexible(0, LS9073_CURR_W_BIT_LENGTH_DATA, LS9073_PARAMETER_VGS, LS9073_CURR_W_BIT_LENGTH, LS9073_PARAMETER_VGS_EN,LS9073_PARAMETER_A_LOAD_EN,LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
////    LS9073_send_fun_full_receipt_memory(0, 0, 0, 0);//全收显存
////    LS9073_send_fun_display_switch_memory();
////		LS9073_send_fun_full_receipt_flexible(0xAAAA, LS9073_CURR_W_BIT_LENGTH_DATA, LS9073_PARAMETER_VGS, LS9073_CURR_W_BIT_LENGTH, LS9073_PARAMETER_VGS_EN,LS9073_PARAMETER_A_LOAD_EN,LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
////		delay_ms(1);
////		LS9073_send_fun_loading_id();//加载id
//////		delay_ms(1);
////		LS9073_send_fun_reset_id();
////		LS9073_send_fun_reset_known_id(0x3ff,0);
//		delay_ms(1);
//          if(flag < LS9073_LIGHT_NUMBER){
////			  if(flag>30) {
//				   LS9073_color[flag].value = LS9073_default_color[color_number].value;
//				  //定点刷新函数
//				  LS9073_send_fun_fixed_point_refresh(flag);
//	//			  LS9073_send_fun_full_receipt_memory(0,0xff,0,0);
//				  //显示切换
//				  LS9073_send_fun_display_switch_memory();
////			  }
//          }else{
//              LS9073_color[flag-LS9073_LIGHT_NUMBER].value = LS9073_default_color[15].value;
//              //定点刷新函数
//              LS9073_send_fun_fixed_point_refresh(flag-LS9073_LIGHT_NUMBER);
////			  LS9073_send_fun_full_receipt_memory(0,0,0,0);
//              //显示切换
//              LS9073_send_fun_display_switch_memory();
//          }
////			if(flag%2) {
////				LS9073_color[0].value = LS9073_default_color[0].value;
////              //定点刷新函数
////              LS9073_send_fun_fixed_point_refresh(0);
////              //显示切换
////              LS9073_send_fun_display_switch_memory();
////
////			} else {
////				LS9073_color[53].value = LS9073_default_color[1].value;
////              //定点刷新函数
////              LS9073_send_fun_fixed_point_refresh(53);
////              //显示切换
////              LS9073_send_fun_display_switch_memory();
////
////			}
//          flag++;
//          if(flag >= LS9073_LIGHT_NUMBER*2){
//              flag=0;
//              color_number++;
//              if(color_number >= 16)
//                  color_number = 0;
//          }

//      }


//// 读id成功自动点亮
//		cnt++;
//		if (cnt >= 50000) {
//				cnt = 0;
////				LS9073_send_fun_full_receipt_memory(0, 0, 0, 0);//全收显存
////				LS9073_send_fun_display_switch_memory();
//				LS9073_send_fun_full_receipt_flexible(0xAAAA, LS9073_CURR_W_BIT_LENGTH_DATA, LS9073_PARAMETER_VGS, LS9073_CURR_W_BIT_LENGTH, LS9073_PARAMETER_VGS_EN,LS9073_PARAMETER_A_LOAD_EN,LS9073_PARAMETER_COLOR_M,LS9073_PARAMETER_ID_M);
//				LS9073_send_fun_loading_id();//加载id
////				LS9073_send_fun_reset_id();
//				LS9073_read_id_light(flag);
//				LS9073_send_fun_display_switch_memory();
//				flag++;
//          if(flag >= LS9073_LIGHT_NUMBER){
//              flag=0;
//          }
//
//		}




//亮度测试程序
//		//全收显存亮度测试
//		cnt++;
//		if(cnt>=500){
//			//全收参数
//			//LS9073_send_fun_full_receipt();
//		    cnt=0;
//		    //全收显存
////		    LS9073_send_fun_full_receipt_memory(LS9073_default_color[flag].rgb.r,LS9073_default_color[flag].rgb.g,LS9073_default_color[flag].rgb.b,LS9073_default_color[flag].rgb.w);
//		    LS9073_send_fun_full_receipt_memory(flag%0xff,flag1%0xff,flag2%0xff,flag3%0xff);
//			//显示切换
//		    LS9073_send_fun_display_switch_memory();
//			if(flag==0&&flag1==0&&flag2==0&&flag3==0) {
//				flag = LS9073_default_color[0].rgb.r;
//				flag1 = LS9073_default_color[0].rgb.g;
//				flag2 = LS9073_default_color[0].rgb.b;
//				flag3 = LS9073_default_color[0].rgb.w;
//			}
//			if(flag) {
//				flag ++;
//			}
//			if(flag1) {
//				flag1 ++;
//			}
//			if(flag2) {
//				flag2 ++;
//			}
//			if(flag3) {
//				flag3 ++;
//			}
//			if(flag%0xff==254||flag1%0xff==254||flag2%0xff==254||flag3%0xff==254) {
//				color_number++;
//				if(color_number >= 16)
//					color_number = 0;
//				flag = LS9073_default_color[color_number].rgb.r;
//				flag1 = LS9073_default_color[color_number].rgb.g;
//				flag2 = LS9073_default_color[color_number].rgb.b;
//				flag3 = LS9073_default_color[color_number].rgb.w;
//			}
//		}

		//串收显存亮度测试
//		cnt++;
//		if (cnt >= 1) {
//			cnt = 0;
//			if(flag==0&&flag1==0&&flag2==0&&flag3==0) {
//				flag = LS9073_default_color[0].rgb.r;
//				flag1 = LS9073_default_color[0].rgb.g;
//				flag2 = LS9073_default_color[0].rgb.b;
//				flag3 = LS9073_default_color[0].rgb.w;
//			}
//			for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i++) {
////				LS9073_color[i].value = LS9073_color[0].value; //显存清除
//				LS9073_color[i].rgb.r=flag;
//				LS9073_color[i].rgb.g=flag1;
//				LS9073_color[i].rgb.b=flag2;
//				LS9073_color[i].rgb.w=flag3;
//			}
//			if(flag) {
//				flag=(flag+1)%0xff;
//			}
//			if(flag1) {
//				flag1=(flag1+1)%0xff;
//			}
//			if(flag2) {
//				flag2=(flag2+1)%0xff;
//			}
//			if(flag3) {
//				flag3=(flag3+1)%0xff;
//			}
//			//串收显存亮度测试
//			LS9073_send_fun_serial_receipt_memory();
//			//定点点亮亮度测试
////			for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i++) {
////				//定点点亮
////				LS9073_send_fun_fixed_point_refresh(i);//把id为0的灯点亮
////				//显示切换
////				LS9073_send_fun_display_switch_memory();
////			}
//			if(flag%0xff==254||flag1%0xff==254||flag2%0xff==254||flag3%0xff==254) {
//				color_number++;
//				if(color_number >= 16)
//					color_number = 0;
//				flag = LS9073_default_color[color_number].rgb.r;
//				flag1 = LS9073_default_color[color_number].rgb.g;
//				flag2 = LS9073_default_color[color_number].rgb.b;
//				flag3 = LS9073_default_color[color_number].rgb.w;
//			}
//		}

    }
}
