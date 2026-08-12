#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "DHT11.h"
#include "OLED.h"
#include "BlueTeeth.h"
uint8_t T1, T2, W1, W2;
uint8_t Chack(void);
GPIO_InitTypeDef GPIO_InitStructure;
void Mode(uint8_t i);
//初始化要发送旗帜信号，用于判断是否初始成功
uint8_t DHT_Init(void){
    RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE); 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // 设置为输出模式， 用于拉高电平和拉低电平
    GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_WriteBit(DHT11_GPIOx, DHT11_GPIO_Pin, Bit_RESET); //拉低
    Delay_ms(120);                              // 上电延时
    GPIO_WriteBit(DHT11_GPIOx, DHT11_GPIO_Pin, Bit_RESET);//拉低
    Delay_ms(20);                               //至少20秒

    
    Mode(1); // 切换为输入模式, 用于检测外部引脚电平
    
    uint8_t time = 0;
    while(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == SET){
        time++;
        Delay_us(1);
        if(time > 100) return 0;
    }
    time = 0;
    while(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == RESET){
        time++;
        Delay_us(1);
        if(time > 100) return 0;
    }
    time = 0;
    while(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == SET){
        time++;
        Delay_us(1);
        if(time > 100) return 0;
    }
    return 1;

}

//用于切换DAT的输入输出模式， 1 为输入模式， 0 为输出模式
void Mode(uint8_t i){
    if(i == 1){
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    }else{
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    }
    GPIO_Init(DHT11_GPIOx, &GPIO_InitStructure);
}

uint8_t Judge_0_1(void){
    uint8_t time = 0; 
    while(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == RESET);  //等待低电平结束
    Delay_us(30);
    if(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == SET){
        while(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == SET);
        return 1;
    }else{
        return 0;
    }
}
//读一个字节
uint8_t ReadByte(void){
    uint8_t temp = 0;
    Mode(1);
    for(uint8_t i = 0; i < 8; i++){
        temp <<= 1;
        temp |= Judge_0_1();
    }
    return temp;
}


void SaveData(uint8_t *T, uint8_t *T2, uint8_t *W, uint8_t *W2){

        
        uint8_t Data[5];
    
        if(DHT_Init() == 0) return;
    
        for(uint8_t i = 0; i < 5; i++){
            Data[i] = ReadByte();
        }   
        Mode(0);
        if(Data[0] + Data[1] + Data[2] + Data[3] == Data[4]){
            *W = Data[0];
            *W2 = Data[1];
            *T = Data[2];
            *T2 = Data[3];
        }
        
}

void DHT11_Result(void){
    SaveData(&T1, &T2, &W1, &W2);
    if((T2 >> 8) == 1){
        OLED_ShowChar(2,1,'-');
        T2 <<= 1;
        T2 >>= 1;
    }
    OLED_ShowNum(2,2, T1, 2);
    OLED_ShowChar(2,5,'.');
    OLED_ShowNum(2, 6, T2, 2);
    OLED_ShowString(1, 1, "Temputer");
    OLED_ShowString(3, 1, "Humidity");
    OLED_ShowNum(4, 1, W1, 2);
    Delay_ms(200);
}

void DHT11_BlueTeeth_Sent(void){
    SaveData(&T1, &T2, &W1, &W2);
    BlueTeeth_SentByte(0xFF);
    BlueTeeth_SentByte(T1);
    BlueTeeth_SentByte(T2);
    BlueTeeth_SentByte(W1);
    BlueTeeth_SentByte(W2);
    //BlueTeeth_SentByte(0x00);
}

void DHT11_BlueTeeth_Result(void){
    
    OLED_ShowNum(2,1, Result[0], 2);
    OLED_ShowChar(2,3,'.');
    OLED_ShowNum(2, 4, Result[1], 2);
    OLED_ShowString(2,6,"      ");
    OLED_ShowString(1, 1, "Temputer");
    OLED_ShowString(3, 1, "Humidity");
    OLED_ShowNum(4, 1, Result[2], 2);
    Delay_ms(200);
}



