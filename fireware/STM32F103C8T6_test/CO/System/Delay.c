#include "stm32f10x.h"

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{
    uint32_t i;
    while (xus--) {
        i = 8;   // 72MHz 下约 1us，可根据实际情况微调
        while (i--) {
            __NOP();
        }
    }
}

void Delay_ms(uint32_t xms)
{
    while (xms--) {
        Delay_us(1000);
    }
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 
