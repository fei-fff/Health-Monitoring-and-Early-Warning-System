#ifndef __APP_HEALTH_H
#define __APP_HEALTH_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    ALARM_NONE        = 0,
    ALARM_TEMP_HIGH   = (1U << 0),
    ALARM_VDD_LOW     = (1U << 1)
} alarm_flag_t;

typedef struct
{
    float mcu_temp_c;
    float vref_mv;
    float vdd_mv;
    uint32_t event_count;
    uint16_t alarm_flags;
} health_sample_t;

typedef struct
{
    health_sample_t sample;
    float temp_high_c;
    float vdd_low_mv;
} app_state_t;

void app_health_init(void);
void app_health_update(void);
const health_sample_t *app_health_get_sample(void);
bool app_health_is_alarm(void);
void app_health_set_thresholds(float temp_high_c, float vdd_low_mv);

#endif



