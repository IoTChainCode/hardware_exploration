#ifndef __BH1750_H__
#define __BH1750_H__

//include
#include "stm32l4xx_hal.h"
//define
#define I2C_OWN_ADDRESS                            0x0A

#define I2C_WR	        0		/* 写控制bit */
#define I2C_RD	        1		/* 读控制bit */

#define I2C_GPIO_CLK_ENABLE()               __HAL_RCC_GPIOC_CLK_ENABLE()
#define I2C_GPIO_PORT                       GPIOC   
#define I2C_SCL_PIN                         GPIO_PIN_11
#define I2C_SDA_PIN                         GPIO_PIN_10

//高低电平
#define HIGH    1  
#define LOW     0 

#define I2C_SCL(n) (n?HAL_GPIO_WritePin(I2C_GPIO_PORT,I2C_SCL_PIN,GPIO_PIN_SET):HAL_GPIO_WritePin(I2C_GPIO_PORT,I2C_SCL_PIN,GPIO_PIN_RESET)) //SCL  
#define I2C_SDA(n) (n?HAL_GPIO_WritePin(I2C_GPIO_PORT,I2C_SDA_PIN,GPIO_PIN_SET):HAL_GPIO_WritePin(I2C_GPIO_PORT,I2C_SDA_PIN,GPIO_PIN_RESET)) //SDA 
#define I2C_SDA_READ()               HAL_GPIO_ReadPin(I2C_GPIO_PORT,I2C_SDA_PIN)

#define BH1750_Addr 0x46
#define BH1750_ON   0x01
#define BH1750_CON  0x10
#define BH1750_ONE  0x20
#define BH1750_RSET 0x07

#define SDA_IN()  {GPIOC->MODER&=~(3<<(10*2));GPIOC->MODER|=0<<10*2;}       //PH5????  
#define SDA_OUT() {GPIOC->MODER&=~(3<<(10*2));GPIOC->MODER|=1<<10*2;}   //PH5???? 
//function
void bh1750_init(void);								  //IO初始化，
void bh1750_start(void);			//上电，设置清除数据寄存器
void bh1750_read(void);                               //连续的读取内部寄存器数据
float bh1750_convert(void);

#endif
