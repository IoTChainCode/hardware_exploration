//dht11:data sample of temperature & humidity
#include "DHT11_BUS.h"

//delay us
static void DHT11_Delay(uint16_t time)
{
	uint8_t i;
	
	while(time)
	{
		for(i=0;i<10;i++);
		time--;
	}
}

//DHT11-DATA,input pin as pull up
static void DHT11_Mode_IPU(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  
	GPIO_InitStruct.Pin   = DHT11_Dout_PIN;
	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(DHT11_Dout_PORT, &GPIO_InitStruct);
}

//DHT11-DATA,output pin as push out
static void DHT11_Mode_Out_PP(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  
	GPIO_InitStruct.Pin = DHT11_Dout_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DHT11_Dout_PORT, &GPIO_InitStruct); 	 
}

//read 1 byte from DHT11(MSB)
static uint8_t DHT11_ReadByte(void)
{
	uint8_t i,temp=0;

	for (i=0;i<8;i++)
	{
		//each bit start as 50us low level, polling until slave send out 50us low level
		while(DHT11_Data_IN()==GPIO_PIN_RESET);
		//DHT11 use 26~28us high level as "0", use 70us high level as "1". Detect delay is x=40us
		DHT11_Delay(40);//延时x us这个延时需要大于数据0持续的时间即可
		if (DHT11_Data_IN()==GPIO_PIN_SET)//x us后仍为高电平表示数据“1”
		{
			while(DHT11_Data_IN()==GPIO_PIN_SET);//等待数据1的高电平结束
			temp|=(uint8_t)(0x01<<(7-i));//把第7-i位置1，MSB先行 
		}
		else//x us后为低电平表示数据“0”
			temp&=(uint8_t)~(0x01<<(7-i)); //把第7-i位置0，MSB先行
	}
	
	return temp;
}

//dht11 init
void dht11_init(void)
{
	DHT11_Dout_GPIO_CLK_ENABLE();
	DHT11_Mode_Out_PP();
	DHT11_Dout_HIGH();//pull up GPIO
}

//Whole data transfer 40bit, high bit first
//dht11_t dht11:input param
//return:SUCCESS-ok,ERROR-err
uint8_t dht11_read(dht11_t *dht11)
{
	uint8_t temp;
	uint16_t humi_temp;
  
	//config dht11(before read)
	DHT11_Mode_Out_PP();
	DHT11_Dout_LOW();
	Delay_ms(18);
	DHT11_Dout_HIGH();
	DHT11_Delay(30);
	DHT11_Mode_IPU();
	//judge slave's signal,no response then exit, response then go next
	if (DHT11_Data_IN()!=GPIO_PIN_RESET)
		return ERROR;
    //polling until slave send out 80us low level
    while(DHT11_Data_IN()==GPIO_PIN_RESET);
    while(DHT11_Data_IN()==GPIO_PIN_SET);
    //recv data
    dht11->humi_high8bit= DHT11_ReadByte();
    dht11->humi_low8bit = DHT11_ReadByte();
    dht11->temp_high8bit= DHT11_ReadByte();
    dht11->temp_low8bit = DHT11_ReadByte();
    dht11->check_sum    = DHT11_ReadByte();
    //config dht11(after read)
    DHT11_Mode_Out_PP();
    DHT11_Dout_HIGH();
    //check data
    humi_temp=dht11->humi_high8bit*100+dht11->humi_low8bit;
    dht11->humidity=(float)humi_temp/100;
    humi_temp=dht11->temp_high8bit*100+dht11->temp_low8bit;
    dht11->temperature=(float)humi_temp/100;
    temp=dht11->humi_high8bit+dht11->humi_low8bit+dht11->temp_high8bit+dht11->temp_low8bit;
    if (dht11->check_sum!=temp)
		return ERROR;
	
	return SUCCESS;
}
