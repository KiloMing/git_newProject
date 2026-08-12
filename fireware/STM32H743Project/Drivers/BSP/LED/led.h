/**
 ****************************************************************************************************
 * @file        led.h
 * @version     V1.0
 * @date        2022-09-06
 * @brief       LED 驱动代码
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:STM32H743开发板
 *
 * 修改说明
 * V1.0 2020906
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __LED_H
#define __LED_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* 引脚 定义 */

#define LED0_GPIO_PORT                  GPIOB
#define LED0_GPIO_PIN                   GPIO_PIN_1
#define LED0_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

#define LED1_GPIO_PORT                  GPIOB
#define LED1_GPIO_PIN                   GPIO_PIN_0
#define LED1_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

/* LED端口定义 */
#define LED0(x)   do{ x ? \
                      HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_RESET); \
                  }while(0)             /* LED0 = RED */

#define LED1(x)   do{ x ? \
                      HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_RESET); \
                  }while(0)             /* LED1 = GREEN */

/* LED取反定义 */
#define LED0_TOGGLE()    do{ HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_GPIO_PIN); }while(0)   /* LED0 = !LED0 */
#define LED1_TOGGLE()    do{ HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_GPIO_PIN); }while(0)   /* LED1 = !LED1 */

/******************************************************************************************/

void led_init(void);                    /* LED初始化 */

#endif


