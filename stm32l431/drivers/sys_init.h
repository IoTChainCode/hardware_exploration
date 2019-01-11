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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYS_H_
#define __SYS_H_

//LiteOS
#include "los_base.h"
#include "los_config.h"
#include "los_sys.h"
#include "los_typedef.h"
#include "los_task.ph"
#include "los_hwi.h"
#include "los_sem.h"
#include "los_event.h"
#include "los_memory.h"
#include "los_queue.ph"
//evb board
#include "stdlib.h"
#include "string.h"
#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "stm32l4xx_it.h"
#include "delay.h"
#include "adc.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "oled.h"
#include "at.h"
#include "los_dev_st_uart.h"
//sensor
#include "DHT11_BUS.h"
#include "BH1750.h" 
#include "gps.h"
//algorithm
#include "ecc.h"
#include "drng.h"
#include "crypt_sha256.h"
#include "crypt_sha3.h"
//
#include "ota.h"
#include "include.h"
//struct
typedef struct
{
	uint32 reserved;
	uint32 flag;//whether generated
	uint32 len_normal;
	uint32 len_ota;//server offered(compare with normal area & server)
	uint8 hash[NUM_ECC_DIGITS];//server offered(compare with normal area & server)
	uint8 key_pub[NUM_ECC_DIGITS*2];//x+y
	uint8 key_pri[NUM_ECC_DIGITS];
	uint8 id[NUM_ECC_DIGITS];
}store_t;

#ifdef __cplusplus
 extern "C" {
#endif

uint32_t HAL_GetTick(void);
void SystemClock_Config(void);
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif
