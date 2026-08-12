#include "stm32f10x.h"

#define DEMCR_REG       (*((__IO uint32_t *)0xE000EDFCU))
#define DWT_CTRL_REG    (*((__IO uint32_t *)0xE0001000U))
#define DWT_CYCCNT_REG  (*((__IO uint32_t *)0xE0001004U))
#define DEMCR_TRCENA    (1UL << 24)
#define DWT_CYCCNTENA   (1UL << 0)

extern volatile uint32_t systick_ms;

static void Delay_CycleCounterInit(void)
{
    DEMCR_REG |= DEMCR_TRCENA;
    DWT_CYCCNT_REG = 0U;
    DWT_CTRL_REG |= DWT_CYCCNTENA;
}

void Delay_TickInit(void)
{
    systick_ms = 0U;
    Delay_CycleCounterInit();
    (void)SysTick_Config(SystemCoreClock / 1000U);
}

uint32_t Delay_GetTick(void)
{
    return systick_ms;
}

void Delay_us(uint32_t xus)
{
    uint32_t start_cycle;
    uint32_t wait_cycles;

    if ((DWT_CTRL_REG & DWT_CYCCNTENA) == 0U)
    {
        Delay_CycleCounterInit();
    }

    start_cycle = DWT_CYCCNT_REG;
    wait_cycles = (SystemCoreClock / 1000000U) * xus;
    while ((uint32_t)(DWT_CYCCNT_REG - start_cycle) < wait_cycles)
    {
    }
}

void Delay_ms(uint32_t xms)
{
    while (xms--)
    {
        Delay_us(1000U);
    }
}

void Delay_s(uint32_t xs)
{
    while (xs--)
    {
        Delay_ms(1000U);
    }
}
