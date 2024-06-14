#ifndef __COM_H
#define __COM_H

#include "base_types.h"

// 0x55 0xAA len(包含1byte校验) datas(数据) checksum
#define COM_CMD_BUF_LEN         80
#define COM_CMD_HEARD_1         0x55
#define COM_CMD_HEARD_2         0xAA
#define COM_CMD_DATA_LEN_BYTES  1   // 数据长度占用的位数

typedef enum tag_com_status
{
    COM_STATUS_IDLE = 0,
    COM_STATUS_RCV,
    COM_STATUS_BUTT
} com_status_e;

void com_init(void);
uint8_t *com_cmd_get(void);
void com_cmd_send(uint8_t *data, uint8_t len);

#endif // __COM_H
