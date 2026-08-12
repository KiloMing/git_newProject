#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "DHT11.h"
#include "MQ7.h"
#include "BlueTeeth.h"
uint8_t flag = 1;
int main(){
    BlueTeeth_Init();
    OLED_Init();
    Key_Init();
	flag = DHT_Init();
    MQ7_Init();
    if(!flag) OLED_ShowString(1,1,"error");
	uint8_t ShowFlag = 1;
    
    while(flag){
        SaveData(&T1, &T2, &W1, &W2);
        DHT11_Result();
        SaveData(&T1, &T2, &W1, &W2);
        BlueTeeth_SentByte(0xFF);
        BlueTeeth_SentByte(T1);
        BlueTeeth_SentByte(T2);
        BlueTeeth_SentByte(W1);
        BlueTeeth_SentByte((MQ7_GetData() >> 8) & 0xFF);
        BlueTeeth_SentByte(MQ7_GetData() & 0xFF);
        BlueTeeth_SentByte(0x00);
    }
    
}
