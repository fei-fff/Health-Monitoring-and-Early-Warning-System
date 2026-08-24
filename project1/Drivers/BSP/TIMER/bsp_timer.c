#include "./BSP/TIMER/bsp_timer.h"

static TIM_HandleTypeDef g_tim6_handle;
static volatile uint32_t g_tick_1ms = 0;
static volatile uint8_t g_sample_due = 0;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    }
}

void bsp_timer_init(void)
{
    g_tim6_handle.Instance = TIM6;
    g_tim6_handle.Init.Prescaler = 84 - 1;
    g_tim6_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    g_tim6_handle.Init.Period = 1000 - 1;
    g_tim6_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&g_tim6_handle);
    HAL_TIM_Base_Start_IT(&g_tim6_handle);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        g_tick_1ms++;

        if (g_tick_1ms >= 1000)
        {
            g_tick_1ms = 0;
            g_sample_due = 1;
        }
    }
}

bool bsp_timer_sample_due(void)
{
    if (g_sample_due != 0)
    {
        g_sample_due = 0;
        return true;
    }

    return false;
}

void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&g_tim6_handle);
}


