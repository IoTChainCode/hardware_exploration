#ifndef _OTA_H_
#define _OTA_H_

//include
#include "include.h"
#include "stm32l4xx_hal.h"
//define
#define OTA_LATEST 0x12345678
#define OTA_UPDATE 0x9abcdef0

#define FLAG_GENERATED 0x12345678
#define FLAG_NONE  0x9abcdef0
//function
uint32 ota_checklocal(void);
uint32 ota_checkremote(void);
uint32 ota_download(void);
uint32 ota_move(void);

#endif
