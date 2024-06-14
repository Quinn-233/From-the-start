#ifndef STRING_LIGHTS_H
#define STRING_LIGHTS_H
#include "light_vm.h"
#define STRLIGHTS_COLOR_RGBW    // 是否支持RGBW模式

#define STRLIGHTS_COLOR_MAX_VAL     0xff    // 色彩最大值
#define STRLIGHTS_LUMINANCE_MAX_VAL 0xff    // 最大亮度值

#define STRLIGHTS_START_ADDR        1
#define STRLIGHTS_LENGTH            10     // 需同步修改 LIGVM_CHG_FLAG_BUF_LEN 宏定义
#define STRLIGHTS_MAX_PER_LT_TIME   450     // 设置一个灯珠所需时间(us)，与灯串时序有关

#define STRLIGHTS_DATA_PORT         1//3
#define STRLIGHTS_DATA_PIN          4//5//3

#define STRLIGHTS_CTRL_PORT         0//2
#define STRLIGHTS_CTRL_PIN          3//4

////////////****************************///////
#define LS9073_LIGHT_SEQUENTIAL_NEGATION 0
//固有id 15位							   // 14  -----------  0
#define LS9073_LIGHT_INHERENT_ID    0x0000 //0XXX XXXX XXXX XXXX
//9bit rid								   //        8 -----   0
#define LS9073_LIGHT_RID            0x0000 //0000 000X XXXX XXXX

#define LS9073_LIGHT_NUMBER     	50 //灯的个数
#define LS9073_BIT_LENGTH     		14   //每个包的宽度
#define LS9073_COLORS_BIT_LENGTH    4   //颜色的模式 3：rgb  4：rgbw
//时序位宽1或者6
#define LS9073_CURR_W_BIT_LENGTH    6 // 颜色位宽 1 2 3 4 6

#if LS9073_COLORS_BIT_LENGTH == 3
#define LS9073_CURR_W_BIT_LENGTH_DATA     0
#else
#define LS9073_CURR_W_BIT_LENGTH_DATA     1
#endif
//重置id命令								//
#define LS9073_RESET_ID_COMMAND     0x0400 //0000 0100 0000 0000
//读取id命令								//
#define LS9073_READ_ID_COMMAND     0x1DC4 //0001 1101 1100 0100
//加载id命令								//
#define LS9073_LOADING_ID_COMMAND     0x2664 //0010 0110 0110 0100
#define LS9073_FULL_RECEIPT_COMMAND_FLEXIBLE 0x0800 //0000 1000 0000 0000
//全收参数命令
#define LS9073_FULL_RECEIPT_COMMAND     0x088C //0000 1000 1000 1100
#define LS9073_PARAMETER_ID_M          3//0:9bit mode  1:11bit mode 2:15bit mode 3:20bit mode
#define LS9073_PARAMETER_COLOR_M        3//0:out r  1:out g 2:out b 3:out w
#define LS9073_PARAMETER_A_LOAD_EN		1
#define LS9073_PARAMETER_VGS_EN         1
#define LS9073_PARAMETER_VGS             0x00 //0x00-0x0F
//mode:0
//vgs：00
//curr_w：110
//ep_oe[5:0] :  10 1010(2a)

//串收显存
#define LS9073_SERIAL_RECEIPT_MEMORY_COMMAND     0x0CCC //0000 1100 1100 1100

//全收显存
#define LS9073_FULL_RECEIPT_MEMORY_COMMAND      0x1110//0001 0001 0001 0000
//显示切换指令
#define LS9073_DISPLAY_SWITCH_MEMORY_COMMAND     0x1554//0001 0101 0101 0100
//定点刷新命令
#define LS9073_FIXED_POINT_REFRESH_COMMAND     0x1998  //0001 1001 1001 1000
//extern uint8_t  LS9073_LIGHT_NUMBER;
//加载指令
#define LS9073_LOAD_ID_COMMAND         0x0010           //0000 0000 0001 0000
//串配VGS指令
#define LS9073_SERIAL_VGS_COMMAND         0x2EEC           //0010 1110 1110 1100
//e-fuse指令
#define LS9073_SERIAL_EFUSE_COMMAND         0x2220           //0010 0010 0010 0000

//读取点亮指令
#define LS9073_READ_ID_LIGHT_COMMAND        0x2800 //0010 1000 0000 0000
////////*************************//


// // 老板需反向
// #define STRLIGHTS_DATA_H()          Gpio_SetIO(STRLIGHTS_DATA_PORT, STRLIGHTS_DATA_PIN, 0); // 输出到灯串，反向
// #define STRLIGHTS_DATA_L()          Gpio_SetIO(STRLIGHTS_DATA_PORT, STRLIGHTS_DATA_PIN, 1);
#define STRLIGHTS_DATA_H()          Gpio_SetIO(STRLIGHTS_DATA_PORT, STRLIGHTS_DATA_PIN, 1);
#define STRLIGHTS_DATA_L()          Gpio_SetIO(STRLIGHTS_DATA_PORT, STRLIGHTS_DATA_PIN, 0);

#define STRLIGHTS_ON()              Gpio_SetIO(STRLIGHTS_CTRL_PORT, STRLIGHTS_CTRL_PIN, 1);
#define STRLIGHTS_OFF()             Gpio_SetIO(STRLIGHTS_CTRL_PORT, STRLIGHTS_CTRL_PIN, 0);

void strlights_gpio_init(void);
void strlights_set_color(unsigned short int addr, unsigned char *color);
void LS9073_send_data_packet(unsigned short int bitdate);
void LS9073_send_fun_reset_id(void);
void LS9073_send_fun_reset_one_id(void);
void LS9073_send_fun_reset_known_id(uint32_t known_id,uint16_t n_id);
void LS9073_send_fun_read_id(void);
void LS9073_send_fun_loading_id(void);
void LS9073_send_fun_full_receipt(void);
void LS9073_color_length_swap666(unsigned short int addr,unsigned short int *data,unsigned short int *data1,unsigned short int *data2);
void LS9073_send_fun_serial_receipt_memory(void);
void LS9073_send_fun_full_receipt_memory(unsigned char r,unsigned char g,unsigned char b,unsigned char w);
void LS9073_send_fun_display_switch_memory(void);
void LS9073_send_fun_fixed_point_refresh(unsigned short int addr);
//void LS9073_send_fun_e_fuse(unsigned short int OTP,unsigned short int P_time);
void LS9073_send_fun_full_receipt_flexible(unsigned short int Ep_oe,unsigned short int mode,unsigned short int VGS,unsigned short int Curr_w,unsigned short int Vgs_en,unsigned short int a_load_en,unsigned short int color_m,unsigned short int id_m);
void LS9073_send_fun_serial_vgs(void);
void LS9073_send_pwm_delay(unsigned short int cnt);
void LS9073_read_id_light(unsigned short int id);
#endif // STRING_LIGHTS_H
