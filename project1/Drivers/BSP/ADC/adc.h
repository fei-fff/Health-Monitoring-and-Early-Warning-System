#ifndef __ADC_H
#define __ADC_H

#include "./SYSTEM/sys/sys.h"

void adc_health_init(void);
void adc_health_read(float *temp_c, float *vref_mv, float *vdd_mv);

#endif

