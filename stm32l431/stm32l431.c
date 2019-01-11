/*----------------------------------------------------------------------------
 * Copyright (c) <2016-2018>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "sys_init.h"

UINT32 g_task;
netdata_t g_netdata;//message data from at
static char g_uart[500];//global buffer for nbiot uart
store_t g_store;//store in last flash page,0xa8

//#if MODULE==MODULE_DHT11
dht11_t g_dht11;
msg_dht11_t g_msg_dht11;
//#elif MODULE==MODULE_SMOKE
msg_smoke_t g_msg_smoke;
//#elif MODULE==MODULE_GPS
gps_t g_gps;
msg_gps_t g_msg_gps;
float longitude;
float latitude;
static uint8 gps_uart[1000];
//#elif MODULE==MODULE_BH1750
msg_bh1750_t g_msg_bh1750;
//#endif

VOID data_collection_task(VOID)
{
	UINT32 uwRet;
	char *str = "ITC";

//#if MODULE==MODULE_DHT11
	dht11_init();
//#elif MODULE==MODULE_SMOKE		
	short int value;
	short int max=1000;
	g_msg_smoke.index=1;
//#elif MODULE==MODULE_GPS
//#elif MODULE==MODULE_BH1750
	short int lux;
	bh1750_init();
//#endif
	OLED_Init();
	OLED_Clear();
	OLED_ShowCHinese(0,0,0);
	OLED_ShowCHinese(18,0,1);
	OLED_ShowCHinese(36,0,2);
	OLED_ShowCHinese(54,0,3);
	OLED_ShowCHinese(72,0,4);
	OLED_ShowCHinese(90,0,5);
	OLED_ShowString(0,2,(uint8_t*)str,16);
	LOS_HwiCreate(EXTI0_IRQn, 2,0,EXTI0_IRQHandler,NULL);
	LOS_HwiCreate(EXTI1_IRQn, 3,0,EXTI1_IRQHandler,NULL);
	LOS_HwiCreate(EXTI2_IRQn, 4,0,EXTI2_IRQHandler,NULL);
	LOS_HwiCreate(EXTI3_IRQn, 5,0,EXTI3_IRQHandler,NULL);
	LOS_HwiCreate(USART2_IRQn, 6,0,USART2_IRQHandler,NULL);
	LOS_HwiCreate(USART3_IRQn, 7,0,USART3_IRQHandler,NULL);
	los_dev_uart_init(LOS_STM32L431_UART2,9600,g_uart,500);
	//data loop
	while(1)
	{
//#if MODULE==MODULE_DHT11 //temperature(℃) & humidity(%RH)
		if (dht11_read(&g_dht11)==SUCCESS)
			printf("Humidity:%.1f％RH,Temperature:%.1f℃\n",g_dht11.humidity,g_dht11.temperature);
		else
		{
			printf("Error\r\n");
			dht11_init();
		}
		sprintf(g_msg_dht11.temperature,"%.1f",g_dht11.temperature);
		sprintf(g_msg_dht11.humidity,"%.1f",g_dht11.humidity);
//#elif MODULE==MODULE_SMOKE
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 50);
		value = HAL_ADC_GetValue(&hadc1);
		printf("MQ2:%d\n",value);
		sprintf(g_msg_smoke.value,"%4d",value);
		sprintf(g_msg_smoke.max,"%4d",max);
//#elif MODULE==MODULE_GPS
		HAL_UART_Receive_IT(&huart3,gps_uart,1000);
		gps_analysis(&g_gps,(uint8_t*)gps_uart);	//分析字符串
		longitude=(float)((float)g_gps.longitude_bd/100000);	
		printf("Longitude:%.5f %lc\n",longitude,g_gps.ewhemi_bd);
		latitude=(float)((float)g_gps.latitude_bd/100000);
		printf("Latitude:%.5f %1c\n",latitude,g_gps.nshemi_bd);	
//#elif MODULE==MODULE_BH1750 //lux
		lux=(int)bh1750_convert();
		printf("BH1750:%d\n",lux);
		sprintf(g_msg_bh1750.lux, "%5d",lux);	
//#endif
		uwRet=LOS_TaskDelay(1500);
		if (uwRet!=LOS_OK)
			break;
	}
}

VOID data_transfer_task(VOID)
{
	UINT32 ret;
	
	at_reboot();//AT+NRB
	while(at_get_connect()<0);//AT+CGATT?
	at_set_cdp("139.159.140.34");//连接华为or电信平台
	while(1)
	{
		if (at_send_coap((const char*)&g_msg_dht11,sizeof(g_msg_dht11))>=0)
			printf("at_send_coap OK!\n");
		else
			printf("at_send_coap Fail!\n");
		uart_data_flush();
		memset(g_netdata.nmgr,0,5);
		at_recv_coap(g_netdata.nmgr,5);
		printf("%s\n",g_netdata.nmgr);
		/*
//#if MODULE==MODULE_DHT11 //temperature(℃) & humidity(%RH)
		if (at_send_coap((const char*)&g_msg_dht11,sizeof(g_msg_dht11))>=0)
			printf("at_send_coap OK!\n");
		else
			printf("at_send_coap Fail!\n");
		uart_data_flush();
		memset(g_netdata.nmgr,0,5);
		at_recv_coap(g_netdata.nmgr,5);
		printf("%s\n",g_netdata.nmgr);
		if (strcmp(g_netdata.nmgr,"ON")==0) //开灯
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);    // 输出高电平
		if (strcmp(g_netdata.nmgr,"OFF")==0) //关灯
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);  // 输出低电平
//#elif MODULE==MODULE_SMOKE
		g_msg_smoke.csq=at_get_csq();
		if (at_send_coap((const char*)&g_msg_smoke,sizeof(g_msg_smoke))>=0)		//发送数据到平台	
			printf("at_send_coap OK!\n");
		else
			printf("at_send_coap Fail!\n");
		uart_data_flush();
		memset(g_netdata.nmgr,0,5);
		at_recv_coap(g_netdata.nmgr,5);
		printf("%s\n",g_netdata.nmgr);
		if (strcmp(g_netdata.nmgr,"ON")==0)
			HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_SET);    //开蜂鸣器,输出高电平
		if (strcmp(g_netdata.nmgr,"OFF")==0)
			HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);  //关蜂鸣器,输出低电平
//#elif MODULE==MODULE_GPS
		if (latitude==0 || longitude==0)
			continue;
		memset(g_msg_gps.latitude, 0, 8);
		memset(g_msg_gps.longitude, 0, 9);
		sprintf(g_msg_gps.latitude, "%.5f", latitude);
		sprintf(g_msg_gps.longitude, "%.5f", longitude);		
		if (at_send_coap((const char*)&g_msg_gps,sizeof(g_msg_gps))>=0)//发送数据到平台	
			printf("at_send_coap OK!\n");
		else
			printf("at_send_coap Fail!\n");
		uart_data_flush();
		memset(g_netdata.nmgr, 0, 5);
		at_recv_coap(g_netdata.nmgr,5);
		printf("%s\n",g_netdata.nmgr);
		if (strcmp(g_netdata.nmgr,"ON")==0)
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);    //开灯,输出高电平
		if (strcmp(g_netdata.nmgr,"OFF")==0)
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);  //关灯,输出低电平
//#elif MODULE==MODULE_BH1750 //lux
		if (at_send_coap((const char*)&g_msg_bh1750,sizeof(g_msg_bh1750))>=0)//发送数据到平台
			printf("at_send_coap OK!\n");
		else
			printf("at_send_coap Fail!\n");
		uart_data_flush();
		memset(g_netdata.nmgr, 0, 5);
		at_recv_coap(g_netdata.nmgr,5);
		printf("%s\n",g_netdata.nmgr);
		if (strcmp(g_netdata.nmgr,"ON")==0)
			HAL_GPIO_WritePin(Light_GPIO_Port,Light_Pin,GPIO_PIN_RESET);    //开灯,输出低电平
		if (strcmp(g_netdata.nmgr,"OFF")==0)
			HAL_GPIO_WritePin(Light_GPIO_Port,Light_Pin,GPIO_PIN_SET);  //关灯,输出高电平
//#endif
		*/
		ret=LOS_TaskDelay(500);
		if (ret!=LOS_OK)
			break;
	}
}

UINT32 create_data_collection_task()
{
	UINT32 ret;
	TSK_INIT_PARAM_S task_init_param;

	task_init_param.usTaskPrio=0;
	task_init_param.pcName="data_collection_task";
	task_init_param.pfnTaskEntry=(TSK_ENTRY_FUNC)data_collection_task;
	task_init_param.uwStackSize=0x800;
	ret=LOS_TaskCreate(&g_task,&task_init_param);
	
	return ret;
}

UINT32 create_data_transfer_task()
{
	UINT32 ret;
	TSK_INIT_PARAM_S task_init_param;

	task_init_param.usTaskPrio=1;
	task_init_param.pcName="data_transfer_task";
	task_init_param.pfnTaskEntry=(TSK_ENTRY_FUNC)data_transfer_task;
	task_init_param.uwStackSize=0x400;
	ret=LOS_TaskCreate(&g_task,&task_init_param);
	
	return ret;
}

//check whether generated
//0-exist,1-not exist
uint8 ecc_check(void)
{
	g_store.flag=*(uint32 *)(ADDR_STORE+0x04);
	return g_store.flag==FLAG_GENERATED ? 0 : 1;
}

//gen key & id
//0-success,1-error
int ecc_gen(void) 
{
	uint32_t i;
	EccPoint l_public;
	#if 0 //ecdsa
	uint8_t l_private[NUM_ECC_DIGITS]={0x7a,0x61,0xd8,0xa1,0x32,0x6b,0x3a,0x0b,0x6b,0x1b,0x88,0xc8,0x0d,0xcc,0xe3,0x3e,0x3c,0x85,0xc4,0x25,0x96,0xc2,0xb0,0xc0};
    uint8_t l_hash[NUM_ECC_DIGITS]={0xec,0x5c,0x8d,0x08,0xe0,0x9a,0xe3,0x31,0x37,0x7c,0x47,0xf2,0x3f,0xe0,0x5a,0x4c,0x90,0x14,0x69,0xa0,0x89,0xa8,0xcc,0x2a};
    uint8_t l_random[NUM_ECC_DIGITS]={0x48,0x88,0xcf,0xac,0xcf,0xe6,0x2a,0x91,0x3d,0xac,0xb5,0xfc,0x59,0xbf,0x99,0xc4,0x81,0x3c,0x8b,0x55,0xdf,0x05,0x91,0xe7};
    uint8_t l_r[NUM_ECC_DIGITS]={0x5a,0x66,0x35,0x04,0x13,0x70,0xb9,0x02,0xe2,0xf9,0x04,0x8a,0x6c,0x13,0x45,0x5b,0x51,0x32,0x5d,0x8b,0x3a,0xb1,0x1e,0x08};
    uint8_t l_s[NUM_ECC_DIGITS]={0x88,0x03,0xd2,0x92,0xc1,0x3b,0x1a,0x48,0x26,0x49,0x89,0xcf,0x42,0xd2,0x5a,0x6e,0x1a,0x6c,0x52,0x60,0xd9,0xe7,0xfe,0x3f};
    uint8_t l_r_out[NUM_ECC_DIGITS];
    uint8_t l_s_out[NUM_ECC_DIGITS];
	uint8_t x[NUM_ECC_DIGITS]={0x10,0x1e,0x65,0x48,0x5b,0x4f,0x26,0x43,0x7c,0xd4,0x05,0x5a,0xd4,0x4b,0xec,0x08,0xe4,0x90,0x39,0x33,0x0d,0x19,0x79,0x68};
	uint8_t y[NUM_ECC_DIGITS]={0x69,0x65,0x1f,0xfd,0x58,0x77,0x26,0x16,0x72,0x91,0xa4,0x59,0x07,0x48,0xbb,0x97,0xa3,0xf5,0x4f,0x11,0x8f,0x0c,0x95,0xaf};
	#endif
	#if 0 //ecdsa+drng
	uint8_t l_private[NUM_ECC_DIGITS];
	uint8_t l_hash[NUM_ECC_DIGITS]={0xec,0x5c,0x8d,0x08,0xe0,0x9a,0xe3,0x31,0x37,0x7c,0x47,0xf2,0x3f,0xe0,0x5a,0x4c,0x90,0x14,0x69,0xa0,0x89,0xa8,0xcc,0x2a};
	uint8_t l_random[NUM_ECC_DIGITS];
	uint8_t l_r_out[NUM_ECC_DIGITS];
    uint8_t l_s_out[NUM_ECC_DIGITS];
	uint8_t seed[32];
	#endif
	#if 0 //ecdsa+drng+sha256
	uint8_t l_message[4]={0x35,0x33,0x37,0x36};
	uint8_t l_private[NUM_ECC_DIGITS];
	uint8_t l_hash[NUM_ECC_DIGITS];
	uint8_t l_random[NUM_ECC_DIGITS];
	uint8_t l_r_out[NUM_ECC_DIGITS];
    uint8_t l_s_out[NUM_ECC_DIGITS];
	//uint8_t seed[32];
	uint32_t seed[8];
	RNG_HandleTypeDef handle_rng;
	#endif
	#if 1 //ecdsa+drng+sha256+sha3(id)
	//uint8_t l_message[4]={0x35,0x33,0x37,0x36};
	//uint8_t l_message[8]={0x00,0x00,0x01,0x20,0x09,0x00,0x00,0x08};
	//uint8_t l_private[NUM_ECC_DIGITS];
	//uint8_t l_hash[NUM_ECC_DIGITS];
	uint8_t l_random[NUM_ECC_DIGITS];
	//uint8_t l_r_out[NUM_ECC_DIGITS];
    //uint8_t l_s_out[NUM_ECC_DIGITS];
	uint32_t seed[8];//uint8_t seed[32];
	uint8_t hash[NUM_ECC_DIGITS]={0xc5,0xd2,0x46,0x01,0x86,0xf7,0x23,0x3c,0x92,0x7e,0x7d,0xb2,0xdc,0xc7,0x03,0xc0,0xe5,0x00,0xb6,0x53,0xca,0x82,0x27,0x3b,0x7b,0xfa,0xd8,0x04,0x5d,0x85,0xa4,0x70};//null
	RNG_HandleTypeDef handle_rng;
	//uint8_t pubkey[2*NUM_ECC_DIGITS];
	//uint8_t id[NUM_ECC_DIGITS];
	FLASH_EraseInitTypeDef flash;
	uint32_t error;
	uint64_t *point;
	#endif
		
	//hash
	//sha256_init();
	//sha256_update(l_message,4);
	//sha256_final(l_hash);
	//sha3_init(NUM_ECC_DIGITS*8);
	//sha3_update(l_message,8*8);
	//sha3_final(l_hash);
	//trng
	memset(seed,0,32);
	__HAL_RCC_HSI48_ENABLE();
	__HAL_RCC_RNG_CLK_ENABLE();//ahb2enr.rngen=enable
	handle_rng.Instance = RNG;
	if (HAL_RNG_Init(&handle_rng)!= HAL_OK)
		return 1;
	for (i=0;i<8;i++)
		HAL_RNG_GenerateRandomNumber(&handle_rng,&seed[i]);
	if (HAL_RNG_DeInit(&handle_rng)!=HAL_OK)
		return 1;
	__HAL_RCC_RNG_CLK_DISABLE();
	__HAL_RCC_HSI48_DISABLE();
	//drbg+aes128
	ctr_init((uint8_t*)seed);
	ctr_generate(NUM_ECC_DIGITS*8,g_store.key_pri);
	ctr_generate(NUM_ECC_DIGITS*8,l_random);
	//gen key
	ecc_make_key(&l_public,g_store.key_pri,g_store.key_pri);
	if (!ecc_valid_public_key(&l_public))
        return 1;
    //if (!ecdsa_sign(l_r_out,l_s_out,g_store.key_pri,l_random,l_hash))
    //    return 1;
    //if (!ecdsa_verify(&l_public,l_hash,l_r_out,l_s_out))
    //    return 1;
	//gen id
	memcpy(g_store.key_pub,l_public.x,NUM_ECC_DIGITS);
	memcpy(&g_store.key_pub[NUM_ECC_DIGITS],l_public.y,NUM_ECC_DIGITS);
	sha3_init(NUM_ECC_DIGITS*8);
	sha3_update(g_store.key_pub,2*NUM_ECC_DIGITS*8);
	sha3_final(g_store.id);
	//write to store area
	g_store.reserved=0;
	g_store.flag=FLAG_GENERATED;
	g_store.len_normal=0;
	g_store.len_ota=0;
	memcpy(g_store.hash,hash,NUM_ECC_DIGITS);//null
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);
    flash.TypeErase=FLASH_TYPEERASE_PAGES;
    flash.Page=PAGE_STORE;//last page
    flash.NbPages=1;
	HAL_FLASHEx_Erase(&flash,&error);
	point=(uint64_t *)&g_store;
	for (i=0;i<sizeof(store_t)/8;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_STORE+i*8,point[i]);
	HAL_FLASH_Lock();
	
	return 0;
}

int main(void)
{
	uint32 ret;

	__HAL_RCC_MSI_RANGE_CONFIG(RCC_MSIRANGE_10);//ahb clk=32mhz,rng clk=48 mhz
	//check whether key & id generated
	if (ecc_check())
	{
		//gen key & id
		ret=ecc_gen();
		if (ret)
			return LOS_NOK;
	}
	//check local(normal area)
	ret=ota_checklocal();
	if (ret!=OTA_LATEST)
	{
		//ota area->normal area
		while(1)
		{
			ret=ota_move();
			if (ret==OTA_LATEST)
				break;
		}
	}
	//check remote(ota area),should be placed at command line
	ret=ota_checkremote();
	if (ret!=OTA_LATEST)
	{
		//server->ota area
		while(1)
		{
			ret=ota_download();
			if (ret==OTA_LATEST)
				break;
		}
		//ota area->normal area
		while(1)
		{
			ret=ota_move();
			if (ret==OTA_LATEST)
				break;
		}
	}
	//initial hardware(hal,clk,uart,adc,i2c,gpio)
	HAL_Init();
	SystemClock_Config();
	DelayInit();
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_USART3_UART_Init();
	MX_ADC1_Init();
	MX_I2C1_Init();
	//initial os
	ret=LOS_KernelInit();
	if (ret!=LOS_OK)
		return LOS_NOK;
	//create task(data collection & data transfer)
	ret=create_data_collection_task();
	if (ret!=LOS_OK)
		return LOS_NOK;
	ret=create_data_transfer_task();
	if (ret!=LOS_OK)
		return LOS_NOK;
	//start os
	LOS_Start();
	
	return LOS_OK;
}
