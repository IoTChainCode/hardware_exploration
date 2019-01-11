#include "ota.h"
#include "sys_init.h"

extern store_t g_store;//store in last flash page,0xa8

//check whether normal should be update
//uint32:OTA_LATEST-already latest,OTA_UPDATE-need update
uint32 ota_checklocal(void)
{
	uint8 hash[0x20];
	
	//1.read store area
	memcpy(&g_store,(uint8 *)ADDR_STORE,sizeof(store_t));
	//2.check length & hash(normal and ota)
	if (g_store.len_normal!=g_store.len_ota)
		return OTA_UPDATE;
	sha3_init(NUM_ECC_DIGITS*8);
	sha3_update((uint8 *)ADDR_NORMAL,g_store.len_normal*8);
	sha3_final(hash);
	if (memcmp(hash,(uint8 *)(ADDR_STORE+0x10),0x20))
		return OTA_UPDATE;
	
	return OTA_LATEST;
}

//check whether ota should be update
//uint32:OTA_LATEST-already latest,OTA_UPDATE-need update
uint32 ota_checkremote(void)
{
	EccPoint key_pub;
	uint32 length=0x08;
	uint8 hash[0x20]={0x23,0x3b,0x79,0x94,0xab,0xa6,0xde,0x2e,0xf1,0xec,0x65,0x23,0xe0,0x81,0x25,0xcc,0x3c,0x64,0x9b,0x45,0xe1,0x40,0xaf,0xc8,0x88,0x1a,0x22,0xdd,0xf2,0xe6,0x7c,0xeb};//sha3_256
	uint8 sig_r[0x20]={0x9d,0xdc,0xe0,0xf9,0x4e,0x14,0x0c,0x09,0xb3,0xe3,0x79,0xb0,0xb9,0xae,0x6b,0xb1,0x32,0xda,0x3e,0x1d,0x0b,0xcd,0xcc,0x0d,0x7b,0xc9,0xa5,0x79,0xac,0x35,0x19,0xf6};
	uint8 sig_s[0x20]={0xa3,0xf4,0x71,0x1f,0xc3,0x61,0x87,0xdd,0x10,0x7e,0x6a,0x8c,0xa5,0x35,0xa1,0xd7,0x45,0x91,0x50,0x0f,0xb7,0x11,0x8b,0x70,0x5a,0xf4,0xab,0x75,0x09,0x2e,0xcf,0xf2};
	//uint8 pub_x[0x20]={0xdf,0xf3,0x86,0xa0,0x27,0xb1,0x1f,0x4d,0xcc,0x5e,0xbd,0xec,0x15,0xc1,0x2b,0x79,0xa5,0x5c,0xbe,0xc6,0x79,0x4a,0x14,0xbb,0x6c,0xa6,0xe9,0x29,0x24,0x74,0xbd,0xd8};
	//uint8 pub_y[0x20]={0x79,0x1e,0xfe,0xe0,0x3a,0x81,0xe3,0x8f,0xd3,0xf7,0xde,0x11,0x87,0xcd,0x5b,0xfe,0x0f,0xc8,0x92,0x1c,0x07,0x52,0x67,0x3e,0x37,0x55,0xad,0xea,0x91,0xef,0xf5,0x2c};

	//1.apply server for ota's length & hash & sig
	//2.check length & hash & sig(ota and server)
	g_store.len_ota=length;
	memcpy(g_store.hash,hash,0x20);
	if (length!=*(uint32 *)(ADDR_STORE+0x0c))
		return OTA_UPDATE;
	if (memcmp(hash,(uint8 *)(ADDR_STORE+0x10),0x20))
		return OTA_UPDATE;
	memcpy(key_pub.x,(uint8 *)(ADDR_STORE+0x30),0x20);
	memcpy(key_pub.y,(uint8 *)(ADDR_STORE+0x50),0x20);
	//memcpy(key_pub.x,pub_x,0x20);
	//memcpy(key_pub.y,pub_y,0x20);
	if (!ecdsa_verify(&key_pub,hash,sig_r,sig_s))
        return OTA_UPDATE;

	return OTA_LATEST;
}

//download ota from server
//uint32:OTA_LATEST-already latest,OTA_UPDATE-need update
uint32 ota_download(void)
{
	uint32 i;
	uint64_t data[1]={0x0800000920010000};
	uint8 hash[0x20];
	FLASH_EraseInitTypeDef flash;
	uint32_t error;
	
	//1.apply server for ota's data
	//2.write to ota area
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);
    flash.TypeErase=FLASH_TYPEERASE_PAGES;
    flash.Page=PAGE_OTA;
    flash.NbPages=PAGE_OTA;
	HAL_FLASHEx_Erase(&flash,&error);
	for (i=0;i<g_store.len_ota/8;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_OTA+i*8,data[i]);
	HAL_FLASH_Lock();
	//3.check hash(ota and server)
	sha3_init(NUM_ECC_DIGITS*8);
	sha3_update((uint8 *)ADDR_OTA,g_store.len_ota*8);
	sha3_final(hash);
	if (memcmp(g_store.hash,hash,0x20))
		return OTA_UPDATE;
	//4.write to store area
	while(1)
	{
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);
		flash.TypeErase=FLASH_TYPEERASE_PAGES;
		flash.Page=PAGE_STORE;
		flash.NbPages=1;
		HAL_FLASHEx_Erase(&flash,&error);
		for (i=0;i<sizeof(store_t)/8;i++)
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_STORE+i*8,*(uint64_t *)((uint8 *)&g_store+i*8));
		HAL_FLASH_Lock();
		if (!memcmp((uint8 *)ADDR_STORE,&g_store,sizeof(store_t)))
			break;
	}
	
	return OTA_LATEST;
}

//move ota to normal
//uint32:OTA_LATEST-already latest,OTA_UPDATE-need update
uint32 ota_move(void)
{
	uint32 i;
	//uint8 hash[0x20];
	FLASH_EraseInitTypeDef flash;
	uint32_t error;
	
	//1.erase normal area, read from ota area & write to normal area
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);
	flash.TypeErase=FLASH_TYPEERASE_PAGES;
	flash.Page=PAGE_NORMAL;
	flash.NbPages=g_store.len_ota/2048+(g_store.len_ota%2048 ? 1 : 0);
	HAL_FLASHEx_Erase(&flash,&error);
	for (i=0;i<g_store.len_ota/8;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_NORMAL+i*8,*(uint64_t *)(ADDR_OTA+i*8));
	HAL_FLASH_Lock();
	//2.check hash(normal and ota)
	//sha3_init(NUM_ECC_DIGITS*8);
	//sha3_update((uint8 *)ADDR_NORMAL,g_store.len_ota*8);
	//sha3_final(hash);
	//if (memcmp(g_store.hash,hash,0x20))
	//	return OTA_UPDATE;
	//3.write to store area
	g_store.len_normal=g_store.len_ota;
	while(1)
	{
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);
		flash.TypeErase=FLASH_TYPEERASE_PAGES;
		flash.Page=PAGE_STORE;
		flash.NbPages=1;
		HAL_FLASHEx_Erase(&flash,&error);
		for (i=0;i<sizeof(store_t)/8;i++)
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_STORE+i*8,*(uint64_t *)((uint8 *)&g_store+i*8));
		HAL_FLASH_Lock();
		for (i=0;i<sizeof(store_t);i++)
			if (*(uint8 *)(ADDR_STORE+i)!=*((uint8 *)&g_store+i))
				break;
		if (i==sizeof(store_t))
			break;
		//if (!memcmp((uint8 *)ADDR_STORE,&g_store,sizeof(store_t)))
		//	break;
	}
	//4.reset chip
	HAL_NVIC_SystemReset();
	
	return OTA_LATEST;
}
