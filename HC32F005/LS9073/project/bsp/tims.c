#include "hc32f005.h"
#include "tims.h"
#include "key.h"
#include "board_stkhc32f005.h"
#include "light_event.h"
#include "light_effects.h"

void Tim1_Handler(void)
{
    // static int cnt = 0;
    if (TRUE == Bt_GetIntFlag(TIM1)) {
        Bt_ClearIntFlag(TIM1);

        // cnt++;
        // if (cnt % 2 == 0) {
        //     ligeff_show_frames();
        // }
    }
}

void Tim2_Handler(void)
{
    static u8 led_cnt = 0;
    if (TRUE == Bt_GetIntFlag(TIM2)) {
        Bt_ClearIntFlag(TIM2);

        if (led_cnt == 0) {
            SYS_LED_ON();
        } else if (led_cnt == 50) {
            SYS_LED_OFF();
        } else if (led_cnt == 100) {
            led_cnt = 0xff;
        }
        led_cnt++;

        // key_scan();
        // ligevt_key_process();
    }
}

en_result_t btim1_init(void)
{
    stc_bt_config_t stcConfig;
    en_result_t enResult = Error;
    // uint16_t u16ArrData = 0x8ACF; // 0xFFFF - 30000 = 35535; 3M频率，每10ms溢出一次
    uint16_t u16ArrData = 0xF447; // 1ms 定时器
    uint16_t u16InitCntData = 0x8000;

    // 使能baseTime1 时钟
    Clk_SetPeripheralGate(ClkPeripheralBt, TRUE);

    stcConfig.pfnTim1Cb = Tim1_Handler;

    stcConfig.enGateP = BtPositive;
    stcConfig.enGate  = BtGateDisable;
    stcConfig.enPRS   = BtPCLKDiv8; // 目前PCLK配置的频率与HCLK相同24M
    stcConfig.enTog   = BtTogDisable;
    stcConfig.enCT    = BtTimer;
    stcConfig.enMD    = BtMode2; // 模式2自动重装载16位计数器/定时器
    if (Ok != Bt_Init(TIM1, &stcConfig)) {
        enResult = Error;
    }

    Bt_ClearIntFlag(TIM1);
    Bt_EnableIrq(TIM1);
    EnableNvic(TIM1_IRQn, 1, TRUE);

    Bt_ARRSet(TIM1, u16ArrData);
    Bt_Cnt16Set(TIM1, u16InitCntData);
    Bt_Run(TIM1);

    return enResult;
}

en_result_t btim2_init(void)
{
    stc_bt_config_t stcConfig;
    en_result_t enResult = Error;
    uint16_t u16ArrData = 0x8ACF; // 0xFFFF - 30000 = 35535; 3M频率，每10ms溢出一次
    uint16_t u16InitCntData = 0x8000;

    Clk_SetPeripheralGate(ClkPeripheralBt, TRUE);

    stcConfig.pfnTim2Cb = Tim2_Handler;

    stcConfig.enGateP = BtPositive;
    stcConfig.enGate  = BtGateDisable;
    stcConfig.enPRS   = BtPCLKDiv8; // 目前PCLK配置的频率与HCLK相同24M
    stcConfig.enTog   = BtTogDisable;
    stcConfig.enCT    = BtTimer;
    stcConfig.enMD    = BtMode2; // 模式2自动重装载16位计数器/定时器
    if (Ok != Bt_Init(TIM2, &stcConfig)) {
        enResult = Error;
    }

    Bt_ClearIntFlag(TIM2);
    Bt_EnableIrq(TIM2);
    EnableNvic(TIM2_IRQn, 3, TRUE);

    Bt_ARRSet(TIM2, u16ArrData);
    Bt_Cnt16Set(TIM2, u16InitCntData);
    Bt_Run(TIM2);

    return enResult;
}
