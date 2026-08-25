#ifndef __APP_UI_H
#define __APP_UI_H

#include <stdint.h>
#include "app_health.h"

typedef enum
{
    UI_PAGE_DASHBOARD = 0,
    UI_PAGE_THRESHOLDS,
    UI_PAGE_EVENTS
} ui_page_t;

void ui_init(void);
void ui_set_page(uint8_t page);
uint8_t ui_get_page(void);
void ui_show_page(const health_sample_t *sample);

#endif
