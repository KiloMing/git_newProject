#include "BlueTeeth.h"

char str[10];
uint16_t BlueTeeth_value;
uint16_t Result[10];

void BlueTeeth_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USARTx, ENABLE);          //不要忘记开串口时钟
    RCC_APB2PeriphClockCmd(BlueTeeth_GPIO_CLK, ENABLE );	        // 打开串口端口时钟
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = BlueTeeth_Pin_Tx;					// 配置TX引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		            // 设置为推完输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BlueTeeth_GPIOx, &GPIO_InitStructure);				// 初始化Tx
    
    GPIO_InitStructure.GPIO_Pin = BlueTeeth_Pin_Rx;					// 配置Rx引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		            // 配置上拉输入
    GPIO_Init(BlueTeeth_GPIOx, &GPIO_InitStructure);				// 初始化 Rx
    
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USARTx, &USART_InitStruct);
    

    USART_ITConfig(USARTx,  USART_IT_RXNE, ENABLE);                 // 开启读寄存器中断
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USARTx, ENABLE);
}

void BlueTeeth_SentByte(uint8_t Byte){
    
    USART_SendData(USARTx, Byte);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) != SET);

}
/*
void BlueTeeth_SentString(char *str){
    uint8_t i = 0;
    while(str[i] != '\0'){
        BlueTeeth_SentByte(str[i]);
        i++;
    }
}
*/


//接收数据包， 数据包格式  0xFF xx xx xx 0x00
void USART1_IRQHandler(void){
    static uint8_t ReceiveFlag = 0;
    static uint8_t Count = 0;
    if(USART_GetITStatus(USARTx, USART_IT_RXNE) == SET){
        BlueTeeth_value = USART_ReceiveData(USARTx);
        if(BlueTeeth_value == 0xFF){
            ReceiveFlag = 1;
        }else if(ReceiveFlag == 1){
            if(BlueTeeth_value == 0x00){
                ReceiveFlag = 0;
                Count = 0;
            }
            else{
                Result[Count] = BlueTeeth_value;
                Count++;
            }
        }
    }
}






