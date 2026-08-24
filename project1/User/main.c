#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/TIMER/bsp_timer.h"
#include "app_health.h"
#include <stdio.h>

int main(void)
{
    const health_sample_t *health;
    char buf[40];
    char *status = "Normal";

    HAL_Init();
    sys_stm32_clock_init(336, 8, 2, 7);
    delay_init(168);
    usart_init(115200);
    lcd_init();
    adc_health_init();
    bsp_timer_init();
    app_health_init();

    lcd_clear(WHITE);

    while (1)
    {
        if (bsp_timer_sample_due())
        {
            app_health_update();
            health = app_health_get_sample();

            sprintf(buf, "MCU Temp: %d.%d C", (int)health->mcu_temp_c,
                    (int)(health->mcu_temp_c * 10.0f) % 10);
            lcd_show_string(30, 50, 200, 16, 16, buf, BLUE);

            sprintf(buf, "VDD: %d.%03d V", (int)(health->vdd_mv / 1000.0f),
                    ((int)health->vdd_mv) % 1000);
            lcd_show_string(30, 80, 200, 16, 16, buf, BLUE);

            if ((health->alarm_flags & ALARM_TEMP_HIGH) != 0)
            {
                status = "Temp High";
            }
            else if ((health->alarm_flags & ALARM_VDD_LOW) != 0)
            {
                status = "VDD Low";
            }
            else
            {
                status = "Normal";
            }

            sprintf(buf, "Status: %s", status);
            lcd_show_string(30, 110, 200, 16, 16, buf, RED);

            sprintf(buf, "Events: %u", (unsigned int)health->event_count);
            lcd_show_string(30, 140, 200, 16, 16, buf, BLUE);
        }
    }
}


