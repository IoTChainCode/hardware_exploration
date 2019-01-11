#include "include.h"

void word2byte(uint8 *a,uint32 b)
{
	a[0]=b & 0xff;
	a[1]=(b>>8) & 0xff;
	a[2]=(b>>16) & 0xff;
	a[3]=b>>24;
}

uint32 byte2word(uint8 *a)
{
	uint8 i;
	uint32 result;

	result=0;
	for (i=0;i<4;i++)
		result |= (uint32)a[i]<<(i<<3);

	return result;
}

//uint8 *result:output
//uint32 len:byte length
void mem_swap8(uint8 *result,uint32 len)
{
	uint32 i;
	uint8 swap;

	for (i=0;i<len>>1;i++)
	{
		swap=result[i];
		result[i]=result[len-1-i];
		result[len-1-i]=swap;
	}
}

//string to ascii
//bufin:input para,string point
//len:input byte length
//bufout:output para,ascii point
//0-ok,!0-err
int string2ascii(const unsigned char *bufin, int len, char *bufout)
{
    int i;
	int tmp;
	
    if (NULL == bufin || len <= 0 || NULL == bufout)
        return -1;
	for (i=0;i<len;i++)
	{
		tmp=bufin[i]>>4;
		bufout[i*2]=tmp>0x09 ? tmp+0x37 : tmp+0x30;
		tmp=bufin[i]&0x0f;
		bufout[i*2+1]=tmp>0x09 ? tmp+0x37 : tmp+0x30;
	}
	/*
    for (i=0;i<len;i++)
        sprintf(bufout+i*2,"%02X",bufin[i]);
	*/
    return 0; 
}

//ascii to string
//bufin:input para,string point
//len:input byte length
//bufout:output para,ascii point
//0-ok,!0-err
int ascii2string(const char *bufin, int len, char *bufout)
{
    int i;
    unsigned char tmp_c;
    unsigned int tmp_i;
	
    if (NULL == bufin || len <= 0 || NULL == bufout)
        return -1;
    for (i=0;i<len;i+=2)
    {
        tmp_c=bufin[i];
        tmp_c=tmp_c<='9' ? tmp_c-0x30 : tmp_c-0x37;
        tmp_i=bufin[i+1];
        tmp_i=tmp_i<='9' ? tmp_i-0x30 : tmp_i-0x37;
        bufout[i/2]=(tmp_c<<4) | (tmp_i&0x0F);
    }

    return 0; 
}
