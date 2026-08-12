#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Delay.h"

uint16_t second = 0;
uint16_t ShowResultFlag = 1;
void TIM_Init(void){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
   
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 10000 - 1;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 7200-1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0; // CNT的计数周期为1s
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
    NVIC_Init(&NVIC_InitStructure);
    

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    TIM_Cmd(TIM2, ENABLE);
    
}

void TIM2_IRQHandler(void){
    
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){// //???????
            second++;
            if(second==9){
                second = 0;
                ShowResultFlag = !(ShowResultFlag);
            }
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
}

