#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "DHT11.h"
#include "MQ7.h"
#include "BlueTeeth.h"
#include "TIM.h"
uint8_t flag = 1;
/****************说明******************/
/*
此函数为主机接收从机发送的信号，在主机端进行OLED轮询显示，周期为10s
*/



int main(){
    BlueTeeth_Init();
    OLED_Init();
    TIM_Init();
    uint8_t ClearFlag1 = 1;         
    uint8_t ClearFlag2 = 1;         //OLED刷新标志位
    while(flag){

        if(ShowResultFlag){
            if(ClearFlag1 == 1){
                OLED_Clear();
                ClearFlag1 = 0;
                ClearFlag2 = 1;
            }
            DHT11_BlueTeeth_ShowResult();   //DHA11温湿度信信息显示             
        }else{
            if(ClearFlag2 == 1){
                OLED_Clear();
                ClearFlag1 = 1;
                ClearFlag2 = 0;
                MQ7_BlueTeeth_ShowResult();  //
            }
            
           
        }
        
        
        //MQ7_BlueTeeth_ShowResult();
        //MQ7_Result();

        
        //OLED_ShowNum(1,1, 55, 3);
    }
    
}
