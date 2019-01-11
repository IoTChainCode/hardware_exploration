/**
  ******************************************************************************
  * @file           : stm32l431.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#ifndef __STM32L431_H__
#define __STM32L431_H__

//include
#include "los_base.h"
#include "los_config.h"
#include "los_sys.h"
#include "los_typedef.h"
#include "los_task.ph"
#include "los_event.ph"
#include "los_event.h"
//define
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define Light_Pin GPIO_PIN_5
#define Light_GPIO_Port GPIOA
#define KEY1_Pin GPIO_PIN_0
#define KEY1_GPIO_Port GPIOC
#define KEY1_EXTI_IRQn EXTI0_IRQn
#define KEY2_Pin GPIO_PIN_1
#define KEY2_GPIO_Port GPIOC
#define KEY2_EXTI_IRQn EXTI1_IRQn
#define KEY3_Pin GPIO_PIN_2
#define KEY3_GPIO_Port GPIOC
#define KEY3_EXTI_IRQn EXTI2_IRQn
#define KEY4_Pin GPIO_PIN_3
#define KEY4_GPIO_Port GPIOC
#define KEY4_EXTI_IRQn EXTI3_IRQn
#define MQ2_DO_Pin GPIO_PIN_6
#define MQ2_DO_GPIO_Port GPIOA
#define OLED_SCL_Pin GPIO_PIN_10
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_11
#define OLED_SDA_GPIO_Port GPIOB
#define Beep_Pin GPIO_PIN_5
#define Beep_GPIO_Port GPIOB
//struct
typedef struct
{
	char time[30];
	char csq[5];
	char cfun[5];
	char ip[20];
	char cscon[5];
	char cereg[5];
	char cgatt[5];
	char npsmr[5];
	char nsocr[10];
	char nsocl[10];
	char nsost[30];
	char nsonmi[10];
	char nsorf[30];
	char nmgr[30];
	char signal_power[10];
	char total_power[10];
	char tx_power[10];
	char tx_time[10];
	char rx_time[10];
	char scell_id[10];
	char ecl[10];
	char snr[10];
	char earfcn[10];
	char pci[10];
	char rsrq[10];
}netdata_t;
extern netdata_t g_netdata;
typedef struct
{
	char temperature[4];
	char humidity[4];
}msg_dht11_t;
typedef struct
{
	char index;
	unsigned char csq;
	char value[4];
	char max[4];		
}msg_smoke_t;
typedef struct
{
	char latitude[8];
	char longitude[9];	
}msg_gps_t;
typedef struct
{
	char lux[5];
}msg_bh1750_t;
#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);
#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
