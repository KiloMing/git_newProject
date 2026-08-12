#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "DHT11.h"
#include "OLED.h"
#include "BlueTeeth.h" 
uint8_t T1, T2, W1, W2;  //用于存储数据
GPIO_InitTypeDef GPIO_InitStructure;
void Mode(uint8_t i);    //修改GPIO的模式
//初始化要发送旗帜信号，用于判断是否初始成功
uint8_t DHT_Init(void){
    RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE); 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // 设置为输出模式， 用于拉高电平和拉低电平，此步骤对应上文 步骤1 
    GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //  对应步骤2，主机发送信号，拉低低电平至少18ms， 切换GPIO引脚模式
    GPIO_WriteBit(DHT11_GPIOx, DHT11_GPIO_Pin, Bit_RESET); //拉低
    Delay_ms(120);                              // 上电延时      
    GPIO_WriteBit(DHT11_GPIOx, DHT11_GPIO_Pin, Bit_RESET);//拉低
    Delay_ms(20);                               //至少20微秒
    Mode(1); // 切换为输入模式, 用于检测外部引脚电平
    
/*
    以下过程对应上文的 步骤3 ，从机响应信号，此时的电平为低电平持续87微秒，
    但是这里写了等待100us，如果在100us后从机还没有发送低电平信号那么就返回0，
    说明从机应答失败；从机拉高电平持续87us， 接着检测从机是否输出高电平，原理同上，
*/
    
    uint8_t time = 0;
    while(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == SET){    //是否拉低
        time++;                                                          
        Delay_us(1);                                                     
        if(time > 100) return 0;
    }
    time = 0;
    while(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == RESET){ // 检测拉低时间是否为83us（不超过100us）
        time++;
        Delay_us(1);
        if(time > 100) return 0;
    }
    time = 0;
    while(GPIO_ReadInputDataBit(DHT11_GPIOx, DHT11_GPIO_Pin) == SET){ //检测拉高时间是否为为87us（不超过100us）
        time++;
        Delay_us(1);
        if(time > 100) return 0;
    }
    return 1;   //返回 1 则证明主机和从机的通信正常

}

//用于切换DATA的输入输出模式， 1 为输入模式， 0 为输出模式
void Mode(uint8_t i){
    if(i == 1){
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
    }else{
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    }
    GPIO_Init(DHT11_GPIOx, &GPIO_InitStructure);
}


// 判断数据为 1 或 0，对应步骤4
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

//读取DHT11发送的40位数据
void SaveData(uint8_t *T, uint8_t *T2, uint8_t *W, uint8_t *W2){
        uint8_t Data[5];
        if(DHT_Init() == 0) return;    //主机从机应答失败直接返回，不执行任何操作
//      依次读取5个字节
        for(uint8_t i = 0; i < 5; i++){
            Data[i] = ReadByte();
        }   
        Mode(0);
//      校验位检测，如果数据正确则输出对应结果
        if(Data[0] + Data[1] + Data[2] + Data[3] == Data[4]){
            *W = Data[0];
            *W2 = Data[1];
            *T = Data[2];
            *T2 = Data[3];
        }
        
}
//本地显示温度湿度结果，多应用于DHT11检测，可以在本地接OLED显示屏来显示传感器的检测情况
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

//蓝牙发送数据    具体发送格式见  蓝牙模块部分
void DHT11_BlueTeeth_Sent(void){
    SaveData(&T1, &T2, &W1, &W2);
    BlueTeeth_SentByte(0xFF);
    BlueTeeth_SentByte(T1);
    BlueTeeth_SentByte(T2);
    BlueTeeth_SentByte(W1);
    BlueTeeth_SentByte(W2);
    //BlueTeeth_SentByte(0x00);
}

//OLED蓝牙接收端显示数据
//将HC08接收到的数据显示在OLED上
void DHT11_BlueTeeth_ShowResult(void){
    
    OLED_ShowNum(2,1, Result[0], 2);
    OLED_ShowChar(2,3,'.');
    OLED_ShowNum(2, 4, Result[1], 2);
    OLED_ShowString(2,6,"      ");
    OLED_ShowString(1, 1, "Temputer");
    OLED_ShowString(3, 1, "Humidity");
    OLED_ShowNum(4, 1, Result[2], 2);
    Delay_ms(200);
}



