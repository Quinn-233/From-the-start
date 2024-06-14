#ifndef __MAIN_H__
#define __MAIN_H__

#define SYS_LED_PORT    3
#define SYS_LED_PIN     4//4

#define SYS_LED_INIT()  Gpio_InitIO(SYS_LED_PORT, SYS_LED_PIN, GpioDirOut)
#define SYS_LED_ON()    Gpio_SetIO(SYS_LED_PORT, SYS_LED_PIN, 1)
#define SYS_LED_OFF()   Gpio_SetIO(SYS_LED_PORT, SYS_LED_PIN, 0)
#define SYS_LED_SETIO(val)   Gpio_SetIO(SYS_LED_PORT, SYS_LED_PIN, val)

#endif
