#include "app_ui.h"
#include "./BSP/LCD/lcd.h"
#include <stdio.h>

static uint8_t g_current_page = UI_PAGE_DASHBOARD;

static void ui_show_dashboard(const health_sample_t *sample)
{
    char buf[40];
    char *status = "Normal";

    lcd_show_string(30, 10, 200, 16, 16, "Health Monitor", BLUE);

    sprintf(buf, "MCU Temp: %d.%d C", (int)sample->mcu_temp_c,
            (int)(sample->mcu_temp_c * 10.0f) % 10);
    lcd_show_string(30, 50, 200, 16, 16, buf, BLUE);

    sprintf(buf, "VDD: %d.%03d V", (int)(sample->vdd_mv / 1000.0f),
            ((int)sample->vdd_mv) % 1000);
    lcd_show_string(30, 80, 200, 16, 16, buf, BLUE);

    if ((sample->alarm_flags & ALARM_TEMP_HIGH) != 0)
        status = "Temp High";
    else if ((sample->alarm_flags & ALARM_VDD_LOW) != 0)
        status = "VDD Low";

    sprintf(buf, "Status: %s", status);
    lcd_show_string(30, 110, 200, 16, 16, buf, RED);

    sprintf(buf, "Events: %u", (unsigned int)sample->event_count);
    lcd_show_string(30, 140, 200, 16, 16, buf, BLUE);
}

static void ui_show_thresholds(void)
{
    float temp_high_c = app_health_get_temp_threshold();
    float vdd_low_mv = app_health_get_vdd_threshold();
    char buf[40];

    lcd_show_string(30, 10, 200, 16, 16, "Thresholds", BLUE);

    sprintf(buf, "Temp High: %d.%d C", (int)temp_high_c,
            (int)(temp_high_c * 10.0f) % 10);
    lcd_show_string(30, 60, 200, 16, 16, buf, BLUE);

    sprintf(buf, "VDD Low: %d.%03d V", (int)(vdd_low_mv / 1000.0f),
            ((int)vdd_low_mv) % 1000);
    lcd_show_string(30, 90, 200, 16, 16, buf, BLUE);
}

static void ui_show_events(const health_sample_t *sample)
{
    char buf[40];
    char *status = "Normal";

    lcd_show_string(30, 10, 200, 16, 16, "Events", BLUE);

    sprintf(buf, "Count: %u", (unsigned int)sample->event_count);
    lcd_show_string(30, 60, 200, 16, 16, buf, BLUE);

    if ((sample->alarm_flags & ALARM_TEMP_HIGH) != 0)
        status = "Temp High";
    else if ((sample->alarm_flags & ALARM_VDD_LOW) != 0)
        status = "VDD Low";

    sprintf(buf, "Alarm: %s", status);
    lcd_show_string(30, 90, 200, 16, 16, buf, RED);
}

void ui_init(void)
{
    g_current_page = UI_PAGE_DASHBOARD;
    lcd_clear(WHITE);
}

void ui_set_page(uint8_t page)
{
    if (page > UI_PAGE_EVENTS)
        page = UI_PAGE_DASHBOARD;

    g_current_page = page;
    lcd_clear(WHITE);
}

uint8_t ui_get_page(void)
{
    return g_current_page;
}

void ui_show_page(const health_sample_t *sample)
{
    if (g_current_page == UI_PAGE_DASHBOARD)
        ui_show_dashboard(sample);
    else if (g_current_page == UI_PAGE_THRESHOLDS)
        ui_show_thresholds();
    else
        ui_show_events(sample);
}
