#include "hc32f005.h"
#include "xyzmodem.h"
#include "stddef.h"
//#include "driver_W25LX10.h"
//#include "external_flash.h"

#if UART_YMODEM_ENABLE
static int xyzmodem_stream_open(struct connection_info_t *info, int *err);
static void xyzmodem_stream_close(int *err);
static void xyzmodem_stream_terminate(bool method, int (*getc)(void));
static int xyzmodem_stream_read(char *buf, int size, int *err);

static struct _xyz xyz;
unsigned int ymo_front;
unsigned int ymo_rear;
unsigned int ymodem_start_flag;
// 队列缓存区
__attribute__((aligned(4))) unsigned char rec_buff[rec_buff_Len] = {0};
unsigned char check_buff[20] = {0};
unsigned char check_index = 0;
char test_command[] = {"enable\r\n"};
char del_custom_flash_command[] = {"delcustomflash\r\n"};
char mask_command[] = {"mask download\r\n"};
////extern bulbsShape_ymodem_t bulbsShape_ymodem_buf;
// 初始化队列
void initQueue(void)
{
    ymo_front = 0;
    ymo_rear = 0;
}
// 判断队空 1:队空
u8 isQueueEmpty()
{
    if (ymo_front == ymo_rear) {
        return 1;
    } else {
        return 0;
    }
}

/*************** 进队函数 ***************/
u8 enQueue(u8 x)
{
    // 判断队是否已满
    if ((ymo_rear + 1) % rec_buff_Len == ymo_front) {
        return 0;
    }
    ymo_rear = (ymo_rear + 1) % rec_buff_Len;
    rec_buff[ymo_rear] = x;
    return 1;
}

/*************** 出队函数 ***************/
u8 deQueue(u8* value_addr)
{
    if (ymo_front == ymo_rear) {
        return 0;
    }
    ymo_front = (ymo_front + 1) % rec_buff_Len;
    *value_addr = rec_buff[ymo_front];
    return 1;
}

/*********** CRC校验底层函数 ************
**
**  *src：         待校验数据
**  len：          待校验数据长度
**  polynomial：   生成多项式
**  initial_value：crc初始数据
**  pad：          true/false
****************************************/
u16 crc16_ymodem(const u8 *src, size_t len, u16 polynomial,
                 u16 initial_value, bool pad)
{
    u16 crc = initial_value;
    size_t padding = pad ? sizeof(crc) : 0;  // crc大小（2字节）/ 0
    size_t i, b;

    /* src length + padding (if required) */
    for (i = 0; i < len + padding; i++) {

        for (b = 0; b < 8; b++) {
			// 取最高位（第16位）
            u16 divide = crc & 0x8000UL;

            crc = (crc << 1U);

            /* choose input bytes or implicit trailing zeros */
            if (i < len) {
                crc |= !!(src[i] & (0x80U >> b));
            }

            if (divide != 0U) {
                crc = crc ^ polynomial;
            }
        }
    }
    return crc;
}

/************* CRC校验函数 **************
**
**  *s： 待校验数据
**  len：待校验数据长度
****************************************/
static u16 cyg_crc16(u8 *s, size_t len)
{
    return crc16_ymodem(s, len, 0x1021, 0, true);
}

/***** 判断当前队列里是否有可用数据 *****
**
**  0：无可用数据
**  1：有可用数据
****************************************/
static int serial_tstc(void)
{
    // return uart_ndmairq_get();
    return (!isQueueEmpty());
}

/********** 从队列中获取数据 ***********
**
**  retern一个字节
****************************************/
u8 serial_getc(void)
{
    u8 data = 0;
    deQueue(&data); // 从队列里获取
    return data;
}

/********** 通过串口发送数据 ***********
**
**  发送一个字节
****************************************/
void serial_putc(char data)
{
    if (data == '\n') {
		Uart_SendByte(xyzmodem_uartx,'\r');
    }
	Uart_SendByte(xyzmodem_uartx,(unsigned char)data); // 使用串口直接发送
}

/************** 延时函数 ***************/
extern void delay_ms(u16 time);

void udelay(u16 useconds)
{
    delay_ms((useconds * 7) / 2);
}

/********** 等待数据到来函数 ************
**
**  chan：？
**  *c：  存放队列取出的数据
****************************************/
static int cygacc_comm_if_getc_timeout(char chan, char *c)
{
#define DELAY 20
    unsigned long counter = 0;
    // 队列无可用数据 & 未超时 --> 等待
    while (!serial_tstc() &&
            (counter < XYZMODEM_CHAR_TIMEOUT * 1000 / DELAY)) {
        udelay(DELAY);
        counter++;
    }
    // 队列可用数据 -> 取出
    if (serial_tstc()) {
        *c = serial_getc();
        return 1;
    }

    return 0;
}

/********** 等待数据到来函数 ************
**
**  x：？
**  y：存放队列取出的数据
****************************************/
static void cygacc_comm_if_putc(char x, char y)
{
    serial_putc(y);
}

/********** 判断数据是否为字符 ***********/
static bool is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

/****** 将数据由ASCII转换为十六进制 ******/
static int convert_ascii_to_hex(char c)
{
    int ret = 0;

    if ((c >= '0') && (c <= '9')) {
        ret = (c - '0');
    } else if ((c >= 'a') && (c <= 'f')) {
        ret = (c - 'a' + 0x0a);
    } else if ((c >= 'A') && (c <= 'F')) {
        ret = (c - 'A' + 0x0A);
    }

    return ret;
}

/********* 将大写字符转换为小写 *********/
static char convert_charac_to_lower(char c)
{
    if ((c >= 'A') && (c <= 'Z')) {
        c = (c - 'A') + 'a';
    }

    return c;
}

/************** 解析数据包 *************
**
**  *s：    待查的数据包地址
**  *val：  存放结果的数据包地址
**  **es：  ？
**  *delim：分隔符
****************************************/
bool parse_num(char *s, u32 *val, char **es, char *delim)
{
    bool first = true;
    int radix = 10;
    char c;
    u32 result = 0;
    int digit;
    char *dp;
    // 跳过数据开头的空格
    while (*s == ' ') {
        s++;
    }
    while (*s) {
        // 判断开头数据是不是十六进制数
        if (first && (s[0] == '0') && (convert_charac_to_lower(s[1]) == 'x')) {
            radix = 16;
            s += 2;
        }
        first = false;
        // 数据逐位转换成十六进制数
        c = *s++;
        digit = convert_ascii_to_hex(c);
        // 判断数据是否有效（数据为数字/字母 & 开头数据有效）
        if (is_hex(c) && (digit < radix)) {
            // 小端模式排列结果
            result = (result * radix) + digit;
        } else {
            if (delim != (char *)0) {
                dp = delim;
                // 查找数据包中的分隔符
                while (*dp && (c != *dp)) {
                    dp++;
                }
                // 找到合适的分隔符，结束这一帧数据包的操作
                if (*dp) {
                    break;
                }
            }
            // 错误的数据格式
            return false;
        }
    }

    *val = result;
    if (es != (char **)0) {
        *es = s;
    }

    return true;
}

/************* 等待线路空闲 *************/
static void xyzmodem_flush(void)
{
    int res;
    char c;

    while (true) {
        res = cygacc_comm_if_getc_timeout(*xyz.__chan, &c);
        if (!res) {
            return;
        }
    }
}

/************* 等待线路空闲 *************/
static int xyzmodem_get_hdr(void)
{
    char c;
    int res;
    bool hdr_found = false;
    int i, can_total, hdr_chars;
    u16 cksum;

    /* Find the start of a header */
    can_total = 0;
    hdr_chars = 0;

    if (xyz.tx_ack) {
		// 发送 接收成功应答
        cygacc_comm_if_putc(*xyz.__chan, ACK);
        xyz.tx_ack = false;
    }

    while (!hdr_found) {
        res = cygacc_comm_if_getc_timeout(*xyz.__chan, &c);
		// 正在取数据
        if (res) {
            hdr_chars++;
            switch (c) {
                // 准备128字节数据包
                case SOH:
                    xyz.total_soh++;
                // 准备1024字节数据包
                case STX:
                    if (c == STX) {
                        xyz.total_stx++;
                    }
                    hdr_found = true;
                    break;
                // 传输终止
                case CAN:
                    xyz.total_can++;
                    if (++can_total == XYZMODEM_CAN_COUNT) {
                        return xyzmodem_cancel;
                    }
                    break;
                // 传输完成
                case EOT:
                    /* EOT only supported if no noise */
                    if (hdr_chars == 1) {
                        cygacc_comm_if_putc(*xyz.__chan, ACK);
                        return xyzmodem_eof;
                    }
                    break;

                default:
                    /* Ignore, waiting for start of header */
                    break;
            }
        } else {
            /* Data stream timed out */
            xyzmodem_flush();   /* Toss any current input */
            CYGACC_CALL_IF_DELAY_US(250000);
            return xyzmodem_timeout;
        }
    }

    /* Header found, now read the data */
    res = cygacc_comm_if_getc_timeout(*xyz.__chan, (char *)&xyz.blk);
    if (!res) {
        return xyzmodem_timeout;
    }

    res = cygacc_comm_if_getc_timeout(*xyz.__chan, (char *)&xyz.cblk);
    if (!res) {
        return xyzmodem_timeout;
    }

    xyz.len = (c == SOH) ? 128 : 1024;
    xyz.bufp = xyz.pkt;
    for (i = 0; i < xyz.len; i++) {
        res = cygacc_comm_if_getc_timeout(*xyz.__chan, &c);
        if (res) {
            xyz.pkt[i] = c;
        } else {
            return xyzmodem_timeout;
        }
    }

    res = cygacc_comm_if_getc_timeout(*xyz.__chan, (char *) &xyz.crc1);
    if (!res) {
        return xyzmodem_timeout;
    }

    if (xyz.crc_mode) {
        res = cygacc_comm_if_getc_timeout(*xyz.__chan,
                                          (char *)&xyz.crc2);
        if (!res) {
            return xyzmodem_timeout;
        }
    }

    /* Validate the message */
    if ((xyz.blk ^ xyz.cblk) != (u8)0xFF) {

        // uart_test_data3 = (u8)ymo_front;
        // uart_test_data4 = (u8)ymo_rear;
        // uart_test_data1 = xyz.blk;
        // uart_test_data2 = xyz.cblk;
        // gpio_write(GPIO_PC2, 1);
        // gpio_write(GPIO_PC2, 0);
        // for(unsigned char i=0;i<22;i++)
        //       {
        //           uart_ndma_send_byte(rec_buff[i]);
        //       }
        xyzmodem_flush();
        return xyzmodem_frame;
    }

    /* Verify checksum/CRC */
    if (xyz.crc_mode) {
        cksum = cyg_crc16(xyz.pkt, xyz.len); // 每次对收到的包进行 crc 校验
        if (cksum != ((xyz.crc1 << 8) | xyz.crc2)) {
            return xyzmodem_cksum;
        }
    } else {
        cksum = 0;
        for (i = 0; i < xyz.len; i++) {
            cksum += xyz.pkt[i];
        }

        if (xyz.crc1 != (cksum & 0xFF)) {
            return xyzmodem_cksum;
        }
    }

    /* If we get here, the message passes [structural] muster */
    return 0;
}

static int xyzmodem_stream_open(struct connection_info_t *info, int *err)
{
    int stat = 0;
    int retries = XYZMODEM_MAX_RETRIES;
    int crc_retries = XYZMODEM_MAX_RETRIES_WITH_CRC;
    int dummy = 0;

#ifdef xyzmodem_zmodem
    if (info->mode == xyzmodem_zmodem) {
        *err = xyzmodem_noZmodem;
        return -1;
    }
#endif

    /* TODO: CHECK ! */
    xyz.__chan = &dummy;
    xyz.len = 0;
    xyz.crc_mode = true;
    xyz.at_eof = false;
    xyz.tx_ack = false;
    xyz.mode = info->mode;
    xyz.total_retries = 0;
    xyz.total_soh = 0;
    xyz.total_stx = 0;
    xyz.total_can = 0;
#ifdef USE_YMODEM_LENGTH
    xyz.read_length = 0;
    xyz.file_length = 0;
#endif

    cygacc_comm_if_putc(*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));

    if (xyz.mode == xyzmodem_xmodem) {
        /* X-modem doesn't have an information header - exit here */
        xyz.next_blk = 1;
        return 0;
    }

    while (retries-- > 0) {
        stat = xyzmodem_get_hdr();
        if (stat == 0) {
            /* Y-modem file information header */
            if (xyz.blk == 0) {
#ifdef USE_YMODEM_LENGTH
                /* skip filename */
                while (*xyz.bufp++) {
                }

                /* get the length */
                parse_num((char *)xyz.bufp,
                          &xyz.file_length, NULL, " ");
#endif
                /* The rest of the file name data
                 * block quietly discarded
                 */
                xyz.tx_ack = true;
            }
            xyz.next_blk = 1;
            xyz.len = 0;
            return 0;
        } else if (stat == xyzmodem_timeout) {
            if (--crc_retries <= 0) {
                xyz.crc_mode = false;
            }

            /* Extra delay for startup */
            CYGACC_CALL_IF_DELAY_US(5 * 100000);
            cygacc_comm_if_putc(*xyz.__chan,
                                (xyz.crc_mode ? 'C' : NAK));
            xyz.total_retries++;
        }
        if (stat == xyzmodem_cancel) {
            break;
        }
    }
    *err = stat;

    return -1;
}


#define BREAK           1
#define CONTINUE        2

static int xyzmodem_retry(int *stat)
{
    *stat = xyzmodem_get_hdr();
    if (*stat == 0) {
        if (xyz.blk == xyz.next_blk) {
            xyz.tx_ack = true;
            xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

            /* Data blocks can be padded with ^Z (EOF) characters */
            /* This code tries to detect and remove them */
            if ((xyz.mode == xyzmodem_xmodem ||
                    xyz.file_length == 0) &&
                    (xyz.bufp[xyz.len - 1] == EOF) &&
                    (xyz.bufp[xyz.len - 2] == EOF) &&
                    (xyz.bufp[xyz.len - 3] == EOF)) {
                while (xyz.len &&
                        (xyz.bufp[xyz.len - 1] == EOF)) {
                    xyz.len--;
                }
            }

#ifdef USE_YMODEM_LENGTH
            /*
             * See if accumulated length exceeds that of the file.
             * If so, reduce size (i.e., cut out pad bytes)
             * Only do this for Y-modem (and Z-modem should it ever
             * be supported since it can fall back to Y-modem mode).
             */
            if (xyz.mode != xyzmodem_xmodem &&
                    xyz.file_length != 0) {
                xyz.read_length += xyz.len;
                if (xyz.read_length > xyz.file_length) {
                    xyz.len -= xyz.read_length -
                               xyz.file_length;
                }
            }
#endif
            return BREAK;
        } else if (xyz.blk == ((xyz.next_blk - 1) & 0xFF)) {
            /* Just re-ACK this so sender will get on with it */
            cygacc_comm_if_putc(*xyz.__chan, ACK);
            return CONTINUE;    /* Need new header */
        }

        *stat = xyzmodem_sequence;
    }

    if (*stat == xyzmodem_cancel) {
        return BREAK;
    }

    if (*stat == xyzmodem_eof) {
        cygacc_comm_if_putc(*xyz.__chan, ACK);
        if (xyz.mode == xyzmodem_ymodem) {
            cygacc_comm_if_putc(*xyz.__chan,
                                (xyz.crc_mode ? 'C' : NAK));
            xyz.total_retries++;
            *stat = xyzmodem_get_hdr();
            cygacc_comm_if_putc(*xyz.__chan, ACK);
        }
        xyz.at_eof = true;
        return BREAK;
    }
    cygacc_comm_if_putc(*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
    xyz.total_retries++;

    return 0;
}

static int xyzmodem_stream_read(char *buf, int size, int *err)
{
    int stat, total, len;
    int retries;
    int ret;

    total = 0;
    stat = xyzmodem_cancel;
    /* Try and get 'size' bytes into the buffer */
    while (!xyz.at_eof && (size > 0)) {
        if (xyz.len == 0) {
            retries = XYZMODEM_MAX_RETRIES;
            while (retries-- > 0) {
                ret = xyzmodem_retry(&stat);
                if (ret == BREAK) {
                    break;
                } else if (ret == CONTINUE) {
                    continue;
                }
            }

            // if (stat < 0) {
            if (stat < 0 || xyz.at_eof) {
                *err = stat;
                xyz.len = -1;
                return total;
            }
        }
        /* Don't "read" data from the EOF protocol package */
        if (!xyz.at_eof) {
            len = xyz.len;
            if (size < len) {
                len = size;
            }
            memcpy(buf, xyz.bufp, len);
            size -= len;
            buf += len;
            total += len;
            xyz.len -= len;
            xyz.bufp += len;
        }
    }

    return total;
}

static void xyzmodem_stream_close(int *err)
{
#if 0
    printk("mode %s, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
           xyz.crc_mode ? "CRC" : "Cksum",
           xyz.total_soh, xyz.total_stx,
           xyz.total_can, xyz.total_retries);
#endif
}

/* Need to be able to clean out the input buffer, so have to take the */
static void xyzmodem_stream_terminate(bool abort, int (*serial_getc)(void))
{
    int c;

    /*
     * Consume any trailing crap left in the inbuffer from
     * previous received blocks. Since very few files are an exact
     * multiple of the transfer block size, there will almost always
     * be some gunk here. If we don't eat it now, RedBoot will think
     * the user typed it.
     */
    while ((c = (*serial_getc)()) > -1) {
    }
    /*
     * Make a small delay to give terminal programs like minicom
     * time to get control again after their file transfer program
     * exits.
     */
    CYGACC_CALL_IF_DELAY_US(250000);
}


static int getcxmodem(void)
{
    if (serial_tstc()) {
        return serial_getc();
    }
    return -1;
}

#if EXTERNAL_FLASH_ENABLE
// 擦除目标地址
void Erase_gif_bmp_flash(void)
{
    u32 erase_addr = 0;
    for (u32 i = 0; i < (FLASH_FACTORY_FIRING_FILE_SIZE_KB / 4); i++) { // 擦除存储出场烧录文件的空间
        erase_addr = FLASH_FACTORY_FIRING_FILE_START_ADDR + i * 0x1000;
        FLASH_SectorErase(erase_addr); // 扇区4K擦除
    }
}
#endif

// 擦除存储 mask id 的扇区
void Erase_mask_flash(void)
{
////    flash_erase_sector(FLASH_ADR_LTSTR_MASK_ID);
}

#if EXTERNAL_FLASH_ENABLE
int ymodem_download(void)
{
    u32 offset = FLASH_FACTORY_FIRING_FILE_START_ADDR;
    // u32 offset = 0x3800;
    int err = 0;
    int res;
    struct connection_info_t info;
    // 初始化循环队列
    initQueue();
    // 清空循环队列缓冲区
    memset(rec_buff,0x00,rec_buff_Len);
    // 清除 ymodem 接收缓存区
    memset(bulbsShape_ymodem_buf.ymodem_switch_buf, 0, sizeof(bulbsShape_ymodem_t));

    info.mode = xyzmodem_ymodem;
    // 打开ymodem开关，主动向上位机发送交互指令 ，收到上位机回复后方可继续运行，如果超时则会报错进入close
    res = xyzmodem_stream_open(&info, &err);
    if (res) {
        // printk("error open ymodem\n");
        goto close;
    }
    // 擦除目标 flash
    Erase_gif_bmp_flash();
    /* when stat >= 0 */
    do {
        if ((res = xyzmodem_stream_read((char*)bulbsShape_ymodem_buf.ymodem_switch_buf, sizeof(xyz.pkt), &err)) > 0) {
            // if (image_index == 0) {
            //  ota_flash_image_write(OTA_FLASH_SOC, offset, bulbsShape_ymodem_buf.ymodem_switch_buf, res);
            // } else {
            //  ota_flash_image_write(OTA_FLASH_SPI, offset, bulbsShape_ymodem_buf.ymodem_switch_buf, res);
            // }
            FLASH_BufferWrite(bulbsShape_ymodem_buf.ymodem_switch_buf, offset, (unsigned short)res);
            offset += res;
        }
    } while (xyz.len != -1);

    xyzmodem_stream_close(&err);
    xyzmodem_stream_terminate(false, &getcxmodem);

    // uart_irq_rx_enable(uart);
    // printk("Total Size = %d Bytes\n", offset);

    return offset;

close:
    xyzmodem_stream_close(&err);
    xyzmodem_stream_terminate(false, &getcxmodem);
    // uart_irq_rx_enable(uart);

    return -1;
}
#endif
extern u8 ltstr_write_flash(unsigned long addr, unsigned long len, unsigned char *buf);
int ymodem_download_mask(void)
{
////    u32 offset = FLASH_ADR_LTSTR_MASK_ID;
    // u32 offset = 0x3800;
    int err = 0;
    int res;
    struct connection_info_t info;
    // 初始化循环队列
    initQueue();
    // 清空循环队列缓冲区
    memset(rec_buff,0x00,rec_buff_Len);
    // 清除 ymodem 接收缓存区
////    memset(bulbsShape_ymodem_buf.ymodem_switch_buf, 0, sizeof(bulbsShape_ymodem_t));

    info.mode = xyzmodem_ymodem;
    res = xyzmodem_stream_open(&info, &err);
    if (res) {
        // printk("error open ymodem\n");
        goto close;
    }
    // 擦除目标 flash
    Erase_mask_flash();
    /* when stat >= 0 */
    do {
////        if ((res = xyzmodem_stream_read((char*)bulbsShape_ymodem_buf.ymodem_switch_buf, sizeof(xyz.pkt), &err)) > 0) {
////            if ((offset + res) < FLASH_ADR_LTSTR_MASK_ID || (offset + res) >= (FLASH_ADR_LTSTR_MASK_ID + 0x1000)) {
////                Erase_mask_flash();
////                myprint("write error\r\n");
////                break;
////            }
////            ltstr_write_flash(offset, (unsigned short)res, (u8*)(bulbsShape_ymodem_buf.ymodem_switch_buf));
////            offset += res;
////        }
    } while (xyz.len != -1);

    xyzmodem_stream_close(&err);
    xyzmodem_stream_terminate(false, &getcxmodem);

    // uart_irq_rx_enable(uart);
    // printk("Total Size = %d Bytes\n", offset);

////    return offset;

close:
    xyzmodem_stream_close(&err);
    xyzmodem_stream_terminate(false, &getcxmodem);
    // uart_irq_rx_enable(uart);

    return -1;
}
#endif

