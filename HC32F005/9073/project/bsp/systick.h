#ifndef __SYSTICK_H
#define __SYSTICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base_types.h"

#define SysTick_CLKSource_HCLK      ((uint32_t)0x00000004)
#define TICKS_PER_SEC               1000 // tick计数每1ms溢出一次

// extern uint32_t g_sys_hclk;
#define SYS_CLOCK_HZ                24000000 // (g_sys_hclk)
#define SYS_CLOCK_1S                SYS_CLOCK_HZ
#define SYS_CLOCK_1MS               24000 // (SYS_CLOCK_1S / 1000)
#define SYS_CLOCK_1US               24 //(SYS_CLOCK_1S / 1000000)
#define SYS_CLOCK_4S                (SYS_CLOCK_1S << 2)

#define SYS_TICK_PER_MS             SYS_CLOCK_1MS
#define SYS_TICK_PER_US             SYS_CLOCK_1US
#define SYS_TICK_LOAD_MAX           0xffffff	// systick定时器重载值(最大值)
#define SYS_TICK_MAX_MS             699 		// ((uint32_t)(SYS_TICK_LOAD_MAX / CLOCK_SYS_CLOCK_1MS))
#define SYS_TICK_MAX_US             699050 		// ((uint32_t)(SYS_TICK_LOAD_MAX / CLOCK_SYS_CLOCK_1US))

extern uint32_t g_systick_clock_ms;
#define SYS_TIME_GET_MS()           g_systick_clock_ms
#define SYS_TICK_GET_VAL()          (SysTick->VAL)

void sysclk_init(void);
void systick_init(void);
void systick_time_sync(void);
uint8_t sys_time_exceed_ms(u32 ref, u32 span_ms);
uint8_t sys_time_exceed_us(u32 ref, u32 span_us);

#ifdef __cplusplus
}
#endif

#endif // __SYSTICK_H
