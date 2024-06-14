#ifndef LIGHT_EVENT_H
#define LIGHT_EVENT_H

#include "light_vm.h"

typedef struct tag_ligevt_com_cmd
{
    uint8_t effect;
    uint8_t speed;
    uint8_t luminance;
    uint8_t color_len;
    pixel_u color[];
} ligevt_com_cmd_s;


#define LIGEVT_COM_CMD_FAC_REST 0xF1

void ligevt_process(void);

#endif // LIGHT_EVENT_H
