#include "hc32f005.h"
#include "systick.h"
#include "key.h"
#include "light_event.h"
#include "light_effects.h"

uint32_t g_sys_hclk = 0;
uint32_t g_systick_clock_ms = 0;

void SysTick_Handler(void)
{
}
/******************* 内核时钟频率配置 *********************/
void sysclk_init(void)
{
    stc_clk_config_t stcCfg;

    // 设置频率
    Clk_SetRCHFreq(ClkFreq24Mhz);
    stcCfg.enClkSrc = ClkRCH;     // ClkRCH  = 24M   设置时钟源
    stcCfg.enHClkDiv = ClkDiv1;   // HCLK = stcCfg.enClkSrc / stcCfg.enHClkDiv
    stcCfg.enPClkDiv = ClkDiv1;   // PCLK = stcCfg.enClkSrc / stcCfg.enHClkDiv / stcCfg.enPClkDiv

    Clk_Init(&stcCfg);
}
/******************* systick定时器配置 *********************/
void systick_init(void)
{
    g_sys_hclk = Clk_GetHClkFreq();

    SysTick->LOAD = SYS_TICK_LOAD_MAX;	// systick定时器重载值(最大值)
    SysTick->VAL  = 0;					// 清零当前systick定时器的计数值
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;	// systick时钟源为内核时钟HCLK
    // SysTick_Config(g_sys_hclk / 100); // 10ms
}

void systick_time_sync(void)
{
    static u32 tick = SYS_TICK_LOAD_MAX;
    u32 now = 0;
    u32 dvalue = 0;
    u32 add = 0;
    now = SysTick->VAL;		// 当前systick定时器的计数值
    if (tick != now) {
		// 还在向下计数
        if (tick > now) {
            dvalue = tick - now;						// 已经减少的计数值
		// 
        } else {
            dvalue = SYS_TICK_LOAD_MAX - now + tick;	// 已经减少的计数值
        }

        if (dvalue >= SYS_TICK_PER_MS) {
            add = dvalue / SYS_TICK_PER_MS;	// 
            dvalue = add * SYS_TICK_PER_MS;	// 已经过去的时间
			g_systick_clock_ms += add;
            if (tick >= dvalue) {
                tick -= dvalue;
            } else {
                tick = SYS_TICK_LOAD_MAX - dvalue + tick;	// tick=now
            }
        }
    }
}

uint8_t sys_time_exceed_ms(u32 ref, u32 span_ms)
{
    return (g_systick_clock_ms - ref) >= span_ms;
}

// 要求 span_us <= SYS_TICK_MAX_US
// 该函数调用周期需 <= SYS_TICK_MAX_MS 699ms(tick计数走一圈)
uint8_t sys_time_exceed_us(u32 ref, u32 span_us)
{
    u32 now = SysTick->VAL;
    u32 dvalue = 0;
    if (ref != now) {
        if (ref > now) {
            dvalue = ref - now;
        } else {
            dvalue = SYS_TICK_LOAD_MAX - now + ref;
        }
        return (dvalue >= (span_us * SYS_TICK_PER_US));
    } else {
        // 还有一种ref与now相等的情况，即SysTick计数已经走过一圈
        // 该函数不适用这种大颗粒度的延迟判断
        return 0;
    }
}
