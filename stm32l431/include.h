#include "string.h"
#include "ecc.h"

//define
#define MATH_ROTATE(a,n) (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))

#define ADDR_NORMAL 0x08000000 //normal area
#define ADDR_OTA 0x08018000 //ota area
#define ADDR_STORE 0x0803f800 //store area
#define PAGE_NORMAL 0 //normal page index
#define PAGE_OTA 48 //ota page index
#define PAGE_STORE (128-1) //store page index
//typedef
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
//function
void word2byte(uint8 *a,uint32 b);
uint32 byte2word(uint8 *a);
void mem_swap8(uint8 *result,uint32 len);
int string2ascii(const unsigned char *bufin, int len, char *bufout);
int ascii2string(const char *bufin, int len, char *bufout);
