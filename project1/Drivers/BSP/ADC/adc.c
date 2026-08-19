#include "./SYSTEM/delay/delay.h"
#include "./BSP/ADC/adc.h"

static ADC_HandleTypeDef g_adc_handle;

static uint16_t adc_read_channel(uint32_t ch)
{
    ADC_ChannelConfTypeDef adc_ch = {0};
    uint16_t value = 0;

    adc_ch.Channel = ch;
    adc_ch.Rank = 1;
    adc_ch.SamplingTime = ADC_SAMPLETIME_480CYCLES;

    HAL_ADC_ConfigChannel(&g_adc_handle, &adc_ch);
    HAL_ADC_Start(&g_adc_handle);
    HAL_ADC_PollForConversion(&g_adc_handle, 10);
    value = (uint16_t)HAL_ADC_GetValue(&g_adc_handle);
    HAL_ADC_Stop(&g_adc_handle);

    return value;
}

static uint16_t adc_read_average(uint32_t ch, uint8_t times)
{
    uint32_t sum = 0;
    uint8_t i;

    for (i = 0; i < times; i++)
    {
        sum += adc_read_channel(ch);
        delay_ms(2);
    }

    return (uint16_t)(sum / times);
}

void adc_health_init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();

    g_adc_handle.Instance = ADC1;
    g_adc_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    g_adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
    g_adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    g_adc_handle.Init.ScanConvMode = DISABLE;
    g_adc_handle.Init.ContinuousConvMode = DISABLE;
    g_adc_handle.Init.NbrOfConversion = 1;
    g_adc_handle.Init.DiscontinuousConvMode = DISABLE;
    g_adc_handle.Init.NbrOfDiscConversion = 0;
    g_adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    g_adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    g_adc_handle.Init.DMAContinuousRequests = DISABLE;

    HAL_ADC_Init(&g_adc_handle);

    /* 开启内部温度传感器和 VREFINT */
    ADC->CCR |= ADC_CCR_TSVREFE;
}

void adc_health_read(float *temp_c, float *vref_mv, float *vdd_mv)
{
    uint16_t temp_raw = 0;
    uint16_t vref_raw = 0;
    float vdd = 3.3f;
    float v_sense = 0.0f;

    vref_raw = adc_read_average(ADC_CHANNEL_VREFINT, 10);
    temp_raw = adc_read_average(ADC_CHANNEL_TEMPSENSOR, 10);

    vdd = (1210.0f * 4095.0f) / (float)vref_raw;
    v_sense = (float)temp_raw * vdd / 4095.0f;

    *vref_mv = 1210.0f;
    *vdd_mv = vdd;
    *temp_c = (v_sense - 760.0f) / 2.5f + 25.0f;
}

