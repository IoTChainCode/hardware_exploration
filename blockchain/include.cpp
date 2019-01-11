#include "include.h"

//uint8 *result:data point
//uint32 len:data byte
void mem_swap8(uint8 *result,uint32 len)
{
	//byte endian
	uint32 i;
	uint8 swap;

	for (i=0;i<len>>1;i++)
	{
		swap=result[i];
		result[i]=result[len-1-i];
		result[len-1-i]=swap;
	}
}

//word -> byte
void mem_word2byte(uint8 *a,uint32 b)
{
	//endian in word
	a[0]=(uint8)(b & 0xff);
	a[1]=(uint8)((b>>8) & 0xff);
	a[2]=(uint8)((b>>16) & 0xff);
	a[3]=(uint8)(b>>24);
}

//byte -> word
uint32 mem_byte2word(uint8 *a)
{
	uint8 i;
	uint32 result;

	result=0;
	for (i=0;i<4;i++)
		result |= (uint32)a[i]<<(i<<3);

	return result;
}

//java hash()
int32 math_hashcode(int8 *a,uint32 length)
{
	uint32 i;
	int32 result;

	if (!a)
		return 0;
	result=1;
	for (i=0;i<length;i++)
		result=result*31+a[i];

	return result;
}

//distance between 2 point
double math_distance(uint32 x1,uint32 y1,uint32 x2,uint32 y2)
{
	return sqrt(pow((double)x2-(double)x1,2.0)+pow((double)y2-(double)y1,2.0));
}
