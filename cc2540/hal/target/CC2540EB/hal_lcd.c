#include "hal_types.h"
#include "hal_lcd.h"
#include "OSAL.h"
#include "OnBoard.h"
#include "stdio.h"

#ifndef HAL_LCD_REVERSE
#define HAL_LCD_REVERSE     0x80//reverse show
#endif

#if (HAL_LCD == TRUE)

#if (HAL_UART == TRUE)
#ifdef LCD_TO_UART              //show lcd use uart
  #include "npi.h"
#endif
#endif

//Oled0.96 driver based on ssd1306 128*64
/*
screen threads(from left to right):
1. GND  =  ground
2. VCC  =  2V~5V
3. D0   =  SPI_SCLK
4. D1   =  SPI_MOSI
5. RST  =  SPI_RESET
6. DC   =  SPI_DC
no cs, spi interface(no input only output), dc is not belonging to spi
dc=1, write the data; dc=0, write the command
*/

// oled's D0
#define LCD_SPI_SCLK_SBIT         P1_5       //SCLK D0£¨SCLK£©
#define LCD_SPI_SCLK_DDR          P1DIR      //port
#define LCD_SPI_SCLK_BV           BV(5)      //bit(0~7)

// oled's D1
#define LCD_SPI_MOSI_SBIT         P1_6       //SDA D1£¨MOSI£©
#define LCD_SPI_MOSI_DDR          P1DIR      //port
#define LCD_SPI_MOSI_BV           BV(6)      //bit(0~7)

// oled's RST
#define LCD_SPI_RESET_SBIT        P1_7       //_RES hardware reset
#define LCD_SPI_RESET_DDR         P1DIR      //port
#define LCD_SPI_RESET_BV          BV(7)      //bit(0~7)

// oled's DC
#define LCD_SPI_DC_SBIT           P1_2       //A0 H/L command/data select. H-data£¬L-command,sfr.bit
#define LCD_SPI_DC_DDR            P1DIR      //port,sfr.bit
#define LCD_SPI_DC_BV             BV(2)      //bit(0~7)

// initial 4 io for output
#define LCD_SPI_PORT_INIT()                   \
    do{                                         \
        LCD_SPI_SCLK_DDR |= LCD_SPI_SCLK_BV;    \
        LCD_SPI_MOSI_DDR |= LCD_SPI_MOSI_BV;    \
        LCD_SPI_RESET_DDR |= LCD_SPI_RESET_BV;  \
        LCD_SPI_DC_DDR |= LCD_SPI_DC_BV;        \
   }while(0)

// define LCD's width/height
#define X_WIDTH        128      //width
#define Y_WIDTH        64       //height

// define LCD's buffer length
#define LCD_MAX_BUF                     25

// 6*8 pixel matrix(ascii). tool generated
static const unsigned char F6x8[92][6] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// sp
    0x00, 0x00, 0x00, 0x2f, 0x00, 0x00,// !
    0x00, 0x00, 0x07, 0x00, 0x07, 0x00,// "
    0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14,// #
    0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12,// $
    0x00, 0x62, 0x64, 0x08, 0x13, 0x23,// %
    0x00, 0x36, 0x49, 0x55, 0x22, 0x50,// &
    0x00, 0x00, 0x05, 0x03, 0x00, 0x00,// '
    0x00, 0x00, 0x1c, 0x22, 0x41, 0x00,// (
    0x00, 0x00, 0x41, 0x22, 0x1c, 0x00,// )
    0x00, 0x14, 0x08, 0x3E, 0x08, 0x14,// *
    0x00, 0x08, 0x08, 0x3E, 0x08, 0x08,// +
    0x00, 0x00, 0x00, 0xA0, 0x60, 0x00,// ,
    0x00, 0x08, 0x08, 0x08, 0x08, 0x08,// -
    0x00, 0x00, 0x60, 0x60, 0x00, 0x00,// .
    0x00, 0x20, 0x10, 0x08, 0x04, 0x02,// /
    0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E,// 0
    0x00, 0x00, 0x42, 0x7F, 0x40, 0x00,// 1
    0x00, 0x42, 0x61, 0x51, 0x49, 0x46,// 2
    0x00, 0x21, 0x41, 0x45, 0x4B, 0x31,// 3
    0x00, 0x18, 0x14, 0x12, 0x7F, 0x10,// 4
    0x00, 0x27, 0x45, 0x45, 0x45, 0x39,// 5
    0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30,// 6
    0x00, 0x01, 0x71, 0x09, 0x05, 0x03,// 7
    0x00, 0x36, 0x49, 0x49, 0x49, 0x36,// 8
    0x00, 0x06, 0x49, 0x49, 0x29, 0x1E,// 9
    0x00, 0x00, 0x36, 0x36, 0x00, 0x00,// :
    0x00, 0x00, 0x56, 0x36, 0x00, 0x00,// ;
    0x00, 0x08, 0x14, 0x22, 0x41, 0x00,// <
    0x00, 0x14, 0x14, 0x14, 0x14, 0x14,// =
    0x00, 0x00, 0x41, 0x22, 0x14, 0x08,// >
    0x00, 0x02, 0x01, 0x51, 0x09, 0x06,// ?
    0x00, 0x32, 0x49, 0x59, 0x51, 0x3E,// @
    0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C,// A
    0x00, 0x7F, 0x49, 0x49, 0x49, 0x36,// B
    0x00, 0x3E, 0x41, 0x41, 0x41, 0x22,// C
    0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C,// D
    0x00, 0x7F, 0x49, 0x49, 0x49, 0x41,// E
    0x00, 0x7F, 0x09, 0x09, 0x09, 0x01,// F
    0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A,// G
    0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F,// H
    0x00, 0x00, 0x41, 0x7F, 0x41, 0x00,// I
    0x00, 0x20, 0x40, 0x41, 0x3F, 0x01,// J
    0x00, 0x7F, 0x08, 0x14, 0x22, 0x41,// K
    0x00, 0x7F, 0x40, 0x40, 0x40, 0x40,// L
    0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F,// M
    0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F,// N
    0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E,// O
    0x00, 0x7F, 0x09, 0x09, 0x09, 0x06,// P
    0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E,// Q
    0x00, 0x7F, 0x09, 0x19, 0x29, 0x46,// R
    0x00, 0x46, 0x49, 0x49, 0x49, 0x31,// S
    0x00, 0x01, 0x01, 0x7F, 0x01, 0x01,// T
    0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F,// U
    0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F,// V
    0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F,// W
    0x00, 0x63, 0x14, 0x08, 0x14, 0x63,// X
    0x00, 0x07, 0x08, 0x70, 0x08, 0x07,// Y
    0x00, 0x61, 0x51, 0x49, 0x45, 0x43,// Z
    0x00, 0x00, 0x7F, 0x41, 0x41, 0x00,// [
    0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55,// 55
    0x00, 0x00, 0x41, 0x41, 0x7F, 0x00,// ]
    0x00, 0x04, 0x02, 0x01, 0x02, 0x04,// ^
    0x00, 0x40, 0x40, 0x40, 0x40, 0x40,// _
    0x00, 0x00, 0x01, 0x02, 0x04, 0x00,// '
    0x00, 0x20, 0x54, 0x54, 0x54, 0x78,// a
    0x00, 0x7F, 0x48, 0x44, 0x44, 0x38,// b
    0x00, 0x38, 0x44, 0x44, 0x44, 0x20,// c
    0x00, 0x38, 0x44, 0x44, 0x48, 0x7F,// d
    0x00, 0x38, 0x54, 0x54, 0x54, 0x18,// e
    0x00, 0x08, 0x7E, 0x09, 0x01, 0x02,// f
    0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C,// g
    0x00, 0x7F, 0x08, 0x04, 0x04, 0x78,// h
    0x00, 0x00, 0x44, 0x7D, 0x40, 0x00,// i
    0x00, 0x40, 0x80, 0x84, 0x7D, 0x00,// j
    0x00, 0x7F, 0x10, 0x28, 0x44, 0x00,// k
    0x00, 0x00, 0x41, 0x7F, 0x40, 0x00,// l
    0x00, 0x7C, 0x04, 0x18, 0x04, 0x78,// m
    0x00, 0x7C, 0x08, 0x04, 0x04, 0x78,// n
    0x00, 0x38, 0x44, 0x44, 0x44, 0x38,// o
    0x00, 0xFC, 0x24, 0x24, 0x24, 0x18,// p
    0x00, 0x18, 0x24, 0x24, 0x18, 0xFC,// q
    0x00, 0x7C, 0x08, 0x04, 0x04, 0x08,// r
    0x00, 0x48, 0x54, 0x54, 0x54, 0x20,// s
    0x00, 0x04, 0x3F, 0x44, 0x40, 0x20,// t
    0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C,// u
    0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C,// v
    0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C,// w
    0x00, 0x44, 0x28, 0x10, 0x28, 0x44,// x
    0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C,// y
    0x00, 0x44, 0x64, 0x54, 0x4C, 0x44,// z
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14,// horiz lines
};

uint8 static g_reverseflag = 0;

// delay ms
void LCD_DLY_ms(unsigned int ms)
{                         
    unsigned int a;
    while(ms)
    {
        a=1800;
        while(a--);
        ms--;
    }
}

// LCD write a byte data
void LCD_WrDat(unsigned char dat)     
{
    unsigned char i=8, temp=0;
    LCD_SPI_DC_SBIT = 1;  
    for(i=0;i<8;i++) //8bit data
    {
        LCD_SPI_SCLK_SBIT = 0;  
        
        temp = dat&0x80;
        if (temp == 0)
        {
            LCD_SPI_MOSI_SBIT = 0;
        }
        else
        {
            LCD_SPI_MOSI_SBIT = 1;
        }
        LCD_SPI_SCLK_SBIT = 1;             
        dat<<=1;    
    }
}

// LCD write a byte command
void LCD_WrCmd(unsigned char cmd)
{
    unsigned char i=8, temp=0;
    LCD_SPI_DC_SBIT=0;
    for(i=0;i<8;i++) //8bit data
    { 
        LCD_SPI_SCLK_SBIT=0; 
       
        temp = cmd&0x80;
        if (temp == 0)
        {
            LCD_SPI_MOSI_SBIT = 0;
        }
        else
        {
            LCD_SPI_MOSI_SBIT = 1;
        }
        LCD_SPI_SCLK_SBIT=1;
        cmd<<=1;
    }     
}

// LCD set coordinate
void LCD_Set_Pos(unsigned char x, unsigned char y) 
{ 
    LCD_WrCmd(0xb0+y);
    LCD_WrCmd(((x&0xf0)>>4)|0x10);
    LCD_WrCmd((x&0x0f)|0x01); 
} 

// LCD fulfill same color
void LCD_Fill(unsigned char bmp_dat) 
{
    unsigned char y,x;
    for(y=0;y<8;y++)
    {
        LCD_WrCmd(0xb0+y);
        LCD_WrCmd(0x01);
        LCD_WrCmd(0x10);
        for(x=0;x<X_WIDTH;x++)
            LCD_WrDat(bmp_dat);
    }
}

// LCD fill a line(0¡«7)
void LCD_FillLine(unsigned char y,unsigned char ch)
{
    unsigned char x;    
    LCD_WrCmd(0xb0+y);
    LCD_WrCmd(0x01);
    LCD_WrCmd(0x10); 
    for(x=0;x<X_WIDTH;x++)
        LCD_WrDat(ch);
}

// LCD initial. write data into oled(SSD1306) using spi
#if 1
void HalLcdInit(void)
{
    LCD_SPI_PORT_INIT();  //4 io set output

    LCD_SPI_SCLK_SBIT=0;
    LCD_SPI_MOSI_SBIT=0;
    LCD_SPI_DC_SBIT=0;
    LCD_SPI_RESET_SBIT=0;    
    LCD_DLY_ms(10);
    LCD_SPI_RESET_SBIT=1;//enough time should be reserved for RC reset(before command write)
    LCD_DLY_ms(10);
    LCD_WrCmd(0xae);//turn off oled panel
    LCD_WrCmd(0x00);//set low column address
    LCD_WrCmd(0x10);//set high column address
    LCD_WrCmd(0x40);//set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    LCD_WrCmd(0x81);//set contrast control register
    LCD_WrCmd(0xcf);//Set SEG Output Current Brightness
    LCD_WrCmd(0xa1);//Set SEG/Column Mapping. 0xa0-left/right change,0xa1-normal
    LCD_WrCmd(0xc8);//Set COM/Row Scan Direction. 0xc0-up/down change,0xc8-normal
    LCD_WrCmd(0xa6);//set normal display
    LCD_WrCmd(0xa8);//set multiplex ratio(1 to 64)
    LCD_WrCmd(0x3f);//1/64 duty
    LCD_WrCmd(0xd3);//set display offset    Shift Mapping RAM Counter (0x00~0x3F)
    LCD_WrCmd(0x00);//not offset
    LCD_WrCmd(0xd5);//set display clock divide ratio/oscillator frequency
    LCD_WrCmd(0x80);//set divide ratio, Set Clock as 100 Frames/Sec
    LCD_WrCmd(0xd9);//set pre-charge period
    LCD_WrCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    LCD_WrCmd(0xda);//set com pins hardware configuration
    LCD_WrCmd(0x12);
    LCD_WrCmd(0xdb);//set vcomh
    LCD_WrCmd(0x40);//Set VCOM Deselect Level
    LCD_WrCmd(0x20);//Set Page Addressing Mode (0x00/0x01/0x02)
    LCD_WrCmd(0x02);//
    LCD_WrCmd(0x8d);//set Charge Pump enable/disable
    LCD_WrCmd(0x14);//set(0x10) disable
    LCD_WrCmd(0xa4);//Disable Entire Display On (0xa4/0xa5)
    LCD_WrCmd(0xa6);//Disable Inverse Display On (0xa6/a7) 
    LCD_WrCmd(0xaf);//turn on oled panel
    LCD_Fill(0x00); //initial screen black,0x00
    LCD_Set_Pos(0,0); 
} 
#else
void HalLcdInit(void)
{
    LCD_SPI_PORT_INIT();  //4 io set output

    LCD_SPI_SCLK_SBIT=0;
    LCD_SPI_MOSI_SBIT=0;
    LCD_SPI_DC_SBIT=0;
    LCD_SPI_RESET_SBIT=0;    
    LCD_DLY_ms(10);
    LCD_SPI_RESET_SBIT=1;//enough time should be reserved for RC reset(before command write)
    LCD_DLY_ms(10);
    
    //LCD_WrCmd(0xae);//turn off oled panel
    //
    LCD_WrCmd(0x8d);//set Charge Pump enable/disable
    LCD_WrCmd(0x14);//set(0x10) disable
    //
    LCD_WrCmd(0xaf);//turn on oled panel
    //LCD_Fill(0xff); //initial screen black,0x00
    //LCD_Set_Pos(0,0); 
    //LCD_FillLine(0, 0xFF);//ding
} 
#endif

// LCD 6*8 ASCII string, coordinate£¨x,y£©
void LCD_P6x8Str(unsigned char x, unsigned char y,unsigned char ch[])
{
    unsigned char c=0,i=0,j=0;      
    while (ch[j]!='\0')
    {    
        c =ch[j]-32;
        if(x>126){x=0;y++;}
        LCD_Set_Pos(x,y);  
        if(c > 0 && c < 92)
        {
            if(g_reverseflag)
            {
                for(i=0;i<6;i++)     
                    LCD_WrDat(~F6x8[c][i]);  
            }
            else
            {
                for(i=0;i<6;i++)     
                    LCD_WrDat(F6x8[c][i]);  
            }
        }
        x+=6;
        j++;
    }
}

// write line ascii(none for black),line(1,8)
void HalLcd_HW_WriteLine(unsigned char line, const char *pText)
{
    if(g_reverseflag)
    {
        LCD_FillLine(line-1, 0xFF);          //light
    }
    else
    {
        LCD_FillLine(line-1, 0);             //black
    }
    LCD_P6x8Str(0, line-1, (unsigned char *)pText);
}

// write string, option(1,8)
void HalLcdWriteString ( char *str, uint8 option)
{
  uint8 strLen = 0;
  uint8 totalLen = 0;
  uint8 *buf;
  uint8 tmpLen;
  static uint8 *Lcd_Line1 = NULL;

  g_reverseflag = (option & HAL_LCD_REVERSE);
  option &= (~HAL_LCD_REVERSE);  //no highest bit
  
#if (HAL_UART == TRUE)
#ifdef LCD_TO_UART
  NPI_WriteTransport ( (uint8*)str,osal_strlen(str)); 
  NPI_WriteTransport ("\r\n",2);
#endif
#endif

  if ( Lcd_Line1 == NULL )
  {
    Lcd_Line1 = osal_mem_alloc( HAL_LCD_MAX_CHARS+1 );
    HalLcdWriteString( "TexasInstruments", 1 );
  }

  strLen = (uint8)osal_strlen( (char*)str );

  /* Check boundries */
  if ( strLen > HAL_LCD_MAX_CHARS )
    strLen = HAL_LCD_MAX_CHARS;

  if ( option == HAL_LCD_LINE_1 )
  {
    /* Line 1 gets saved for later */
    osal_memcpy( Lcd_Line1, str, strLen );
    Lcd_Line1[strLen] = '\0';
  }
  else
  {
    /* Line 2 triggers action */
    tmpLen = (uint8)osal_strlen( (char*)Lcd_Line1 );
    totalLen =  tmpLen + 1 + strLen + 1;
    buf = osal_mem_alloc( totalLen );
    if ( buf != NULL )
    {
      /* Concatenate strings */
      osal_memcpy( buf, Lcd_Line1, tmpLen );
      buf[tmpLen++] = ' ';
      osal_memcpy( &buf[tmpLen], str, strLen );
      buf[tmpLen+strLen] = '\0';
      
      /* Free mem */
      osal_mem_free( buf );
    }
  }

  /* Display the string */
  HalLcd_HW_WriteLine (option, str);

  g_reverseflag &= (~HAL_LCD_REVERSE);
}

//write data,radix for 10/16,option for 1~8
void HalLcdWriteValue ( uint32 value, const uint8 radix, uint8 option)
{
  uint8 buf[LCD_MAX_BUF];

  _ltoa( value, &buf[0], radix );
  HalLcdWriteString( (char*)buf, option );
}

//Write a value to the LCD
void HalLcdWriteScreen( char *line1, char *line2 )
{
  HalLcdWriteString( line1, 1 );
  HalLcdWriteString( line2, 2 );
}

//write string & data,radix for 10/16,option for 1~8
void HalLcdWriteStringValue( char *title, uint16 value, uint8 format, uint8 line )
{
  uint8 tmpLen;
  uint8 buf[LCD_MAX_BUF];
  uint32 err;

  tmpLen = (uint8)osal_strlen( (char*)title );
  osal_memcpy( buf, title, tmpLen );
  buf[tmpLen] = ' ';
  err = (uint32)(value);
  _ltoa( err, &buf[tmpLen+1], format );
  HalLcdWriteString( (char*)buf, line );		
}

//write string & 2data,radix for 10/16,option for 1~8
void HalLcdWriteStringValueValue( char *title, uint16 value1, uint8 format1,
                                  uint16 value2, uint8 format2, uint8 line )
{
  uint8 tmpLen;
  uint8 buf[LCD_MAX_BUF];
  uint32 err;

  tmpLen = (uint8)osal_strlen( (char*)title );
  if ( tmpLen )
  {
    osal_memcpy( buf, title, tmpLen );
    buf[tmpLen++] = ' ';
  }

  err = (uint32)(value1);
  _ltoa( err, &buf[tmpLen], format1 );
  tmpLen = (uint8)osal_strlen( (char*)buf );

  buf[tmpLen++] = ',';
  buf[tmpLen++] = ' ';
  err = (uint32)(value2);
  _ltoa( err, &buf[tmpLen], format2 );

  HalLcdWriteString( (char *)buf, line );	
}

//write string & percent data
void HalLcdDisplayPercentBar( char *title, uint8 value )
{
  uint8 percent;
  uint8 leftOver;
  uint8 buf[17];
  uint32 err;
  uint8 x;

  /* Write the title: */
  HalLcdWriteString( title, HAL_LCD_LINE_1 );

  if ( value > 100 )
    value = 100;

  /* convert to blocks */
  percent = (uint8)(value / 10);
  leftOver = (uint8)(value % 10);

  /* Make window */
  osal_memcpy( buf, "[          ]  ", 15 );

  for ( x = 0; x < percent; x ++ )
  {
    buf[1+x] = '>';
  }

  if ( leftOver >= 5 )
    buf[1+x] = '+';

  err = (uint32)value;
  _ltoa( err, (uint8*)&buf[13], 10 );

  HalLcdWriteString( (char*)buf, HAL_LCD_LINE_2 );
}
#else
/*
 * Initialize LCD Service
 */
void HalLcdInit(void){}
/*
 * Write a string to the LCD
 */
extern void HalLcdWriteString ( char *str, uint8 option){}

/*
 * Write a value to the LCD
 */
extern void HalLcdWriteValue ( uint32 value, const uint8 radix, uint8 option){}

/*
 * Write a value to the LCD
 */
extern void HalLcdWriteScreen( char *line1, char *line2 ){}

/*
 * Write a string followed by a value to the LCD
 */
extern void HalLcdWriteStringValue( char *title, uint16 value, uint8 format, uint8 line ){}

/*
 * Write a string followed by 2 values to the LCD
 */
extern void HalLcdWriteStringValueValue( char *title, uint16 value1, uint8 format1, uint16 value2, uint8 format2, uint8 line ){}

/*
 * Write a percentage bar to the LCD
 */
extern void HalLcdDisplayPercentBar( char *title, uint8 value ){}
#endif

