#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/TIMER/bsp_timer.h"
#include <stdio.h>

int main(void)
{
    float temp = 0.0f;
    float vref = 0.0f;
    float vdd = 0.0f;
    char buf[40];

    HAL_Init();
    sys_stm32_clock_init(336, 8, 2, 7);
    delay_init(168);
    usart_init(115200);
    lcd_init();
    adc_health_init();
    bsp_timer_init();

    lcd_clear(WHITE);

    while (1)
    {
        if (bsp_timer_sample_due())
        {
            adc_health_read(&temp, &vref, &vdd);

            sprintf(buf, "MCU Temp: %d.%d C", (int)temp, (int)(temp * 10.0f) % 10);
            lcd_show_string(30, 50, 200, 16, 16, buf, BLUE);

            sprintf(buf, "VDD: %d.%03d V", (int)(vdd / 1000.0f), ((int)vdd) % 1000);
            lcd_show_string(30, 80, 200, 16, 16, buf, BLUE);
        }
    }
}


