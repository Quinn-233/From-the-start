#ifndef LIGHT_VM_H
#define LIGHT_VM_H

#include "base_types.h"
#include "string_lights.h"

typedef struct tag_rgb
{
//#ifdef STRLIGHTS_COLOR_RGBW
//    unsigned char w;
//#else
//    unsigned char rsv;
//#endif
	unsigned char w;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb_s;

// 小端: 0xbbggrrww
typedef union tag_pixel
{
    rgb_s rgb;
    unsigned int value;
} pixel_u;

typedef struct tag_color
{
    u8 luminance;
    pixel_u pixel;
} color_s;

typedef enum tag_ligvm_status
{
    LIGVM_STATUS_IDLE = 0,
    LIGVM_STATUS_WRITE,
    LIGVM_STATUS_WRITE_DONE,
    LIGVM_STATUS_READ,
    LIGVM_STATUS_BUTT
} ligvm_status_e;

typedef struct tag_ligvm_buf
{
    ligvm_status_e status;
    u8 *chg_flag;
    pixel_u *vm_buf;
} ligvm_buf_s;

extern ligvm_buf_s g_ligvm_buf;
#define LIGVM_BUF()             (g_ligvm_buf.vm_buf)
#define LIGVM_CHANGED(addr)     (g_ligvm_buf.chg_flag[(addr) / 8] |= 1 << ((addr) % 8))
#define LIGVM_NCHANGE(addr)     (g_ligvm_buf.chg_flag[(addr) / 8] &= ~(1 << ((addr) % 8)))
#define LIGVM_IF_CHANGED(addr)  (g_ligvm_buf.chg_flag[(addr) / 8] & (1 << ((addr) % 8)))
#define LIGVM_CHG_FLAG_BUF_LEN  6 // (((u16)(STRLIGHTS_LENGTH / 8) + 1)
extern ligvm_buf_s g_ligvm_buf;
extern pixel_u LS9073_color[512];
extern u8 LS9073_vgs[50];
void ligvm_buf_init(void);
void ligvm_buf_reset(void);
ligvm_status_e ligvm_buf_get_status(void);
void ligvm_buf_set_status(ligvm_status_e status);
void ligvm_buf_set_color(u16 addr, pixel_u rgb, u8 luminance);
void ligvm_buf_display(u16 addr);
void ligvm_buf_display_color(u16 addr, pixel_u rgb);

#endif // LIGHT_VM_H
