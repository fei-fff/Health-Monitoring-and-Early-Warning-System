#include "app_health.h"
#include "./BSP/ADC/adc.h"
#include <string.h>

#define FILTER_ALPHA    0.25f

static app_state_t g_app_state;
static float g_temp_filtered = 0.0f;
static float g_vdd_filtered = 0.0f;
static bool g_filter_initialized = false;

void app_health_init(void)
{
    memset(&g_app_state, 0, sizeof(g_app_state));

    g_app_state.temp_high_c = 65.0f;
    g_app_state.vdd_low_mv = 2800.0f;

    g_temp_filtered = 0.0f;
    g_vdd_filtered = 0.0f;
    g_filter_initialized = false;
}

void app_health_update(void)
{
    float temp = 0.0f;
    float vref = 0.0f;
    float vdd = 0.0f;
    uint16_t previous_flags = 0;
    uint16_t new_flags = ALARM_NONE;
    uint16_t rising_flags = 0;

    adc_health_read(&temp, &vref, &vdd);

    if (g_filter_initialized == false)
    {
        g_temp_filtered = temp;
        g_vdd_filtered = vdd;
        g_filter_initialized = true;
    }
    else
    {
        g_temp_filtered = g_temp_filtered + FILTER_ALPHA * (temp - g_temp_filtered);
        g_vdd_filtered = g_vdd_filtered + FILTER_ALPHA * (vdd - g_vdd_filtered);
    }

    previous_flags = g_app_state.sample.alarm_flags;

    if (g_temp_filtered > g_app_state.temp_high_c)
    {
        new_flags |= ALARM_TEMP_HIGH;
    }

    if (g_vdd_filtered < g_app_state.vdd_low_mv)
    {
        new_flags |= ALARM_VDD_LOW;
    }

    rising_flags = new_flags & ~previous_flags;

    if ((rising_flags & ALARM_TEMP_HIGH) != 0)
    {
        g_app_state.sample.event_count++;
    }

    if ((rising_flags & ALARM_VDD_LOW) != 0)
    {
        g_app_state.sample.event_count++;
    }

    g_app_state.sample.mcu_temp_c = g_temp_filtered;
    g_app_state.sample.vref_mv = vref;
    g_app_state.sample.vdd_mv = g_vdd_filtered;
    g_app_state.sample.alarm_flags = new_flags;
}

const health_sample_t *app_health_get_sample(void)
{
    return &g_app_state.sample;
}

bool app_health_is_alarm(void)
{
    return g_app_state.sample.alarm_flags != ALARM_NONE;
}

void app_health_set_thresholds(float temp_high_c, float vdd_low_mv)
{
    g_app_state.temp_high_c = temp_high_c;
    g_app_state.vdd_low_mv = vdd_low_mv;
}



