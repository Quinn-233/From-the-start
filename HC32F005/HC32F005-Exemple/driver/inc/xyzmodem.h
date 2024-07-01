#ifndef __XYZMODEM_H__
#define __XYZMODEM_H__

#include "base_types.h"
#include "stdbool.h"

//用于debug时方便编写代码。完成后注释掉
#define UART_YMODEM_ENABLE 1

#if UART_YMODEM_ENABLE
#define xyzmodem_xmodem 1
#define xyzmodem_ymodem 2
/* Don't define this until the protocol support is in place */
/*#define xyzmodem_zmodem 3 */

#define xyzmodem_uartx UARTCH1

#define xyzmodem_access   -1	// 
#define xyzmodem_noZmodem -2	// 
#define xyzmodem_timeout  -3	// 传输超时未响应
#define xyzmodem_eof      -4	// 传输完成
#define xyzmodem_cancel   -5	// 传输中止
#define xyzmodem_frame    -6	// 帧序号检测出错
#define xyzmodem_cksum    -7	// CRC检测出错
#define xyzmodem_sequence -8	// 

#define xyzmodem_close 1
#define xyzmodem_abort 2

/* Assumption - run xyz->odem protocol over the console port */

/* Values magic to the protocol */
#define SOH 0x01 // 128字节数据帧，协议类型
#define STX 0x02 // 1024字节数据帧，协议类型
#define EOT 0x04 // 结束传输，发送者发送
#define ACK 0x06 // 接收者处理成功回应，发送者发现下一包数据（1024或者128）
//#define BSP 0x08 //
#define NAK 0x15 // 接收者处理失败回应，发送者需要重发此1024或者128数据,或者接收到第一个EOT的回复包
#define CAN 0x18 // 传输终止
//#define EOF 0x1A        /* ^Z for DOS officionados */

#define USE_YMODEM_LENGTH

#define XYZMODEM_CHAR_TIMEOUT           2000  /* 2 seconds */
#define XYZMODEM_MAX_RETRIES            20
#define XYZMODEM_MAX_RETRIES_WITH_CRC   10
#define XYZMODEM_CAN_COUNT              3     /* Wait for 3 CAN before quit */

#define CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT
#define CYGACC_CALL_IF_SET_CONSOLE_COMM(x)

#define diag_vprintf vprintf
#define diag_printf printf
#define diag_vsprintf vsprintf

#define CYGACC_CALL_IF_DELAY_US(x) udelay(x)

#define rec_buff_Len    128
struct connection_info_t {
    char *filename;	// bin文件名
    int mode;		// 传输长度模式
    int chan;		//
#ifdef CYGPKG_REDBOOT_NETWORKING
    struct sockaddr_in *server;
#endif
};

/* Data & state local to the protocol */
struct _xyz {
    int *__chan;
    u8 pkt[1024], *bufp;					// 存放数据的数组、数组地址指针
    u8 blk, cblk, crc1, crc2;				// 帧序号、帧序号补码
    u8 next_blk;    						// 期待的帧序号
    int len, mode, total_retries;			// 数据最大长度、协议数据长度、总重试次数
    int total_soh, total_stx, total_can;	// SOH总数、STX总数、CAN总数
    bool crc_mode, at_eof, tx_ack;			//
#ifdef USE_YMODEM_LENGTH
    u32 file_length, read_length;			//
#endif
};
extern unsigned int ymodem_start_flag;
extern unsigned int ymo_front;
extern unsigned int ymo_rear;
extern unsigned char rec_buff[rec_buff_Len];
extern unsigned char check_buff[20];
extern unsigned char check_index;
extern char test_command[];
extern char del_custom_flash_command[];
extern char mask_command[];
// 进队
u8 enQueue(u8 x);
// 出队
u8 deQueue(u8* value_addr);
u8 serial_getc(void);

void Erase_gif_bmp_flash(void);
int ymodem_download(void);
int ymodem_download_mask(void);
#endif
#endif /* _XYZMODEM_H_ */
