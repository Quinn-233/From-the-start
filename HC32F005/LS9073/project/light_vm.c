#include "light_vm.h"

static u8 g_vm_change[LIGVM_CHG_FLAG_BUF_LEN] = {0};
static pixel_u g_vm_buf[STRLIGHTS_LENGTH] = {0};
ligvm_buf_s g_ligvm_buf;
pixel_u LS9073_color[512]={0};
u8 LS9073_vgs[50]={0};
void ligvm_buf_init(void)
{
    g_ligvm_buf.chg_flag = g_vm_change;
    g_ligvm_buf.vm_buf = g_vm_buf;
    ligvm_buf_reset();
}

void ligvm_buf_reset(void)
{
    for (u16 i = 0; i < STRLIGHTS_LENGTH; i++) {
        g_vm_buf[i].value = 0;
    }
    for (u8 i = 0; i < LIGVM_CHG_FLAG_BUF_LEN; i++) {
        g_vm_change[i] = 0xff;
    }
    g_ligvm_buf.status = LIGVM_STATUS_IDLE;
}

void ligvm_buf_set_status(ligvm_status_e status)
{
    if (status >= LIGVM_STATUS_BUTT) {
        return;
    }
    g_ligvm_buf.status = status;
}

ligvm_status_e ligvm_buf_get_status(void)
{
    return g_ligvm_buf.status;
}

void ligvm_buf_set_color(u16 addr, pixel_u rgb, u8 luminance)
{
    pixel_u color = {0};

#ifdef STRLIGHTS_COLOR_RGBW
    u8 w = (u8)(((u16)rgb.rgb.w * luminance) / STRLIGHTS_LUMINANCE_MAX_VAL);
    color.rgb.w = w;
#endif
    u8 r = (u8)(((u16)rgb.rgb.r * luminance) / STRLIGHTS_LUMINANCE_MAX_VAL);
    u8 g = (u8)(((u16)rgb.rgb.g * luminance) / STRLIGHTS_LUMINANCE_MAX_VAL);
    u8 b = (u8)(((u16)rgb.rgb.b * luminance) / STRLIGHTS_LUMINANCE_MAX_VAL);

    color.rgb.r = r;
    color.rgb.g = g;
    color.rgb.b = b;

    if (g_ligvm_buf.vm_buf[addr].value != color.value) {
        LIGVM_CHANGED(addr);
        g_ligvm_buf.vm_buf[addr].value = color.value;
    }
}

void ligvm_buf_display(u16 addr)
{
    strlights_set_color(STRLIGHTS_START_ADDR + addr, (u8 *)&(g_ligvm_buf.vm_buf[addr].value));
}

void ligvm_buf_display_color(u16 addr, pixel_u rgb)
{
    g_ligvm_buf.vm_buf[addr].value = rgb.value;
    LIGVM_NCHANGE(addr);
    strlights_set_color(STRLIGHTS_START_ADDR + addr, (u8 *)&(g_ligvm_buf.vm_buf[addr].value));
}
