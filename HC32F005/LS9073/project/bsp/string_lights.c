#include "hc32f005.h"
#include "systick.h"
#include "key.h"
#include "string_lights.h"
#include "light_vm.h"
#define STRLIGHTS_CMD_CODE  3

static __inline void delay_times(u32 n)
{
    while (n--);
}
int delay;
// 老板子
// // L 3.2us H 2.6us（send 0 需5.8us）占空比0.4483
// #define STRLIGHTS_SET_LOW() { \
//     STRLIGHTS_DATA_L(); \
//     delay_times(6); \
//     STRLIGHTS_DATA_H(); \
//     delay_times(1); \
// }

// // L 6.4us H 2.6us（H delay1) (send 1 需9us）
// // 为保证与set_low占空比相同，这里 H 需保持5.2us，故send 1 需11.6us
// #define STRLIGHTS_SET_HIGH() { \
//     STRLIGHTS_DATA_L(); \
//     delay_times(22); \
//     STRLIGHTS_DATA_H(); \
//     delay_times(13); \
// }

// // L 13.5us（send stop 需15.5us）
// #define STRLIGHTS_SET_STOP() { \
//     STRLIGHTS_DATA_L(); \
//     delay_times(54); \
//     STRLIGHTS_DATA_H(); \
//     delay_times(1); \
// }

// L 3.2us H 2.6us（send 0 需5.8us）占空比0.4483
#define STRLIGHTS_SET_LOW() { \
    STRLIGHTS_DATA_L(); \
    delay_times(9); \
    STRLIGHTS_DATA_H(); \
    delay_times(2); \
}

// L 6.4us H 2.6us（H delay1) (send 1 需9us）
// 为保证与set_low占空比相同，这里 H 需保持5.2us，故send 1 需11.6us
#define STRLIGHTS_SET_HIGH() { \
    STRLIGHTS_DATA_L(); \
    delay_times(24); \
    STRLIGHTS_DATA_H(); \
    delay_times(13); \
}

// L 13.5us（send stop 需15.5us）
#define STRLIGHTS_SET_STOP() { \
    STRLIGHTS_DATA_L(); \
    delay_times(54); \
    STRLIGHTS_DATA_H(); \
    delay_times(1); \
}
/////*******************LS9073***************************/////////
//低电平无要求，高电平大于4个振荡周期
//置0时，高电平时间为TM 置1时高电平时间>2TM

//500K主频，周期2us，高电平最小时间4*2=8us，停止位时间为32*2=64us
//// 驱动板供电、示波器测量、0x555芯片（500K主频）
//#define  Low_level   		 12		//  2.8us
//#define  High_level_0   31		//  11us
//#define  High_level_1   138   //   33.2us
//#define  Stop_level_1    345   //  78us
//#define  Low_level   	 30		// 9-11   2us-2us  大于11有一颗随第一颗一起亮
//#define  High_level_0    48		// 30-50      11.4us-15.40us
//#define  High_level_1   200   // 140-295    34.4us-67.2us
//#define  Stop_level_1    600   // 350 79.2us
////// IO、示波器测量、0x2aa芯片（500K主频） 驱动板
//#define  Low_level   		 40		//  2.8us  10
//#define  High_level_0   35		//  11us  35
//#define  High_level_1   150   //   34us   150
//#define  Stop_level_1    360   //  78us

//// IO供电、示波器测量、0x7ff芯片(1M主频)
//#define  Low_level   		 18		//  最小值：2us(极限)
//#define  High_level_0   39		// 最小值：5.6us
//#define  High_level_1   140   //   最小值：16us
//#define  Stop_level_1    600   // 最小值： 35us

////驱动板 0x7ff芯片（1M） 多颗
//#define  Low_level   		 10		//  4us
//#define  High_level_0   18		//  10us
//#define  High_level_1   80   //  35us
//#define  Stop_level_1    400   //  78us

//驱动板 0x7ff芯片（1M） 单颗
//#define  Low_level   		 40		//  4us
//#define  High_level_0   9		//  10us
//#define  High_level_1   65   //  35us
//#define  Stop_level_1    600   //  78us


//新7ff芯片 2M主频 驱动板供电
//#define  Low_level   		 40		//  1.78us
//#define  High_level_0   10		//  6.2
//#define  High_level_1   40   //  35--50
//#define  Stop_level_1    80   //  理论：32*0.5=16us 65: 19/24us

//#define  Low_level   		 8		// 2us
//#define  High_level_0   3		// 0--14   5.2us--8us
//#define  High_level_1   28   //  28--52    11us--16.2us
//#define  Stop_level_1    3   //  理论：32*0.5=16us 65: 19/24us   63:19us

//#define  Low_level   		 20		// 2us
//#define  High_level_0   5		// 0--14   5.2us--8us
//#define  High_level_1   35   //  28--52    11us--16.2us
//#define  Stop_level_1    70   //  理论：32*0.5=16us 65: 19/24us   63:19us

#define  Pwm_Low     30
#define  Pwm_High    60
//FPGA
//#define  Low_level   		 1
//#define  High_level_0   30
//#define  High_level_1   100
//#define  Stop_level_1    300
//新版本灯珠芯片 2M //////////////////////////////////////
//#define  Low_level   		 15		//
//#define  High_level_0   20		//
//#define  High_level_1   55   //
//#define  Stop_level_1    600   //
//#define  Low_level   		 60		//  1.78us  烧录 单颗
//#define  High_level_0   4		//  6.2
//#define  High_level_1   40   //  35--50
//#define  Stop_level_1    200   //  理论：32*0.5=16us 65: 19/24us
// 多颗
//#define  Low_level   		 15		//  1.78us
//#define  High_level_0   0		//  6.2
//#define  High_level_1   30   //  35--50
//#define  Stop_level_1    0   //  理论：32*0.5=
//#define  Low_level   		 35		//  1.78us
//#define  High_level_0   2		//  6.2
//#define  High_level_1   30   //  35--50
//#define  Stop_level_1    2   //  理论：32*0.5=
#define  Low_level   		 25		//  1.78us
#define  High_level_0   0		//  6.2
#define  High_level_1   35   //  35--50
#define  Stop_level_1    2   //  理论：32*0.5=

#if LS9073_LIGHT_SEQUENTIAL_NEGATION
#define LS9073_SET_0() { \
    STRLIGHTS_DATA_H(); \
    delay_times(Low_level); \
	STRLIGHTS_DATA_L(); \
    delay_times(40); \
}
#define LS9073_SET_1() { \
	STRLIGHTS_DATA_H(); \
    delay_times(Low_level); \
    STRLIGHTS_DATA_L(); \
    delay_times(150); \
}
//空闲高电平时间大于32个振荡周期
#define LS9073_SET_free() { \
	STRLIGHTS_DATA_H(); \
    delay_times(Low_level); \
    STRLIGHTS_DATA_L(); \
    delay_times(600); \
}
/*
#if LS9073_LIGHT_SEQUENTIAL_NEGATION
#define LS9073_SET_0() { \
    STRLIGHTS_DATA_H(); \
    delay_times(30); \
	STRLIGHTS_DATA_L(); \
    delay_times(40); \
}
#define LS9073_SET_1() { \
	STRLIGHTS_DATA_H(); \
    delay_times(30); \
    STRLIGHTS_DATA_L(); \
    delay_times(150); \
}
//空闲高电平时间大于32个振荡周期
#define LS9073_SET_free() { \
	STRLIGHTS_DATA_H(); \
    delay_times(30); \
    STRLIGHTS_DATA_L(); \
    delay_times(600); \
}
*/


#else
#define LS9073_SET_0() { \
    STRLIGHTS_DATA_L(); \
    delay_times(Low_level); \
    STRLIGHTS_DATA_H(); \
    delay_times(High_level_0); \
}

#define LS9073_SET_1() { \
    STRLIGHTS_DATA_L(); \
    delay_times(Low_level); \
    STRLIGHTS_DATA_H(); \
    delay_times(High_level_1); \
}

//空闲高电平时间大于32个振荡周期
#define LS9073_SET_free() { \
    STRLIGHTS_DATA_L(); \
    delay_times(Low_level); \
    STRLIGHTS_DATA_H(); \
    delay_times(Stop_level_1); \
}
#endif
//pwm
#define LS9073_PWM_free() { \
    STRLIGHTS_DATA_L(); \
    delay_times(Pwm_Low); \
    STRLIGHTS_DATA_H(); \
    delay_times(Pwm_High); \
}
//发送包的函数 从高位发送
void LS9073_send_data_packet(uint16_t bitdate)
{
    //LS9073_SET_free();
    //发送高14位
    for (u8 i = 0; i < LS9073_BIT_LENGTH; i++) {
        if (bitdate & 0x8000) {
            LS9073_SET_1();
        } else {
            LS9073_SET_0();
        }
        bitdate <<= 1;
    }
    LS9073_SET_free();
}
//uint8_t  LS9073_LIGHT_NUMBER = 9;
//uint16_t inherent_id[LS9073_LIGHT_NUMBER]={1,2,3,4,5,6,7,8,9};
//uint16_t new_id[LS9073_LIGHT_NUMBER]={1,2,3,4,5,6,7,8,9};
//uint16_t inherent_id[] = {0x5555, 0x2aaa, 0, 0x7fff, 0x45, 0x4006, 0x0807, 0x0208, 0x2009, 0x040b};// 模拟板固有id
//uint32_t whole_id[] = {0XD5D55, 0XAAAAA,0X88000,0XFFFFF,0X88045,0XCC006,0X88807,0X88208,0XAA009,0X8840B};
//uint32_t whole_id[] = {0XD5D55, 0XAAAAA,0X88000,0XFF3FF,0X88045,0XCC006,0X88807,0X88208,0XAA009,0X8840B};
uint32_t whole_id[] = {0X3FF, 0XBFF,0X13FF,0X1BFF,0X23FF,0X2BFF,0X33FF,0X3BFF,0X43FF,0X4BFF,0X53FF,0X5BFF,0X63FF,0X6BFF,0X73FF,0X7BFF,0X83FF,0X8BFF,0X93FF,0X9BFF,0XA3FF,0XABFF,0XB3FF,0XBBFF,0XC3FF,0XCBFF,0XD3FF,0XDBFF,0XE3FF,0XEBFF,0XF3FF,0XFBFF,0X103FF,0X10BFF,0X113FF,0X11BFF,0X123FF,0X12BFF,0x133ff,0x13bff,0x143ff,0x14bff,0x153ff,0x15bff,0x163ff,0x16bff,0x173ff,0x17bff,0x183ff,0x18bff,0x193ff,0x19bff,0x1a3ff,0x1abff,0x1b3ff,0x1bbff,0x1c3ff,0x1cbff,0x1d3ff,0x1dbff,0x1e3ff,0x1ebff,0x1f3ff,0x1fbff,0x203ff,0x20bff,0x213ff,0x21bff,0x223ff,0x22bff};
//高9位： 427 ,341,272,511,272,408,273,272,340,272
uint16_t inherent_id[] = {0x5D55, 0xaaaa, 0, 0xffff, 0x8045, 0xc006, 0x8807, 0x8208, 0xa009, 0x840b};// 模拟板固有id
//                        0000     0001    0010     0011   0100     0101    0110    0111   1000     1001  1010    1011  1100   1101   1110  1111
//uint16_t inherent_id[200] = {0x07ff, 0x17ff, 0x1fff, 0x27ff, 0x2fff, 0x37ff, 0x3fff, 0x47ff, 0x4fff,0x57ff,0x5fff,0x67ff,0x6fff,0x77ff,0x7fff,0x0fff, \
//							0x2aa,0xaaa,0x12aa,0x1aaa,0x22aa,0x2aaa,0x32aa,0x3aaa,0x42aa,0x4aaa,0x52aa,0x5aaa,0x62aa,0x6aaa,0x72aa,0x7aaa};
//uint16_t inherent_id[200] = {0x07ff,0x0fff,0x17ff, 0x1fff, 0x27ff, 0x2fff, 0x37ff, 0x3fff, 0x47ff, 0x4fff,0x57ff,0x5fff,0x67ff,0x6fff,0x77ff,0x7fff, \
//							0x2aa,0xaaa,0x12aa,0x1aaa,0x22aa,0x2aaa,0x32aa,0x3aaa,0x42aa,0x4aaa,0x52aa,0x5aaa,0x62aa,0x6aaa,0x72aa,0x7aaa};
//uint16_t inherent_id[200] = {0x0555,0x0d55,0x1555, 0x1d55, 0x2555, 0x2d55, 0x3555, 0x3d55, 0x4555, 0x4d55,0x5555,0x5d55,0x6555,0x6d55,0x7555,0x7d55, \
//							0x2aa,0xaaa,0x12aa,0x1aaa,0x22aa,0x2aaa,0x32aa,0x3aaa,0x42aa,0x4aaa,0x52aa,0x5aaa,0x62aa,0x6aaa,0x72aa,0x7aaa};
//uint16_t inherent_id[200] = {0x2aa,0xaaa,0x12aa,0x1aaa,0x22aa,0x2aaa,0x32aa,0x3aaa,0x42aa,0x4aaa,0x52aa,0x5aaa,0x62aa,0x6aaa,0x72aa,0x7aaa, \
//							0x07ff,0x0fff,0x17ff, 0x1fff, 0x27ff, 0x2fff, 0x37ff, 0x3fff, 0x47ff, 0x4fff,0x57ff,0x5fff,0x67ff,0x6fff,0x77ff,0x7fff};
//uint16_t inherent_id[200] = {0x3fff, 0x47ff, 0x4fff,0x57ff,0x5fff,0x7fff};
//uint16_t inherent_id[] = {0x2aa, 0x555, 0x555, 0x555, 0x555, 0x555, 0x555, 0x555, 0x555, 0x555};
//uint16_t new_id[]={0,1,85,100,120,150,170,300,400,511};
uint16_t efuse_id[] = {0x01ab,0x0155,0x0110,0x01ff,0x0110,0x0198,0x0111,0x0110,0x0154,0x0110};
//uint16_t new_id[] = {200,180, 160, 154, 134, 125, 116, 107, 98, 129,100,500,12,313,314,415,216,317,18,19,20, 21, 22, 23, 24, 25, 26, 27, 28,29,30,31,32};
uint16_t new_id[] = {0,1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20, 21, 22, 23, 24, 25, 26, 27, 28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50};
//uint16_t new_id[] = {30,29, 28, 27, 26, 25,24, 23, 22, 21,20,19,18,17,16,15,14,13,12,11, 10, 9, 8, 7, 6, 5, 4, 3,2,1,0,32};
//uint16_t new_id[] = {10,11, 12, 13, 14, 15, 16, 17, 18, 19,20,21,22,23,24,25,26,27,28,29,30, 31, 32, 33, 34, 35, 36, 37, 38,39,40,41,42};
//uint16_t new_id[] = {0,0, 0, 0, 0, 0, 0, 0, 0, 0,0,11,12,13,14,15,16,17,18,19,20, 21, 22, 23, 24, 25, 26, 27, 28,29,30,31,32};
//uint16_t new_id[] = {0x1fc, 2, 0x1A0, 0x1f3, 0xff, 0x111, 0x16e, 0x191, 0, 0x1e3, 22, 23, 24, 25, 26, 27, 28,70, 71, 72, 73, 126, 127,128, 129, 130, 131, 132, 133, 134};
//uint16_t new_id[200] = {20, 21, 22, 23, 24, 25, 26, 27, 28,70, 71, 72, 73, 74, 75, 76, 77, 78};
//uint16_t new_id[] = {70, 71, 72, 73, 74, 75, 76, 77, 78};
//uint16_t new_id[] = {90, 91, 92, 93, 94, 95, 96, 97, 98};
//uint16_t new_id[] = {119, 120, 121, 122, 123, 124, 125, 126, 127,128, 129, 130, 131, 132, 133, 134, 135, 136};
//uint16_t new_id[200] = {128, 129, 130, 131, 132, 133, 134, 135, 136};
//uint16_t new_id[] = {247, 248, 249, 250, 251, 252, 253, 254, 255};
//uint16_t new_id[] = {300, 301, 302, 303, 304, 305, 306, 307, 308,503, 504, 505, 506, 507, 508, 509, 510, 511};
//uint16_t new_id[] = {503, 504, 505, 506, 507, 508, 509, 510, 511};
//uint16_t new_id[] = { 8, 7, 6, 5, 4, 3, 2, 1, 0};
//uint16_t new_id[] = {511, 510, 509, 508, 507, 506, 505, 504, 503};
//#define LS9073_RESET_ID_PARAMETER1  (LS9073_LIGHT_INHERENT_ID >> 1) & 0X3F00 | 0x4000
//#define LS9073_RESET_ID_PARAMETER2  (LS9073_LIGHT_INHERENT_ID << 11) & 0X3F00 | 0x4000 | (LS9073_LIGHT_RID << 2)

//pwm
void LS9073_send_pwm_delay(uint16_t cnt)
{
	while(cnt--){
		LS9073_PWM_free();
	}
}
//重置id
void LS9073_send_fun_reset_id(void)
{
//	delay_times(1000000);
//	delay_times(1000000);
#if (LS9073_PARAMETER_ID_M==2)|(LS9073_PARAMETER_ID_M==1)
    for (uint16_t i = 0; i < 16; i++) { //LS9073_LIGHT_NUMBER
        uint16_t reset_id_parameter1 = (((whole_id[i]&0x7fff) >> 1) & 0X3FFF) | 0x4000;
        uint16_t reset_id_parameter2 = (((whole_id[i]&0x7fff) << 11) & 0X3F00) | 0x4000 | (new_id[i] << 2);
        LS9073_send_data_packet(LS9073_RESET_ID_COMMAND);
        LS9073_send_data_packet(reset_id_parameter1);
        LS9073_send_data_packet(reset_id_parameter2);
    }
#elif LS9073_PARAMETER_ID_M==0
		uint16_t reset_id_parameter1 = (((whole_id[0]&0x7fff) >> 1) & 0X3FFF) | 0x4000;
        uint16_t reset_id_parameter2 = (((whole_id[0]&0x7fff) << 11) & 0X3F00) | 0x4000 | (new_id[0] << 2);
        LS9073_send_data_packet(LS9073_RESET_ID_COMMAND);
        LS9073_send_data_packet(reset_id_parameter1);
        LS9073_send_data_packet(reset_id_parameter2);
#elif LS9073_PARAMETER_ID_M==3
		for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i++) {
		uint16_t reset_id_parameter = ((whole_id[i]&0xf8000) >> 13) |LS9073_RESET_ID_COMMAND;
        uint16_t reset_id_parameter1 = (((whole_id[i]&0x7fff) >> 1) & 0X3FFF) | 0x4000;
        uint16_t reset_id_parameter2 = (((whole_id[i]&0x7fff) << 11) & 0X3F00) | 0x4000 | (new_id[i] << 2);
        LS9073_send_data_packet(reset_id_parameter);
        LS9073_send_data_packet(reset_id_parameter1);
        LS9073_send_data_packet(reset_id_parameter2);
    }
#endif
}
//每个循环重置1颗灯的id
void LS9073_send_fun_reset_one_id(void)
{
    static uint16_t i = 0;
    uint16_t reset_id_parameter1 = ((inherent_id[i] >> 1) & 0X3FFF) | 0x4000;
    uint16_t reset_id_parameter2 = ((inherent_id[i] << 11) & 0X3F00) | 0x4000 | (new_id[i] << 2);
    LS9073_send_data_packet(LS9073_RESET_ID_COMMAND);
    LS9073_send_data_packet(reset_id_parameter1);
    LS9073_send_data_packet(reset_id_parameter2);
	i++;
    if (i >= 9) {
        i = 0;
    }
}
//根据参数重置1颗灯的id
void LS9073_send_fun_reset_known_id(uint32_t known_id,uint16_t n_id)
{
#if (LS9073_PARAMETER_ID_M==2)|(LS9073_PARAMETER_ID_M==1)
        uint16_t reset_id_parameter1 = (((known_id&0x7fff) >> 1) & 0X3FFF) | 0x4000;
        uint16_t reset_id_parameter2 = (((known_id&0x7fff) << 11) & 0X3F00) | 0x4000 | (n_id << 2);
        LS9073_send_data_packet(LS9073_RESET_ID_COMMAND);
        LS9073_send_data_packet(reset_id_parameter1);
        LS9073_send_data_packet(reset_id_parameter2);
#elif LS9073_PARAMETER_ID_M==0
		uint16_t reset_id_parameter1 = (((known_id&0x7fff) >> 1) & 0X3FFF) | 0x4000;
        uint16_t reset_id_parameter2 = (((known_id&0x7fff) << 11) & 0X3F00) | 0x4000 | (n_id << 2);
        LS9073_send_data_packet(LS9073_RESET_ID_COMMAND);
		LS9073_send_data_packet(reset_id_parameter1);// 不发也不可以
        LS9073_send_data_packet(reset_id_parameter2);// 不发也不可以
#elif LS9073_PARAMETER_ID_M==3
		uint16_t reset_id_parameter = ((known_id&0xf8000) >> 13) |LS9073_RESET_ID_COMMAND;
        uint16_t reset_id_parameter1 = (((known_id&0x7fff) >> 1) & 0X3FFF) | 0x4000;
        uint16_t reset_id_parameter2 = (((known_id&0x7fff) << 11) & 0X3F00) | 0x4000 | (n_id << 2);
        LS9073_send_data_packet(reset_id_parameter);
        LS9073_send_data_packet(reset_id_parameter1);
        LS9073_send_data_packet(reset_id_parameter2);
#endif

}

//读取1颗灯已知的id
void LS9073_send_fun_read_id(void)
{
    LS9073_send_data_packet(LS9073_READ_ID_COMMAND);
}
//加载id命令
void LS9073_send_fun_loading_id(void)
{
    LS9073_send_data_packet(LS9073_LOADING_ID_COMMAND);
		delay_times(400);
}
void LS9073_send_fun_serial_vgs_get(uint16_t addr, uint16_t *data)
{
	*data = 0; //先把该地址里的数据清除
    uint8_t vgs1 = LS9073_vgs[addr] & 0x0f;
    uint8_t vgs2 = LS9073_vgs[addr+1] & 0x0f;
    uint8_t vgs3 = LS9073_vgs[addr+2] & 0x0f;

    *data |= ((uint16_t)vgs1 << 10);
    *data |= ((uint16_t)vgs2 << 6);
	*data |= ((uint16_t)vgs3 << 2);
    *data &= 0x3FFF;
    *data |= 0x4000;
}
//串配VGS命令
void LS9073_send_fun_serial_vgs(void)
{
    uint16_t parameter = 0;
    LS9073_send_data_packet(LS9073_SERIAL_VGS_COMMAND);
	//参数只有一个包，一个包里包含3个灯
	for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i += 3) {
		LS9073_send_fun_serial_vgs_get(i, &parameter);
		LS9073_send_data_packet(parameter);
	}
}
//e-fuse指令
//void LS9073_send_fun_e_fuse(uint16_t OTP,uint16_t P_time)
//{
//	OTP = (OTP&0x7800) >> 11;    //0111 1000 0000 0000
//	P_time = (P_time&0x007f);// 0000 0000 0111 1111
//	uint16_t parameter = 0;
//	parameter = parameter | (OTP << 9);
//	parameter = parameter | (P_time << 2);
//	parameter &= 0x3FFF;
//    parameter |= 0x4000;
//	LS9073_send_data_packet(LS9073_SERIAL_EFUSE_COMMAND);
//	LS9073_send_data_packet(parameter);
//
//}

//全收参数 烧录准备
void LS9073_send_fun_full_receipt_flexible(uint16_t Ep_oe,uint16_t mode,uint16_t VGS,uint16_t Curr_w,uint16_t Vgs_en,uint16_t a_load_en,uint16_t color_m,uint16_t id_m)
{
    uint16_t parameter = 0;
	uint16_t parameter1 = 0;
	uint16_t parameter2 = 0;
//	uint16_t Ep_oe = 0;//16位
//	uint16_t mode = 0;//1位
//	uint16_t VGS = 0;//4位
//	uint16_t Curr_w = 0;//3位
//	uint16_t Vgs_en = 0;//1位
//	uint16_t a_load_en = 0;//1位
//	uint16_t color_m = 0;//2位
	parameter = parameter | (LS9073_FULL_RECEIPT_COMMAND_FLEXIBLE );//命令
	parameter = parameter | (Vgs_en << 2); //    0000 0000 0000 0x00
	parameter = parameter | (a_load_en << 3);//  0000 0000 0000 x000
	parameter = parameter | (color_m << 4);//  0000 0000 0000 x000
	parameter = parameter | (id_m << 6);

    parameter1 = parameter1 | (Curr_w << 6); //  0100 000x xx10 1000
	parameter1 = parameter1 | (VGS << 9);
	parameter1 = parameter1 | (mode << 13);
	parameter1 = parameter1 | (Ep_oe >> 10);
	parameter1 &= 0x3FFF;
    parameter1 |= 0x4000;

	parameter2 = parameter2 | (Ep_oe << 2);
	parameter2 &= 0x3FFF;
    parameter2 |= 0x4000;
	LS9073_send_data_packet(parameter);
    LS9073_send_data_packet(parameter1);
	LS9073_send_data_packet(parameter2);
}
//全收参数
void LS9073_send_fun_full_receipt(void)
{
    uint16_t parameter = 0;

    if (LS9073_COLORS_BIT_LENGTH == 3) { //调参数 mood
        parameter = 0x4E28;    //  0100 1110 0010 1000
    } else {                   //     v  gs
        parameter = 0x6E28;    //  0110 1110 0010 1000
    }

    parameter = parameter | (LS9073_CURR_W_BIT_LENGTH << 6); //  0100 000x xx10 1000

    LS9073_send_data_packet(LS9073_FULL_RECEIPT_COMMAND);
    LS9073_send_data_packet(parameter);
    uint16_t parameter1 = 0;
    parameter1 = 0X6AA8;//0110 1010 1010 1000
    LS9073_send_data_packet(parameter1);
}
//读取点亮指令
void LS9073_read_id_light(uint16_t id)
{
	uint16_t parameter = 0;
	parameter = parameter | (LS9073_READ_ID_LIGHT_COMMAND);
	parameter = parameter | (id << 2);

	LS9073_send_data_packet(parameter);
}
//把色深888转化为666
void LS9073_color_length_swap666(uint16_t addr, uint16_t *data, uint16_t *data1, uint16_t *data2)
{
#if LS9073_COLORS_BIT_LENGTH == 3
    *data = 0; *data1 = 0; *data2 = 0; //先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0xfc;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0xfc;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0xfc;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2);
    *data &= 0x3FFF;
    *data |= 0x4000;
    *data1 |= ((uint16_t)color3 << 6);
    if (addr + 1 != LS9073_LIGHT_NUMBER) {
        uint8_t color4 = LS9073_color[addr + 1].rgb.r & 0xfc;
        uint8_t color5 = LS9073_color[addr + 1].rgb.g & 0xfc;
        uint8_t color6 = LS9073_color[addr + 1].rgb.b & 0xfc;
        *data1 |= ((uint16_t)color4);
        *data2 |= ((uint16_t)color5 << 6);
        *data2 |= ((uint16_t)color6);
        *data2 &= 0x3FFF;
        *data2 |= 0x4000;
    }
    *data1 &= 0x3FFF;
    *data1 |= 0x4000;
#elif LS9073_COLORS_BIT_LENGTH == 4
    *data = 0; *data1 = 0; //先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0xfc;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0xfc;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0xfc;
    uint8_t color4 = LS9073_color[addr].rgb.w & 0xfc;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2);
    *data &= 0x3FFF;
    *data |= 0x4000;
    *data1 |= ((uint16_t)color3 << 6);
    *data1 |= ((uint16_t)color4);
    *data1 &= 0x3FFF;
    *data1 |= 0x4000;
#endif
}
//把色深888转化为111
void LS9073_color_length_swap111(uint16_t addr, uint16_t *data)
{
#if LS9073_COLORS_BIT_LENGTH == 3
    *data = 0;//先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0x80;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0x80;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0x80;
    uint8_t color4 = LS9073_color[addr + 1].rgb.r & 0x80;
    uint8_t color5 = LS9073_color[addr + 1].rgb.g & 0x80;
    uint8_t color6 = LS9073_color[addr + 1].rgb.b & 0x80;
    uint8_t color7 = LS9073_color[addr + 2].rgb.r & 0x80;
    uint8_t color8 = LS9073_color[addr + 2].rgb.g & 0x80;
    uint8_t color9 = LS9073_color[addr + 2].rgb.b & 0x80;
    uint8_t color10 = LS9073_color[addr + 3].rgb.r & 0x80;
    uint8_t color11 = LS9073_color[addr + 3].rgb.g & 0x80;
    uint8_t color12 = LS9073_color[addr + 3].rgb.b & 0x80;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2 << 5);
    *data |= ((uint16_t)color3 << 4);
    *data |= ((uint16_t)color4 << 3);
    *data |= ((uint16_t)color5 << 2);
    *data |= ((uint16_t)color6 << 1);
    *data |= ((uint16_t)color7);
    *data |= ((uint16_t)color8 >> 1);
    *data |= ((uint16_t)color9 >> 2);
    *data |= ((uint16_t)color10 >> 3);
    *data |= ((uint16_t)color11 >> 4);
    *data |= ((uint16_t)color12 >> 5);
    *data &= 0x3FFF;
    *data |= 0x4000;
#elif LS9073_COLORS_BIT_LENGTH == 4
    *data = 0;//先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0x80;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0x80;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0x80;
    uint8_t color4 = LS9073_color[addr].rgb.w & 0x80;
    uint8_t color5 = LS9073_color[addr + 1].rgb.r & 0x80;
    uint8_t color6 = LS9073_color[addr + 1].rgb.g & 0x80;
    uint8_t color7 = LS9073_color[addr + 1].rgb.b & 0x80;
    uint8_t color8 = LS9073_color[addr + 1].rgb.w & 0x80;
    uint8_t color9 = LS9073_color[addr + 2].rgb.r & 0x80;
    uint8_t color10 = LS9073_color[addr + 2].rgb.g & 0x80;
    uint8_t color11 = LS9073_color[addr + 2].rgb.b & 0x80;
    uint8_t color12 = LS9073_color[addr + 2].rgb.w & 0x80;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2 << 5);
    *data |= ((uint16_t)color3 << 4);
    *data |= ((uint16_t)color4 << 3);
    *data |= ((uint16_t)color5 << 2);
    *data |= ((uint16_t)color6 << 1);
    *data |= ((uint16_t)color7);
    *data |= ((uint16_t)color8 >> 1);
    *data |= ((uint16_t)color9 >> 2);
    *data |= ((uint16_t)color10 >> 3);
    *data |= ((uint16_t)color11 >> 4);
    *data |= ((uint16_t)color12 >> 5);
    *data &= 0x3FFF;
    *data |= 0x4000;
#endif

}
//把色深888转化为222
void LS9073_color_length_swap222(uint16_t addr, uint16_t *data, uint16_t *data1)
{
#if LS9073_COLORS_BIT_LENGTH == 3
    *data = 0;//先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0xc0;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0xc0;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0xc0;
    uint8_t color4 = LS9073_color[addr + 1].rgb.r & 0xc0;
    uint8_t color5 = LS9073_color[addr + 1].rgb.g & 0xc0;
    uint8_t color6 = LS9073_color[addr + 1].rgb.b & 0xc0;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2 << 4);
    *data |= ((uint16_t)color3 << 2);
    *data |= ((uint16_t)color4);
    *data |= ((uint16_t)color5 >> 2);
    *data |= ((uint16_t)color6 >> 4);
    *data &= 0x3FFF;
    *data |= 0x4000;
#elif LS9073_COLORS_BIT_LENGTH == 4
    *data = 0; *data1 = 0; //先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0xc0;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0xc0;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0xc0;
    uint8_t color4 = LS9073_color[addr].rgb.w & 0xc0;
    uint8_t color5 = LS9073_color[addr + 1].rgb.r & 0xc0;
    uint8_t color6 = LS9073_color[addr + 1].rgb.g & 0xc0;
    uint8_t color7 = LS9073_color[addr + 1].rgb.b & 0xc0;
    uint8_t color8 = LS9073_color[addr + 1].rgb.w & 0xc0;
    uint8_t color9 = LS9073_color[addr + 2].rgb.r & 0xc0;
    uint8_t color10 = LS9073_color[addr + 2].rgb.g & 0xc0;
    uint8_t color11 = LS9073_color[addr + 2].rgb.b & 0xc0;
    uint8_t color12 = LS9073_color[addr + 2].rgb.w & 0xc0;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2 << 4);
    *data |= ((uint16_t)color3 << 2);
    *data |= ((uint16_t)color4);
    *data |= ((uint16_t)color5 >> 2);
    *data |= ((uint16_t)color6 >> 4);
    *data1 |= ((uint16_t)color7 << 6);
    *data1 |= ((uint16_t)color8 << 4);
    *data1 |= ((uint16_t)color9 << 2);
    *data1 |= ((uint16_t)color10);
    *data1 |= ((uint16_t)color11 >> 2);
    *data1 |= ((uint16_t)color12 >> 4);
    *data &= 0x3FFF; *data |= 0x4000; *data1 &= 0x3FFF; *data1 |= 0x4000;
#endif
}
//把色深888转化为333
void LS9073_color_length_swap333(uint16_t addr, uint16_t *data, uint16_t *data1, uint16_t *data2)
{
#if LS9073_COLORS_BIT_LENGTH == 3
    *data = 0; *data1 = 0; *data2 = 0; //先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0xe0;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0xe0;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0xe0;
    uint8_t color4 = LS9073_color[addr + 1].rgb.r & 0xe0;
    uint8_t color5 = LS9073_color[addr + 1].rgb.g & 0xe0;
    uint8_t color6 = LS9073_color[addr + 1].rgb.b & 0xe0;
    uint8_t color7 = LS9073_color[addr + 2].rgb.r & 0xe0;
    uint8_t color8 = LS9073_color[addr + 2].rgb.g & 0xe0;
    uint8_t color9 = LS9073_color[addr + 2].rgb.b & 0xe0;
    uint8_t color10 = LS9073_color[addr + 3].rgb.r & 0xe0;
    uint8_t color11 = LS9073_color[addr + 3].rgb.g & 0xe0;
    uint8_t color12 = LS9073_color[addr + 3].rgb.b & 0xe0;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2 << 3);
    *data |= ((uint16_t)color3);
    *data |= ((uint16_t)color4 >> 3);
    *data1 |= ((uint16_t)color5 << 6);
    *data1 |= ((uint16_t)color6 << 3);
    *data1 |= ((uint16_t)color7);
    *data1 |= ((uint16_t)color8 >> 3);
    *data2 |= ((uint16_t)color9 << 6);
    *data2 |= ((uint16_t)color10 << 3);
    *data2 |= ((uint16_t)color11);
    *data2 |= ((uint16_t)color12 >> 3);
    *data &= 0x3FFF;
    *data |= 0x4000;
    *data1 &= 0x3FFF;
    *data1 |= 0x4000;
    *data2 &= 0x3FFF;
    *data2 |= 0x4000;
#elif LS9073_COLORS_BIT_LENGTH == 4
    *data = 0;//先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0xe0;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0xe0;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0xe0;
    uint8_t color4 = LS9073_color[addr].rgb.w & 0xe0;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2 << 3);
    *data |= ((uint16_t)color3);
    *data |= ((uint16_t)color4 >> 3);
    *data &= 0x3FFF;
    *data |= 0x4000;
#endif
}
//把色深888转化为444
void LS9073_color_length_swap444(uint16_t addr, uint16_t *data, uint16_t *data1, uint16_t *data2, uint16_t *data3)
{
#if LS9073_COLORS_BIT_LENGTH == 3
    *data = 0;//先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0xf0;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0xf0;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0xf0;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2 << 2);
    *data |= ((uint16_t)color3 >> 2);
    *data &= 0x3FFF;
    *data |= 0x4000;
#elif LS9073_COLORS_BIT_LENGTH == 4
    *data = 0; *data1 = 0; *data2 = 0; //先把该地址里的数据清除
    uint8_t color1 = LS9073_color[addr].rgb.r & 0xf0;
    uint8_t color2 = LS9073_color[addr].rgb.g & 0xf0;
    uint8_t color3 = LS9073_color[addr].rgb.b & 0xf0;
    uint8_t color4 = LS9073_color[addr].rgb.w & 0xf0;
    uint8_t color5 = LS9073_color[addr + 1].rgb.r & 0xf0;
    uint8_t color6 = LS9073_color[addr + 1].rgb.g & 0xf0;
    uint8_t color7 = LS9073_color[addr + 1].rgb.b & 0xf0;
    uint8_t color8 = LS9073_color[addr + 1].rgb.w & 0xf0;
    uint8_t color9 = LS9073_color[addr + 2].rgb.r & 0xf0;
    uint8_t color10 = LS9073_color[addr + 2].rgb.g & 0xf0;
    uint8_t color11 = LS9073_color[addr + 2].rgb.b & 0xf0;
    uint8_t color12 = LS9073_color[addr + 2].rgb.w & 0xf0;
    *data |= ((uint16_t)color1 << 6);
    *data |= ((uint16_t)color2 << 2);
    *data |= ((uint16_t)color3 >> 2);
    *data1 |= ((uint16_t)color4 << 6);
    *data1 |= ((uint16_t)color5 << 2);
    *data1 |= ((uint16_t)color6 >> 2);
    *data2 |= ((uint16_t)color7 << 6);
    *data2 |= ((uint16_t)color8 << 2);
    *data2 |= ((uint16_t)color9 >> 2);
    *data3 |= ((uint16_t)color10 << 6);
    *data3 |= ((uint16_t)color11 << 2);
    *data3 |= ((uint16_t)color12 >> 2);
    *data &= 0x3FFF;
    *data |= 0x4000;
    *data1 &= 0x3FFF;
    *data1 |= 0x4000;
    *data2 &= 0x3FFF;
    *data2 |= 0x4000;
    *data3 &= 0x3FFF;
    *data3 |= 0x4000;
#endif
}
//串收显存
void LS9073_send_fun_serial_receipt_memory(void)
{
#if LS9073_COLORS_BIT_LENGTH == 3
    LS9073_send_data_packet(LS9073_SERIAL_RECEIPT_MEMORY_COMMAND);
    if (LS9073_CURR_W_BIT_LENGTH == 6) {
        //每次发送两个灯  但是要发送3个包
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint16_t parameter2 = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i += 2) {
            LS9073_color_length_swap666(i, &parameter, &parameter1, &parameter2);
            LS9073_send_data_packet(parameter);
            LS9073_send_data_packet(parameter1);
            if (i + 1 != LS9073_LIGHT_NUMBER) {
                LS9073_send_data_packet(parameter2);
            }
        }
    } else if (LS9073_CURR_W_BIT_LENGTH == 1) {
        //参数只有一个包，一个包里包含4个灯（默认mode=rgb）
        uint16_t parameter = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i += 4) {
            LS9073_color_length_swap111(i, &parameter);
            LS9073_send_data_packet(parameter);
        }
    } else if (LS9073_CURR_W_BIT_LENGTH == 2) {
        //参数只有一个包，一个包里包含2个灯（默认mode=rgb）
        uint16_t parameter = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i += 2) {
            LS9073_color_length_swap222(i, &parameter, NULL);
            LS9073_send_data_packet(parameter);
        }
    } else if (LS9073_CURR_W_BIT_LENGTH == 3) {
        //参数有3个包，两个包里面包含4个灯（默认mode=rgb）
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint16_t parameter2 = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i += 4) {
            LS9073_color_length_swap333(i, &parameter, &parameter1, &parameter2);
            LS9073_send_data_packet(parameter);
            LS9073_send_data_packet(parameter1);
            LS9073_send_data_packet(parameter2);
        }
    } else if (LS9073_CURR_W_BIT_LENGTH == 4) {
        //参数有1个包，1个包里面只有1个灯（默认mode=rgb）
        uint16_t parameter = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i++) {
            LS9073_color_length_swap444(i, &parameter, NULL, NULL, NULL);
            LS9073_send_data_packet(parameter);
        }
    }
#elif LS9073_COLORS_BIT_LENGTH == 4
    LS9073_send_data_packet(LS9073_SERIAL_RECEIPT_MEMORY_COMMAND);
    if (LS9073_CURR_W_BIT_LENGTH == 6) {
        //参数有2个包，里面只有1个灯
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i++) {
            LS9073_color_length_swap666(i, &parameter, &parameter1, NULL);
            LS9073_send_data_packet(parameter);
            LS9073_send_data_packet(parameter1);
        }
    } else if (LS9073_CURR_W_BIT_LENGTH == 1) {
        //参数只有一个包，一个包里包含3个灯 mode=rgbw
        uint16_t parameter = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i += 3) {
            LS9073_color_length_swap111(i, &parameter);
            LS9073_send_data_packet(parameter);
        }
    } else if (LS9073_CURR_W_BIT_LENGTH == 2) {
        //参数有2个包，包含3个灯
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i += 3) {
            LS9073_color_length_swap222(i, &parameter, &parameter1);
            LS9073_send_data_packet(parameter);
            LS9073_send_data_packet(parameter1);
        }
    } else if (LS9073_CURR_W_BIT_LENGTH == 3) {
        //参数有1个包，里面包含1个灯
        uint16_t parameter = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i++) {
            LS9073_color_length_swap333(i, &parameter, NULL, NULL);
            LS9073_send_data_packet(parameter);
        }
    } else if (LS9073_CURR_W_BIT_LENGTH == 4) {
        //参数有4个包，里面只有3个灯
        uint16_t parameter = 0, parameter1 = 0, parameter2 = 0, parameter3 = 0;
        for (uint16_t i = 0; i < LS9073_LIGHT_NUMBER; i += 3) {
            LS9073_color_length_swap444(i, &parameter, &parameter1, &parameter2, &parameter3);
            LS9073_send_data_packet(parameter);
            LS9073_send_data_packet(parameter1);
            LS9073_send_data_packet(parameter2);
            LS9073_send_data_packet(parameter3);
        }
    }
#endif
}

//全收显存
void LS9073_send_fun_full_receipt_memory(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
#if LS9073_COLORS_BIT_LENGTH == 3
    if (LS9073_CURR_W_BIT_LENGTH == 6) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = r & 0xfc;
        uint8_t color2 = g & 0xfc;
        uint8_t color3 = b & 0xfc;
        //0000 0000 0000 0000
        parameter |= ((uint16_t)color1 << 6);
        parameter |= (uint16_t)color2;
        parameter1 |= ((uint16_t)color3 << 6);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
    } else if (LS9073_CURR_W_BIT_LENGTH == 1) {
        uint16_t parameter = 0;
        uint8_t color1 = r & 0x80;
        uint8_t color2 = g & 0x80;
        uint8_t color3 = b & 0x80;
        //0000 0000 0000 0000
        parameter |= (uint16_t)color1 << 6;
        parameter |= (uint16_t)color2 << 5;
        parameter |= (uint16_t)color3 << 4;;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
    } else if (LS9073_CURR_W_BIT_LENGTH == 2) {
        uint16_t parameter = 0;
        uint8_t color1 = r & 0xc0;
        uint8_t color2 = g & 0xc0;
        uint8_t color3 = b & 0xc0;
        //0000 0000 0000 0000
        parameter |= (uint16_t)color1 << 6;
        parameter |= (uint16_t)color2 << 4;
        parameter |= (uint16_t)color3 << 2;;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
    } else if (LS9073_CURR_W_BIT_LENGTH == 3) {
        uint16_t parameter = 0;
        uint8_t color1 = r & 0xe0;
        uint8_t color2 = g & 0xe0;
        uint8_t color3 = b & 0xe0;
        //0000 0000 0000 0000
        parameter |= (uint16_t)color1 << 6;
        parameter |= (uint16_t)color2 << 3;
        parameter |= (uint16_t)color3;;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
    } else if (LS9073_CURR_W_BIT_LENGTH == 4) {
        uint16_t parameter = 0;
        uint8_t color1 = r & 0xf0;
        uint8_t color2 = g & 0xf0;
        uint8_t color3 = b & 0xf0;
        //0000 0000 0000 0000
        parameter |= (uint16_t)color1 << 6;
        parameter |= (uint16_t)color2 << 2;
        parameter |= (uint16_t)color3 >> 2;;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
    }
#elif LS9073_COLORS_BIT_LENGTH == 4
    if (LS9073_CURR_W_BIT_LENGTH == 6) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = r & 0xfc;
        uint8_t color2 = g & 0xfc;
        uint8_t color3 = b & 0xfc;
        uint8_t color4 = w & 0xfc;
        //0000 0000 0000 0000
        parameter |= ((uint16_t)color1 << 6);
        parameter |= (uint16_t)color2;
        parameter1 |= ((uint16_t)color3 << 6);
        parameter1 |= (uint16_t)color4;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
    } else if (LS9073_CURR_W_BIT_LENGTH == 1) {
        uint16_t parameter = 0;
        uint8_t color1 = r & 0x80;
        uint8_t color2 = g & 0x80;
        uint8_t color3 = b & 0x80;
        uint8_t color4 = w & 0x80;
        //0000 0000 0000 0000
        parameter |= (uint16_t)color1 << 6;
        parameter |= (uint16_t)color2 << 5;
        parameter |= (uint16_t)color3 << 4;
        parameter |= (uint16_t)color4 << 3;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
    } else if (LS9073_CURR_W_BIT_LENGTH == 2) {
        uint16_t parameter = 0;
        uint8_t color1 = r & 0xc0;
        uint8_t color2 = g & 0xc0;
        uint8_t color3 = b & 0xc0;
        uint8_t color4 = w & 0xc0;
        //0000 0000 0000 0000
        parameter |= (uint16_t)color1 << 6;
        parameter |= (uint16_t)color2 << 4;
        parameter |= (uint16_t)color3 << 2;
        parameter |= (uint16_t)color4 ;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
    } else if (LS9073_CURR_W_BIT_LENGTH == 3) {
        uint16_t parameter = 0;
        uint8_t color1 = r & 0xe0;
        uint8_t color2 = g & 0xe0;
        uint8_t color3 = b & 0xe0;
        uint8_t color4 = w & 0xe0;
        //0000 0000 0000 0000
        parameter |= (uint16_t)color1 << 6;
        parameter |= (uint16_t)color2 << 3;
        parameter |= (uint16_t)color3;
        parameter |= (uint16_t)color4 >> 3;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
    } else if (LS9073_CURR_W_BIT_LENGTH == 4) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = r & 0xf0;
        uint8_t color2 = g & 0xf0;
        uint8_t color3 = b & 0xf0;
        uint8_t color4 = w & 0xf0;
        //0000 0000 0000 0000
        parameter |= (uint16_t)color1 << 6;
        parameter |= (uint16_t)color2 << 2;
        parameter |= (uint16_t)color3 >> 2;
        parameter1 |= (uint16_t)color4 << 6;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FULL_RECEIPT_MEMORY_COMMAND);
        LS9073_send_data_packet(parameter);
		LS9073_send_data_packet(parameter1);
    }
#endif
}
//显示切换
void LS9073_send_fun_display_switch_memory(void)
{
    LS9073_send_data_packet(LS9073_DISPLAY_SWITCH_MEMORY_COMMAND);
}
//定点刷新函数
void LS9073_send_fun_fixed_point_refresh(unsigned short int addr)
{
#if LS9073_COLORS_BIT_LENGTH == 3
    if (LS9073_CURR_W_BIT_LENGTH == 1) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0x80;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0x80;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0x80;
		parameter |= new_id[addr] << 5;
        parameter1 |= ((uint16_t)color1 << 6);
        parameter1 |= ((uint16_t)color2 << 5);
        parameter1 |= ((uint16_t)color3 << 4);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);

    } else if (LS9073_CURR_W_BIT_LENGTH == 2) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0xC0;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0xC0;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0xC0;
		parameter |= new_id[addr] << 5;
        parameter1 |= ((uint16_t)color1 << 6);
        parameter1 |= ((uint16_t)color2 << 4);
        parameter1 |= ((uint16_t)color3 << 2);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
    } else if (LS9073_CURR_W_BIT_LENGTH == 3) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0xe0;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0xe0;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0xe0;
		parameter |= new_id[addr] << 5;
        parameter1 |= ((uint16_t)color1 << 6);
        parameter1 |= ((uint16_t)color2 << 3);
        parameter1 |= ((uint16_t)color3);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
    } else if (LS9073_CURR_W_BIT_LENGTH == 4) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0xf0;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0xf0;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0xf0;
		parameter |= new_id[addr] << 5;
        parameter1 |= ((uint16_t)color1 << 6);
        parameter1 |= ((uint16_t)color2 << 2);
        parameter1 |= ((uint16_t)color3 >> 2);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
    } else if (LS9073_CURR_W_BIT_LENGTH == 6) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint16_t parameter2 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0xfc;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0xfc;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0xfc;
		parameter |= new_id[addr] << 5;
        parameter1 |= (uint16_t)color1 << 6;
        parameter1 |= (uint16_t)color2;
        parameter2 |= (uint16_t)color3 << 6;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        parameter2 &= 0x3FFF;
        parameter2 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
        LS9073_send_data_packet(parameter2);
    }
#elif LS9073_COLORS_BIT_LENGTH == 4
    if (LS9073_CURR_W_BIT_LENGTH == 1) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0x80;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0x80;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0x80;
        uint8_t color4 = LS9073_color[addr].rgb.w & 0x80;
		parameter |= new_id[addr] << 5;
        parameter1 |= ((uint16_t)color1 << 6);
        parameter1 |= ((uint16_t)color2 << 5);
        parameter1 |= ((uint16_t)color3 << 4);
        parameter1 |= ((uint16_t)color4 << 3);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);

    } else if (LS9073_CURR_W_BIT_LENGTH == 2) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0xC0;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0xC0;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0xC0;
        uint8_t color4 = LS9073_color[addr].rgb.w & 0xC0;
		parameter |= new_id[addr] << 5;
        parameter1 |= ((uint16_t)color1 << 6);
        parameter1 |= ((uint16_t)color2 << 4);
        parameter1 |= ((uint16_t)color3 << 2);
        parameter1 |= ((uint16_t)color4);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
    } else if (LS9073_CURR_W_BIT_LENGTH == 3) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0xe0;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0xe0;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0xe0;
        uint8_t color4 = LS9073_color[addr].rgb.w & 0xe0;
		parameter |= new_id[addr] << 5;
        parameter1 |= ((uint16_t)color1 << 6);
        parameter1 |= ((uint16_t)color2 << 3);
        parameter1 |= ((uint16_t)color3);
        parameter1 |= ((uint16_t)color4 >> 3);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
    } else if (LS9073_CURR_W_BIT_LENGTH == 4) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint16_t parameter2 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0xf0;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0xf0;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0xf0;
        uint8_t color4 = LS9073_color[addr].rgb.w & 0xf0;
		parameter |= new_id[addr] << 5;
        parameter1 |= ((uint16_t)color1 << 6);
        parameter1 |= ((uint16_t)color2 << 2);
        parameter1 |= ((uint16_t)color3 >> 2);
        parameter2 |= ((uint16_t)color4 << 6);
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        parameter2 &= 0x3FFF;
        parameter2 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
        LS9073_send_data_packet(parameter2);
    } else if (LS9073_CURR_W_BIT_LENGTH == 6) {
        uint16_t parameter = 0;
        uint16_t parameter1 = 0;
        uint16_t parameter2 = 0;
        uint8_t color1 = LS9073_color[addr].rgb.r & 0xfc;
        uint8_t color2 = LS9073_color[addr].rgb.g & 0xfc;
        uint8_t color3 = LS9073_color[addr].rgb.b & 0xfc;
        uint8_t color4 = LS9073_color[addr].rgb.w & 0xfc;
//		parameter |= new_id[addr] << 5;
		parameter |= addr << 5;
        parameter1 |= (uint16_t)color1 << 6;
        parameter1 |= (uint16_t)color2;
        parameter2 |= (uint16_t)color3 << 6;
        parameter2 |= (uint16_t)color4;
        parameter &= 0x3FFF;
        parameter |= 0x4000;
        parameter1 &= 0x3FFF;
        parameter1 |= 0x4000;
        parameter2 &= 0x3FFF;
        parameter2 |= 0x4000;
        LS9073_send_data_packet(LS9073_FIXED_POINT_REFRESH_COMMAND);
        LS9073_send_data_packet(parameter);
        LS9073_send_data_packet(parameter1);
        LS9073_send_data_packet(parameter2);
    }
#endif
}


////*********LS9073************************************////////
static void strlights_send_bit(uint8_t bitdate, uint8_t bitcnt)
{
    u8 i = 0;
    for (; i < bitcnt; i++) {
        if (bitdate & 0x01) {
            STRLIGHTS_SET_HIGH();
        } else {
            STRLIGHTS_SET_LOW();
        }

        bitdate >>= 1;
    }
}

void strlights_set_color(uint16_t addr, uint8_t *color)
{
    strlights_send_bit(STRLIGHTS_CMD_CODE, 2);

    strlights_send_bit((uint8_t)(addr & 0xff), 8);
#ifdef STRLIGHTS_COLOR_RGBW
    strlights_send_bit((uint8_t)(addr >> 8), 1);

    strlights_send_bit(*color++, 8);
#endif
    strlights_send_bit(*color++, 8);
    strlights_send_bit(*color++, 8);
    strlights_send_bit(*color++, 8);

    STRLIGHTS_SET_STOP();
    key_scan(); // 提高按键检查率
}

void strlights_gpio_init(void)
{
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);

    Gpio_InitIO(STRLIGHTS_DATA_PORT, STRLIGHTS_DATA_PIN, GpioDirOut);
    Gpio_InitIO(STRLIGHTS_CTRL_PORT, STRLIGHTS_CTRL_PIN, GpioDirOut);

    STRLIGHTS_DATA_H();
    //STRLIGHTS_ON();
    STRLIGHTS_OFF();
}


