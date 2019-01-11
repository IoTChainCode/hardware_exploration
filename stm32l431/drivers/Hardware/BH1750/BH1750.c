//bh1750:data sample of light lux
#include "bh1750.h"
#include "stm32l4xx.h"
#include "delay.h"

float result_lx=0;
uint8_t BUF[2]={0};
uint16_t result=0;

//config I2C's GPIO(use analogue)
static void I2C_InitGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	//open gpio clk
	I2C_GPIO_CLK_ENABLE();
	GPIO_InitStruct.Pin = I2C_SCL_PIN|I2C_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH ;
	HAL_GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStruct);
	I2C_SCL(HIGH); 
	I2C_SDA(HIGH); 
}

//CPU send I2C start signal
void I2C_Start(void)
{
	//��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C���������ź�
	SDA_OUT();
	I2C_SDA(HIGH);
	I2C_SCL(HIGH); 
	delay_us(4);
	I2C_SDA(LOW);
	delay_us(4);
	I2C_SCL(LOW); 
}

//CPU send I2C stop signal
void I2C_Stop(void)
{
	//��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C����ֹͣ�ź�
	SDA_OUT();
	I2C_SCL(LOW);
	I2C_SDA(LOW); 
	delay_us(4);
	I2C_SCL(HIGH);	
	I2C_SDA(HIGH);
	delay_us(4);
}

//CPU gen a clk and read ACK
//0-ok,1-no response
uint8_t I2C_WaitAck(void)
{
	uint8_t ucErrTime=0;
	
	SDA_IN(); 
	I2C_SDA(HIGH);//CPU�ͷ�SDA����
	delay_us(4);
	I2C_SCL(HIGH);//CPU����SCL = 1, ��ʱ�����᷵��ACKӦ��
	delay_us(4);
	while (I2C_SDA_READ())//CPU��ȡSDA����״̬
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			I2C_Stop();
			return 1;
		}
	}
	I2C_SCL(LOW); 
	
	return 0;
}

//CPU gen an ACK
void I2C_Ack(void)
{
	I2C_SCL(LOW); 
	SDA_OUT(); 
	I2C_SDA(LOW);//CPU����SDA = 0
	delay_us(2);
	I2C_SCL(HIGH);//CPU����1��ʱ��
	delay_us(2);
	I2C_SCL(LOW); 
}

//CPU gen a NACK
void I2C_NAck(void)
{
	I2C_SCL(LOW); 
	SDA_OUT();
	I2C_SDA(HIGH);//CPU����SDA = 1
	delay_us(2);
	I2C_SCL(HIGH);//CPU����1��ʱ��
	delay_us(2);
	I2C_SCL(LOW); 
}

//CPU send 8bit to I2C
//Byte:data to send out
void I2C_SendByte(uint8_t Byte)
{
	uint8_t t;  
      
    SDA_OUT();//sda
    I2C_SCL(LOW);
    for (t = 0; t < 8; t++)  
    {  
        I2C_SDA((Byte & 0x80) >> 7);  
        Byte <<= 1;  
        delay_us(2);  
        I2C_SCL(HIGH);  
        delay_us(2);  
        I2C_SCL(LOW);  
        delay_us(2);  
    } 
}

//CPU read 8bit from I2C(bit7 first)
//uint8_t:data recv in
uint8_t I2C_ReadByte(uint8_t ack)
{
	uint8_t i;
	uint8_t value;
	
	SDA_IN();
	value = 0;
	for (i = 0; i < 8; i++)  
    {  
        I2C_SCL(LOW);  
        delay_us(2);  
        I2C_SCL(HIGH);  
        value <<= 1;  
        if (I2C_SDA_READ() ) value++;  
        delay_us(1);  
    }  
	if (!ack)
        I2C_NAck();//send nACK
    else
        I2C_Ack();//send ACK  

	return value;
}

//check I2C device. CPU send address to device, then read ACK to judge the existence(I2C_CheckDevice to check device and config GPIO)
//Address:device's I2C address
//uint8_t:0-exist,1-not exist
uint8_t I2C_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;

	I2C_InitGPIO();//����GPIO
	I2C_Start();//���������ź�
	//�����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ�
	I2C_SendByte(_Address | I2C_WR);
	ucAck = I2C_WaitAck();//����豸��ACKӦ��
	I2C_Stop();//����ֹͣ�ź�

	return ucAck;
}

void Cmd_Write_BH1750(uint8_t cmd)
{
    I2C_Start();//��ʼ�ź�
    I2C_SendByte(BH1750_Addr+0);//�����豸��ַ+д�ź�
	while(I2C_WaitAck());
	delay_us(100);
    I2C_SendByte(cmd);//�ڲ��Ĵ�����ַ
	while(I2C_WaitAck());
	delay_us(100);
    I2C_Stop();//����ֹͣ�ź�
	delay_ms(5);
}

//bh1750 init
void bh1750_init(void)
{
	I2C_InitGPIO();	
	Cmd_Write_BH1750(0x01);
}

//bh1750 start
void bh1750_start(void)
{
	Cmd_Write_BH1750(BH1750_ON);	 //power on
	Cmd_Write_BH1750(BH1750_CON);
}

//bh1750 read
void bh1750_read(void)
{
	uint8_t i;
	
    I2C_Start();//��ʼ�ź�
    I2C_SendByte(BH1750_Addr+1);//�����豸��ַ+���ź�
	while(I2C_WaitAck());
	for (i=0; i<3; i++)
    {     
        if (i==3)
			BUF[i]=I2C_ReadByte(0); 
        else
			BUF[i]=I2C_ReadByte(1);//����ACK
	}
	//����NACK
    I2C_Stop();//ֹͣ�ź�
    delay_ms(5);
}

//bh1750 convert
float bh1750_convert(void)
{
	bh1750_start();
	delay_ms(180);
	bh1750_read();
	result=BUF[0];
	result=(result<<8)+BUF[1];//�ϳ����ݣ�����������	
	result_lx=(float)(result/1.2);

	return result_lx;
}
