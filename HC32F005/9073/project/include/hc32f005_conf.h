#ifndef HC32F005_CONF_H
#define HC32F005_CONF_H

// #include "adc.h"
// #include "adt.h"
#include "bt.h"
#include "clk.h"
// #include "crc.h"
#include "ddl.h"
#include "flash.h"
#include "gpio.h"
// #include "i2c.h"
// #include "lpm.h"
// #include "lvd.h"
// #include "pca.h"
// #include "reset.h"
// #include "spi.h"
// #include "trim.h"
#include "uart.h"
// #include "vc.h"
// #include "wdt.h"

#ifdef  USE_FULL_ASSERT
    #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
    void assert_failed(uint8_t* file, uint32_t line);
#else
    #define assert_param(expr) ((void)0)
#endif // USE_FULL_ASSERT

#endif // HC32F005_CONF_H
