#include "gps.h"
#include "usart.h"

//parse buf, get the position of cx's ','
//uint8:0~0xFE-offset of the ',' in buf,0xFF-not exist
uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{
	uint8_t *p = buf;
	while(cx)
	{
		if(*buf=='*'||*buf<' '||*buf>'z')return 0xFF;
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;
}

//return pow(m,n)
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
	uint32_t result = 1;
	while(n--)result *= m;
	return result;
}

//convert str into number, use ',' or '*' as terminator
//buf:str buffer
//dx:number of '.' offset
int NMEA_Str2num(uint8_t *buf,uint8_t *dx)
{
	uint8_t *p = buf;
	uint32_t ires = 0,fres = 0;
	uint8_t ilen = 0,flen = 0,i;
	uint8_t mask = 0;
	int res;
	
	while(1)
	{
		if(*p=='-'){mask |= 0x02;p++;}//negative num exist
		if(*p==','||*p=='*')
			break;//meet the end mark
		if(*p=='.'){mask |= 0x01;p++;}//meet the '.'
		else if(*p>'9'||(*p<'0'))//illegal char exist
		{
			ilen = 0;
			flen = 0;
			break;
		}
		if(mask&0x01)flen++;//'.' offset
		else ilen++;//str++
		p++;//next char
	}
	if(mask&0x02)buf++;//trim the '-'
	for(i=0;i<ilen;i++)//get the integer part
		ires += NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	if(flen>5)flen=5;//5 decimal max
	*dx = flen;
	for(i=0;i<flen;i++)//get the decimal part
		fres +=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	res = ires*NMEA_Pow(10,flen)+fres;
	if(mask&0x02)res = -res;
	
	return res;
}

//init gps
void gps_init(void)
{
	HAL_UART_Transmit(&huart3, "$CCMSG,GGA,1,0,*19\r\n", 20, 200);
	HAL_UART_Transmit(&huart3, "$CCMSG,GSA,1,0,*0D\r\n", 20, 200);
	HAL_UART_Transmit(&huart3, "$CCMSG,GSV,1,0,*1A\r\n", 20, 200);
}

//parse GPRMC(NMEA) info struct
void gps_analysis(gps_t *gps,uint8_t *buf)
{
	uint8_t *p4,dx;			 
	uint8_t posx;     
	uint32_t temp;	   
	float rs;
	
	//get latitude
	p4=(uint8_t*)strstr((const char *)buf,"$GPRMC");//"$GPRMC", '&' is not mandortary stay with "GPRMC"
	posx=NMEA_Comma_Pos(p4,3);
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p4+posx,&dx);
		gps->latitude_bd=temp/NMEA_Pow(10,dx+2);//get бу
		rs=temp%NMEA_Pow(10,dx+2);//get '
		gps->latitude_bd=gps->latitude_bd*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//convert to бу
	}
	posx=NMEA_Comma_Pos(p4,4);//judge south/north latitude
	if(posx!=0XFF)gps->nshemi_bd=*(p4+posx);
	//get longitude
 	posx=NMEA_Comma_Pos(p4,5);
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p4+posx,&dx);		 	 
		gps->longitude_bd=temp/NMEA_Pow(10,dx+2);//get бу
		rs=temp%NMEA_Pow(10,dx+2);//get '
		gps->longitude_bd=gps->longitude_bd*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//convert to бу
	}
	posx=NMEA_Comma_Pos(p4,6);//judge east/west longitude
	if(posx!=0XFF)gps->ewhemi_bd=*(p4+posx);
}
