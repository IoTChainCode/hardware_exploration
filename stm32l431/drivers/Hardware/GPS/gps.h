#ifndef __gps_h__
#define __gps_h__

#include "stm32l4xx_hal.h"
#include "string.h"
#include "stdio.h"

//struct
__packed typedef struct
{
	uint32_t latitude_bd;					//γ�� ������100000����ʵ��Ҫ����100000
	uint8_t nshemi_bd;						//��γ/��γ,N:��γ;S:��γ	
	uint32_t longitude_bd;			    	//���� ������100000��,ʵ��Ҫ����100000
	uint8_t ewhemi_bd;						//����/����,E:����;W:����
}gps_t;//GPS NMEA-0183Э��,������Ϣ
//function
void gps_init(void);
void gps_analysis(gps_t *gps,uint8_t *buf);

#endif
