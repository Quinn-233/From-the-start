#include "hc32f005.h"
#include "key.h"

static volatile u8 KEY1_LONG_Flg = OFF;
static volatile u8 KEY2_LONG_Flg = OFF;
static volatile u8 KEY1_SHORT_Flg = OFF;
static volatile u8 KEY2_SHORT_Flg = OFF;



void key_init(void)
{
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);

    // 上拉输入
    Gpio_InitIOExt(KEY1_PORT, KEY1_PIN, GpioDirIn, TRUE, FALSE, FALSE, FALSE);
    Gpio_InitIOExt(KEY2_PORT, KEY2_PIN, GpioDirIn, TRUE, FALSE, FALSE, FALSE);
}

void key_scan(void)
{
    static u8 key1_status = KEY_OFF;
    static u8 key2_status = KEY_OFF;
    static u8 key1_count = 0;
    static u32 key2_count = 0;

    if (KEY1_VAL() == 0) {
        key1_count++;
        if (key1_count > KEY1_LONGCNT) {
            key1_count = 0;
            key1_status = KEY1_LONGON;
            KEY1_LONG_Flg = ON;
        }
    } else if (key1_status == KEY_OFF) {
        if (key1_count > KEY1_SHORTCNT) {
            key1_count = 0;
            KEY1_SHORT_Flg = ON;
        } else {
            key1_count = 0;
        }
    }
    if (KEY1_VAL() != 0) {
        key1_count = 0;
        key1_status = KEY_OFF;
    }

    if (KEY2_VAL() == 0) {
        key2_count++;
        if (key2_count > KEY2_LONGCNT) {
            key2_count = 0;
            key2_status = KEY2_LONGON;
            KEY2_LONG_Flg = ON;
        }
    }
    else if (key2_status == KEY_OFF) {
        if (key2_count > KEY2_SHORTCNT) {
            key2_count = 0;
            KEY2_SHORT_Flg = ON;
        } else {
            key2_count = 0;
        }
    }
    if (KEY2_VAL() != 0) {
        key2_count = 0;
        key2_status = KEY_OFF;
    }
}

key_status_e key_status_get(u8 key)
{
    key_status_e status = KEY_STATUS_BUTT;
    if (key == KEY1) {
        if (KEY1_LONG_Flg == ON) {
            KEY1_LONG_Flg = OFF;
            status = KEY_STATUS_LON;
        } else if (KEY1_SHORT_Flg == ON) {
            KEY1_SHORT_Flg = OFF;
            status = KEY_STATUS_SON;
        } else {
            status = KEY_STATUS_OFF;
        }
    } else {
        if (KEY2_LONG_Flg == ON) {
            KEY2_LONG_Flg = OFF;
            status = KEY_STATUS_LON;
        } else if (KEY2_SHORT_Flg == ON) {
            KEY2_SHORT_Flg = OFF;
            status = KEY_STATUS_SON;
        } else {
            status = KEY_STATUS_OFF;
        }
    }

    return status;
}
