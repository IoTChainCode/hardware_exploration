#ifndef __ONEWIRE_DHT11_H__
#define __ONEWIRE_DHT11_H__

//include
#include "stm32l4xx_hal.h"
//struct
typedef struct
{
	uint8_t  humi_high8bit;		//原始数据：湿度高8位
	uint8_t  humi_low8bit;	 	//原始数据：湿度低8位
	uint8_t  temp_high8bit;	 	//原始数据：温度高8位
	uint8_t  temp_low8bit;	 	//原始数据：温度高8位
	uint8_t  check_sum;	 	    //校验和
	float    humidity;        //实际湿度
	float    temperature;     //实际温度
}dht11_t;
//define
/***********************   DHT11 连接引脚定义  **************************/
#define DHT11_Dout_GPIO_CLK_ENABLE()              __HAL_RCC_GPIOA_CLK_ENABLE()
#define DHT11_Dout_PORT                           GPIOA
#define DHT11_Dout_PIN                            GPIO_PIN_11
/***********************   DHT11 函数宏定义  ****************************/
#define DHT11_Dout_LOW()                          HAL_GPIO_WritePin(DHT11_Dout_PORT,DHT11_Dout_PIN,GPIO_PIN_RESET) 
#define DHT11_Dout_HIGH()                         HAL_GPIO_WritePin(DHT11_Dout_PORT,DHT11_Dout_PIN,GPIO_PIN_SET)
#define DHT11_Data_IN()	                          HAL_GPIO_ReadPin(DHT11_Dout_PORT,DHT11_Dout_PIN)
#define Delay_ms(x)   HAL_Delay(x)
//function
void dht11_init(void);
uint8_t dht11_read(dht11_t *dht11);

#endif
