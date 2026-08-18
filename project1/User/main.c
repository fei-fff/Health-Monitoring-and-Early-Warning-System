#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LCD/lcd.h"

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(336, 8, 2, 7);
    delay_init(168);
    usart_init(115200);      /* 必须先初始化串口，否则 lcd_init 里的 printf 会卡住 */
    lcd_init();

    lcd_clear(WHITE);
    lcd_show_string(30, 50, 200, 16, 16, "STM32F407 Health Monitor", BLUE);

    while (1)
    {
    }
}

