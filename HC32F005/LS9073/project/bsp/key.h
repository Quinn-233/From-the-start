#ifndef __KEY_H
#define __KEY_H

#define OFF                 0x00
#define ON                  0x01

#define KEY1_SHORTCNT       20
#define KEY2_SHORTCNT       20

#define KEY1_LONGCNT        260 // 这里不需要长按效果，将该值置为无效
#define KEY2_LONGCNT        0x6000 // (normal模式下 400颗灯珠) 8s

#define KEY_OFF             0x00
#define KEY1_ON             0x01
#define KEY1_LONGON         0x02
#define KEY2_ON             0x04
#define KEY2_LONGON         0x08

#define KEY1_PORT           0//1//2
#define KEY1_PIN            2//5//6
#define KEY2_PORT           0//1//2
#define KEY2_PIN            1//4//5

#define KEY1                1
#define KEY2                2

typedef enum tag_key_status
{
    KEY_STATUS_OFF = 0,
    KEY_STATUS_SON, // 短按
    KEY_STATUS_LON, // 长按
    KEY_STATUS_BUTT
} key_status_e;

#define KEY1_VAL()          Gpio_GetIO(KEY1_PORT, KEY1_PIN)
#define KEY2_VAL()          Gpio_GetIO(KEY2_PORT, KEY2_PIN)

void key_init(void);
void key_scan(void);
key_status_e key_status_get(unsigned char key);

#endif  /* __KEY_H */
