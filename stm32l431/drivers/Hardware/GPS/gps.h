#ifndef __gps_h__
#define __gps_h__

#include "stm32l4xx_hal.h"
#include "string.h"
#include "stdio.h"

//struct
__packed typedef struct
{
	uint32_t latitude_bd;					//纬度 分扩大100000倍，实际要除以100000
	uint8_t nshemi_bd;						//北纬/南纬,N:北纬;S:南纬	
	uint32_t longitude_bd;			    	//经度 分扩大100000倍,实际要除以100000
	uint8_t ewhemi_bd;						//东经/西经,E:东经;W:西经
}gps_t;//GPS NMEA-0183协议,卫星信息
//function
void gps_init(void);
void gps_analysis(gps_t *gps,uint8_t *buf);

#endif
