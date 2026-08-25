#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/TIMER/bsp_timer.h"
#include "app_health.h"
#include "app_ui.h"
#include "protocol.h"
#include <string.h>

int main(void)
{
    const health_sample_t *health;
    char tx_buf[96];
    char rx_line[USART_REC_LEN + 1];
    protocol_command_t cmd;
    int len = 0;
    uint8_t ack_code = 0;
    uint16_t last_alarm_flags = 0;
    uint16_t rising_flags = 0;

    HAL_Init();
    sys_stm32_clock_init(336, 8, 2, 7);
    delay_init(168);
    usart_init(115200);
    led_init();
    lcd_init();
    adc_health_init();
    bsp_timer_init();
    app_health_init();
    ui_init();

    while (1)
    {
        if (bsp_timer_sample_due())
        {
            app_health_update();
            health = app_health_get_sample();
            ui_show_page(health);

            len = protocol_build_sample(health, HAL_GetTick(), tx_buf, sizeof(tx_buf));
            HAL_UART_Transmit(&g_uart1_handle, (uint8_t *)tx_buf, len, 100);

            rising_flags = health->alarm_flags & ~last_alarm_flags;

            if ((rising_flags & ALARM_TEMP_HIGH) != 0)
            {
                len = protocol_build_alarm(1, HAL_GetTick(),
                                           (int32_t)(health->mcu_temp_c * 10.0f),
                                           tx_buf, sizeof(tx_buf));
                HAL_UART_Transmit(&g_uart1_handle, (uint8_t *)tx_buf, len, 100);
            }

            if ((rising_flags & ALARM_VDD_LOW) != 0)
            {
                len = protocol_build_alarm(2, HAL_GetTick(),
                                           (int32_t)health->vdd_mv,
                                           tx_buf, sizeof(tx_buf));
                HAL_UART_Transmit(&g_uart1_handle, (uint8_t *)tx_buf, len, 100);
            }

            last_alarm_flags = health->alarm_flags;
        }

        if ((g_usart_rx_sta & 0x8000U) != 0)
        {
            uint16_t rx_len = g_usart_rx_sta & 0x3FFFU;

            if (rx_len < sizeof(rx_line))
            {
                memcpy(rx_line, g_usart_rx_buf, rx_len);
                rx_line[rx_len] = '\0';
            }

            g_usart_rx_sta = 0;
            ack_code = 0;

            if (protocol_parse_line(rx_line, &cmd) != 0)
            {
                if (cmd.cmd == CMD_TH_TEMP)
                {
                    if (cmd.arg > 0 && cmd.arg <= 1500)
                        app_health_set_thresholds((float)cmd.arg / 10.0f, app_health_get_vdd_threshold());
                    else
                        ack_code = 2;
                }
                else if (cmd.cmd == CMD_TH_VDD)
                {
                    if (cmd.arg > 0 && cmd.arg <= 5000)
                        app_health_set_thresholds(app_health_get_temp_threshold(), (float)cmd.arg);
                    else
                        ack_code = 2;
                }
                else if (cmd.cmd == CMD_LED)
                {
                    if (cmd.arg == 1) LED0(0);
                    else if (cmd.arg == 0) LED0(1);
                    else ack_code = 2;
                }
                else if (cmd.cmd == CMD_PAGE)
                {
                    if (cmd.arg >= 0 && cmd.arg <= 2)
                        ui_set_page((uint8_t)cmd.arg);
                    else
                        ack_code = 2;
                }
                else
                {
                    ack_code = 1;
                }
            }
            else
            {
                ack_code = 1;
            }

            len = protocol_build_ack(cmd.cmd, ack_code, tx_buf, sizeof(tx_buf));
            HAL_UART_Transmit(&g_uart1_handle, (uint8_t *)tx_buf, len, 100);
        }
    }
}
