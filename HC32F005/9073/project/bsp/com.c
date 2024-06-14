#include "hc32f005.h"
#include "com.h"

static uint8_t g_rx_flag = 0;
static uint8_t g_rx_len = 0;
static uint8_t g_rx_data[COM_CMD_BUF_LEN] = {0};

void RxIntCallback(void)
{
    static uint8_t heard = 0;
    static uint8_t rcv_cnt = 0;
    static com_status_e status = COM_STATUS_IDLE;

    uint8_t rcv = M0P_UART1->SBUF;
    if (status == COM_STATUS_IDLE) {
        if (rcv == COM_CMD_HEARD_1) {
            heard = rcv;
        }
        if ((rcv == COM_CMD_HEARD_2) && (heard == COM_CMD_HEARD_1)) {
            heard = 0;
            status = COM_STATUS_RCV;

            rcv_cnt = 0;
            // for (uint8_t i = 0; i < COM_CMD_BUF_LEN; i++) {
            //     g_rx_data[i] = 0;
            // }
        }
    } else if (status == COM_STATUS_RCV) {
        if (rcv_cnt == 0) {
            g_rx_len = rcv; // get length 1byte
        } else {
            g_rx_data[rcv_cnt - COM_CMD_DATA_LEN_BYTES] = rcv;
            if (rcv_cnt == g_rx_len) { // 接收完成
                status = COM_STATUS_IDLE;
                g_rx_flag = 1;
                return;
            }
        }
        rcv_cnt++;
    } else {
        // do nothing
    }
}
void ErrIntCallback(void)
{
}

void com_init(void)
{
    uint16_t timer = 0;
    uint32_t pclk = 0;

    stc_uart_config_t stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
    stc_uart_multimode_t stcMulti;
    stc_uart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;

    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);

    Gpio_InitIOExt(3, 5, GpioDirOut, TRUE, FALSE, FALSE, FALSE);
    Gpio_InitIOExt(3, 6, GpioDirOut, TRUE, FALSE, FALSE, FALSE);

    // 通道端口配置
    Gpio_SetFunc_UART1TX_P35();
    Gpio_SetFunc_UART1RX_P36();

    // 外设时钟使能
    Clk_SetPeripheralGate(ClkPeripheralBt, TRUE);   // 模式0/2可以不使能
    Clk_SetPeripheralGate(ClkPeripheralUart1, TRUE);

    stcUartIrqCb.pfnRxIrqCb = RxIntCallback;
    stcUartIrqCb.pfnTxIrqCb = NULL;
    stcUartIrqCb.pfnRxErrIrqCb = ErrIntCallback;
    stcConfig.pstcIrqCb = &stcUartIrqCb;
    stcConfig.bTouchNvic = TRUE;

    stcConfig.enRunMode = UartMode1;    // 异步全双工
    stcMulti.enMulti_mode = UartNormal; // 常规or主机模式，mode2/3才有多主机模式

    stcConfig.pstcMultiMode = &stcMulti;

    stcBaud.bDbaud = 1u;    // 双波特率
    stcBaud.u32Baud = 115200u;
    stcBaud.u8Mode = stcConfig.enRunMode;
    pclk = Clk_GetPClkFreq();
    timer = Uart_SetBaudRate(UARTCH1, pclk, &stcBaud);

    stcBtConfig.enMD = BtMode2;
    stcBtConfig.enCT = BtTimer;
    Bt_Init(TIM1, &stcBtConfig);    // 调用basetimer1设置函数产生波特率
    Bt_ARRSet(TIM1, timer);
    Bt_Cnt16Set(TIM1, timer);
    Bt_Run(TIM1);

    Uart_Init(UARTCH1, &stcConfig);
    Uart_EnableIrq(UARTCH1, UartRxIrq);
    Uart_ClrStatus(UARTCH1, UartRxFull);
    Uart_EnableFunc(UARTCH1, UartRx);
}

// data，待发送的数据；len，待发送的数据长度（不包含校验）
static uint8_t com_cmd_get_checksum(uint8_t *data, uint8_t len)
{
    // 从帧头开始按字节求和得出的结果对 256 求余
    uint8_t checksum = COM_CMD_HEARD_1 + COM_CMD_HEARD_2 + (len + 1);
    for (uint8_t i = 0; i < len; i++) {
        checksum += data[i];
    }
    return checksum %= 256;
}

uint8_t *com_cmd_get(void)
{
    if (g_rx_flag == 1) {
        g_rx_flag = 0;

        if (com_cmd_get_checksum(g_rx_data, g_rx_len - 1) == g_rx_data[g_rx_len - 1]) {
            g_rx_data[g_rx_len - 1] = 0;
            return g_rx_data;
        }
    }

    return NULL;
}

// data，待发送的数据；len，待发送的数据长度（不包含校验）
void com_cmd_send(uint8_t *data, uint8_t len)
{
    uint8_t checksum = com_cmd_get_checksum(data, len);
    Uart_SendData(UARTCH1, COM_CMD_HEARD_1);
    Uart_SendData(UARTCH1, COM_CMD_HEARD_2);
    Uart_SendData(UARTCH1, (len + 1)); // 长度包含1byte校验
    for (uint8_t i = 0; i < len; i++) {
        Uart_SendData(UARTCH1, data[i]);
    }
    Uart_SendData(UARTCH1, checksum);
}
