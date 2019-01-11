#ifndef INCLUDE_H
#define INCLUDE_H

//include
#include <ioCC2540.h> //in iar/embed workbench/8051/inc
#include "hal_lcd.h"
#include "hal_key.h"
#include "hal_drivers.h"
#include "npi.h"
#include "OnBoard.h"
#include "ll.h"
#include "hci_tl.h"
#include "l2cap.h"
#include "att.h"
#include "gap.h"
#include "gatt.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "gapgattserver.h"
#include "OSAL_Tasks.h"
#if defined (OSAL_CBTIMER_NUM_TASKS)
#include "osal_cbTimer.h"
#endif
#include "osal_snv.h"
#include "simpleGATTprofile.h"
#include "devinfoservice.h"
#include "combine.h"

#include "hal_led.h"
//macro
#define DEBUG_CP 0x00 //central-peripheral-central-peripheral-...
#define DEBUG_PC 0x01 //peripheral-central-peripheral-central-...
#define DEBUG_C 0x02 //central always
#define DEBUG_P 0x03 //peripheral always
#define DEBUG_BOTH 0x04 //both central/peripheral(not available <bluetooth v4.1 including cc2540, will available >=bluetooth v4.1 including cc2650)
#define DEBUG DEBUG_P

#define COMPILE_ITC 0x00
#define COMPILE_REFRIGERATOR 0x01
#define COMPILE_GATELOCK 0x02
#define COMPILE_CAMERA 0x03
#define COMPILE_AIRCONDITION 0x04
#define COMPILE_GARAGE 0x05
#define COMPILE_CARLOCK 0x06
#define COMPILE COMPILE_AIRCONDITION
//define
#define NPI_NONE 0x00
#define NPI_AT 0x01

#define APP_EVENT_SWITCH 0x0001
#define APP_EVENT_INITIAL 0x0002
#define APP_EVENT_SCAN 0x0004
#define APP_EVENT_CONNECT 0x0008
#define APP_EVENT_TRANSFER 0x0010

#define SCAN_MAXNUM 3 //maximum devices connect with single device

#define NUM_CENTRAL 3 //number of central connection
#define NUM_PERIPHERAL 1 //number of peripheral connection

#define ROLE_PERIPHERAL 0x00
#define ROLE_CENTRAL 0x01

#define DELAY_GAP 500
#define DELAY_WAIT 10000

#define SWITCH_ON TRUE
#define SWITCH_OFF FALSE

#define CMD_DATA 0x31
/*
#define CMD_QUERY 0x00
#define CMD_SWITCH 0x01
#define CMD_M2M 0x02
#define CMD_CLOCK 0x03
*/
#define RET_DATA 0x30
/*
#define RET_M2M 0x01
#define RET_FIRE 0x02
#define RET_CLOCK 0x03
#define RET_EMERGENCY 0x04
*/
#define DEVICE_REFRIGERATOR 0x00
#define DEVICE_GATELOCK 0x01
#define DEVICE_CAMERA 0x02
#define DEVICE_AIRCONDITIONER 0x03
#define DEVICE_GARAGE 0x04
#define DEVICE_CARLOCK 0x05

#define DEMO_DEVICE 6
#define DEMO_PHONE 1

#define BLUETOOTH_V10 0x01
#define BLUETOOTH_V11 0x02
#define BLUETOOTH_V12 0x03
#define BLUETOOTH_V20 0x04
#define BLUETOOTH_V21 0x05
#define BLUETOOTH_V30 0x06
#define BLUETOOTH_V40 0x07
#define BLUETOOTH_V41 0x08
#define BLUETOOTH_V42 0x09
#define BLUETOOTH_V50 0x0a

#define ATTACH_NONE 0x01
#define ATTACH_NET 0x02

#define ID_NONE 0x1234 //such as chip's manufacture id
#define ID_NET 0x5678 //such as phone's manufacture id

#define MANUFACTURE_LEN 0x02 //phone's byte length(0-bluetooth version,1-attach something,2/3-rfu)
//enum
enum
{
    STATE_IDLE,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_DISCONNECTING,
    STATE_SERVICES,
    STATE_CHARACTERISTIC
};
//struct
typedef struct
{
    uint16 handle_conn;//connection handle(central/peripheral)
    uint16 handle_char;//characteristic handle(central)
    uint16 rssi;//Received Signal Strength Indicator between master and slave(central/peripheral),config by set_para()
    uint16 latency;//connect slave latency(central)
    uint16 timeout;//connect timeout(central)
    uint16 interval;//connect interval(central)
    //
    uint8 status;//role status(central/peripheral)
    bool condition;//remote device condition(central)
    uint8 addr[B_ADDR_LEN];//remote address(central/peripheral)
}multi_t;//multi central/peripheral connection info
//function
uint8 str_cmp(uint8 *p1,uint8 *p2,uint8 len);
char *address2string(uint8 *pAddr);
bool find_manufacture(uint16 id,uint8 *manufacture,uint8 *content,uint8 length);
bool find_uuid(uint16 uuid,uint8 *content,uint8 length);
void send_data(uint8 *buffer,uint8 number,multi_t *multi,uint8 task_id);
void npi_parse(uint8 *buffer,uint8 number,uint8 task_id);
bool parse_manufacture(uint8 *bluetooth,uint8 *attach,uint8 *manufacture,uint8 length,uint16 id);
#if VERSION==VERSION_MODULE || VERSION==VERSION_DEMO
void cmd_parse(uint8 *value,uint8 length,uint8 task_id);
void init_temperature(void);
float get_temperature(void);
float math_mean(void);
#endif
uint16 math_min(uint16 a,uint16 b);
uint16 math_max(uint16 a,uint16 b);
uint8 multi_num(uint8 role);
bool multi_connect(uint8 role,uint16 handle);//central/peripheral
multi_t *multi_point(uint16 handle);//central
uint8 multi_find(uint8 *addr);//central
uint8 multi_del(void);
bool multi_check(uint8 *addr);
//cc2540
//device
#define LED1 (1<<0) //P1_0
#define LED2 (1<<1) //P1_1
#define LED3 (1<<4) //P1_4
#define KEY1 (1<<1) //P0_1
#define UART0 (1<<2) //P0_2
#define UART1 (1<<3) //P0_3
//cpu
#define MEMCTR_xbank 0x07 //xdata bank select(0-7)
#define MEMCTR_xmap 0x08 //xdata map to code.0-disable,1-enable
#define FMAP_map 0x07 //flash bank map(0-7)
#define DPS_dps 0x01 //data pointer select.0-dptr0,1-dptr1
#define PSW_p 0x01 //parity flag
#define PSW_overflow 0x04 //overflow flag
#define PSW_rs 0x18 //register bank select.00-bank0,01-bank1,10-bank2,11-bank3
#define PSW_ac 0x40 //auxiliary carry flag
#define PSW_cy 0x80 //carry flag
#define IEN0_rferrie 0x01 //rf code err interrupt enable
#define IEN0_adcie 0x02 //adc interrupt enable
#define IEN0_urx0ie 0x04 //usart0 rx interrupt enable
#define IEN0_urx1ie 0x08 //usart1 rx interrupt enable
#define IEN0_encie 0x10 //aes enc/dec interrupt enable
#define IEN0_stie 0x20 //sleep timer interrupt enable
#define IEN0_ea 0x80 //disable all interrupt
#define IEN1_dmaie 0x01 //dma transfer interrupt enable
#define IEN1_t1ie 0x02 //timer1 interrupt enable
#define IEN1_t2ie 0x04 //timer2 interrupt enable
#define IEN1_t3ie 0x08 //timer3 interrupt enable
#define IEN1_t4ie 0x10 //timer4 interrupt enable
#define IEN1_p0ie 0x20 //port0 interrupt enable
#define IEN2_rfie 0x01 //rf general interrupt enable
#define IEN2_p2ie 0x02 //port2/usb interrupt enable
#define IEN2_utx0ie 0x04 //usart0 tx interrupt enable
#define IEN2_utx1ie 0x08 //usart1 tx interrupt enable
#define IEN2_p1ie 0x10 //port1 interrupt enable
#define IEN2_wdtie 0x20 //wdg interrupt enable
#define TCON_it0 0x01 //
#define TCON_rferrif 0x02 //rf core error interrupt flag
#define TCON_it1 0x04 //
#define TCON_urx0if 0x08 //usart0 rx interrupt flag
#define TCON_adcif 0x20 //adc interrupt flag
#define TCON_urx1if 0x80 //usart1 rx interrupt flag
#define S0CON_encif_01 0x03 //aes interrupt
#define S1CON_rfif_01 0x03 //rf general interrupt
#define IRCON_dmaif 0x01 //dma complete interrupt flag
#define IRCON_t1if 0x02 //timer1 interrupt flag
#define IRCON_t2if 0x04 //timer2 interrupt flag
#define IRCON_t3if 0x08 //timer3 interrupt flag
#define IRCON_t4if 0x10 //timer4 interrupt flag
#define IRCON_p0if 0x20 //port0 interrupt flag
#define IRCON_stif 0x80 //sleep timer interrupt flag
#define IRCON2_p2if 0x01 //port2 interrupt flag
#define IRCON2_utx0if 0x02 //usart0 tx interrupt flag
#define IRCON2_utx1if 0x04 //usart1 tx interrupt flag
#define IRCON2_p1if 0x08 //port1 interrupt flag
#define IRCON2_wdtif 0x10 //wdg interrupt flag
#define IP0_ipg0 0x01 //interrupt priority 0 group 0
#define IP0_ipg1 0x02 //interrupt priority 0 group 1
#define IP0_ipg2 0x04 //interrupt priority 0 group 2
#define IP0_ipg3 0x08 //interrupt priority 0 group 3
#define IP0_ipg4 0x10 //interrupt priority 0 group 4
#define IP0_ipg5 0x20 //interrupt priority 0 group 5
#define IP1_ipg0 0x01 //interrupt priority 1 group 0
#define IP1_ipg1 0x02 //interrupt priority 1 group 1
#define IP1_ipg2 0x04 //interrupt priority 1 group 2
#define IP1_ipg3 0x08 //interrupt priority 1 group 3
#define IP1_ipg4 0x10 //interrupt priority 1 group 4
#define IP1_ipg5 0x20 //interrupt priority 1 group 5
//power and clock
#define SRCRC_crc_reset_en 0x01 //0-disable reset due to CRC,1-enable
#define SRCRC_crc_result 0x0c //00-crc pass,01/10/11-crc fail
#define SRCRC_force_reset 0x20 //0-no action,1-force wdg reset
#define SRCRC_xosc_amp_det_en 0x80 //0-disable amplitude detector for 32mhz xosc,1-enable
#define PCON_idle 0x01 //power mode control
#define SLEEPCMD_mode 0x03 //power mode setting.00-idle mode,01-mode 1,10-mode 2,11-mode 3
#define SLEEPCMD_osc32k_caldis 0x80 //disable 32khz osc
#define SLEEPSTA_clk32k 0x01 //32khz clk signal
#define SLEEPSTA_rst 0x18 //indicating the cause of last reset.00-power on reset,01-external reset,10-wdg reset,11-clk loss reset
#define SLEEPSTA_osc32k_caldis 0x80 //32khz osc calibration
#define CLKCONCMD_clkspd 0x07 //clock speed.000-32mhz,001-16mhz,010-8mhz,011-4mhz,100-2mhz,101-1mhz,110-500khz,111-250khz
#define CLKCONCMD_tickspd 0x38 //timer ticks output setting.000-32mhz,001-16mhz,010-8mhz,011-4mhz,100-2mhz,101-1mhz,110-500khz,111-250khz
#define CLKCONCMD_osc 0x40 //system clk source select.0-32mhz,1-16mhz
#define CLKCONCMD_osc32k 0x80 //32k clk source select.0-32khz xosc,1-32khz rcosc
#define CLKCONSTA_clkspd 0x07 //clock speed.000-32mhz,001-16mhz,010-8mhz,011-4mhz,100-2mhz,101-1mhz,110-500khz,111-250khz
#define CLKCONSTA_tickspd 0x38 //timer ticks output setting.000-32mhz,001-16mhz,010-8mhz,011-4mhz,100-2mhz,101-1mhz,110-500khz,111-250khz
#define CLKCONSTA_osc 0x40 //system clk source select.0-32mhz,1-16mhz
#define CLKCONSTA_osc32k 0x80 //32k clk source select.0-32khz xosc,1-32khz rcosc
//reset
#define CLD_en 0x01 //clk loss detector enable
//flash
#define FCTL_erase 0x01 //page erase
#define FCTL_write 0x02 //write
#define FCTL_cm 0x0c //cache mode.00-cache disabled,01/10/11-cache enabled
#define FCTL_abort 0x20 //abort status
#define FCTL_full 0x40 //write buffer full status
#define FCTL_busy 0x80 //write/erase in operation
//io
#define PERCFG_u0cfg 0x01 //usart0 io location
#define PERCFG_u1cfg 0x02 //usart1 io location
#define PERCFG_t4cfg 0x10 //timer4 io location
#define PERCFG_t3cfg 0x20 //timer3 io location
#define PERCFG_t1cfg 0x40 //timer1 io location
#define P2SEL_selp2_0 0x01 //p2.0 func select
#define P2SEL_selp2_3 0x02 //p2.3 func select
#define P2SEL_selp2_4 0x04 //p2.4 func select
#define P2SEL_pri0p1 0x08 //port1 peripheral priority control.0-usart0,1-timer1
#define P2SEL_pri1p1 0x10 //port1 peripheral priority control.0-timer1,1-timer4
#define P2SEL_pri2p1 0x20 //port1 peripheral priority control.0-usart1,1-timer3
#define P2SEL_pri3p1 0x40 //port1 peripheral priority control.0-usart0,1-usart1
#define P2DIR_dirp2 0x1f //p2.4~0 io direction.0-input,1-output
#define P2DIR_prip0 0xc0 //port0 peripheral priority control
#define P1INP_mdp1 0xfc //port1 input mode
#define P2INP_mdp2 0x1f //p2.4~0 io input mode
#define P2INP_pdup0 0x20 //port0 up/down select
#define P2INP_pdup1 0x40 //port1 up/down select
#define P2INP_pdup2 0x80 //port2 up/down select
#define P2IFG_p2if 0x1f //port2 input4~0 interrupt flag
#define P2IFG_dpif 0x20 //usb d+ interrupt flag
#define PICTL_p0icon 0x01 //port0 input7~0 interrupt config
#define PICTL_p1iconl 0x02 //port1 input3~0 interrupt config
#define PICTL_p1iconh 0x04 //port1 input7~4 interrupt config
#define PICTL_p2icon 0x08 //port2 input4~0 interrupt config
#define PICTL_padsc 0x80 //drive strength control for io pins
#define P2IEN_p2_ien 0x1f //port p2.4~0 interrupt enable
#define P2IEN_dpien 0x20 //usb d+ interrupt enable
#define PMUX_dregstapin 0x07 //digital regulator status pin
#define PMUX_dregsta 0x08 //digital regulator status
#define PMUX_ckopin 0x70 //clk out pin
#define PMUX_ckoen 0x80 //clk out enable
#define OBSSEL0_sel 0x7f //select output signal on output 0
#define OBSSEL0_en 0x80 //bit controllng the observation output 0
#define OBSSEL1_sel 0x7f //select output signal on output 1
#define OBSSEL1_en 0x80 //bit controllng the observation output 1
#define OBSSEL2_sel 0x7f //select output signal on output 2
#define OBSSEL2_en 0x80 //bit controllng the observation output 2
#define OBSSEL3_sel 0x7f //select output signal on output 3
#define OBSSEL3_en 0x80 //bit controllng the observation output 3
#define OBSSEL4_sel 0x7f //select output signal on output 4
#define OBSSEL4_en 0x80 //bit controllng the observation output 4
#define OBSSEL5_sel 0x7f //select output signal on output 5
#define OBSSEL5_en 0x80 //bit controllng the observation output 5
//dma
#define DMAARM_dmaarm0 0x01 //dma arm channel 0
#define DMAARM_dmaarm1 0x02 //dma arm channel 1
#define DMAARM_dmaarm2 0x04 //dma arm channel 2
#define DMAARM_dmaarm3 0x08 //dma arm channel 3
#define DMAARM_dmaarm4 0x10 //dma arm channel 4
#define DMAARM_abort 0x80 //dma abort
#define DMAREQ_dmareq0 0x01 //dma transfer request channel 0
#define DMAREQ_dmareq1 0x02 //dma transfer request channel 1
#define DMAREQ_dmareq2 0x04 //dma transfer request channel 2
#define DMAREQ_dmareq3 0x08 //dma transfer request channel 3
#define DMAREQ_dmareq4 0x10 //dma transfer request channel 4
#define DMAIRQ_dmaif0 0x01 //dma channel 0 interrupt flag
#define DMAIRQ_dmaif1 0x02 //dma channel 1 interrupt flag
#define DMAIRQ_dmaif2 0x04 //dma channel 2 interrupt flag
#define DMAIRQ_dmaif3 0x08 //dma channel 3 interrupt flag
#define DMAIRQ_dmaif4 0x10 //dma channel 4 interrupt flag
//timer1
#define T1CTL_mode 0x03 //timer1 mode select
#define T1CTL_div 0x0c //prescaler divider value
#define T1STAT_ch0if 0x01 //timer1 channel 0 interrupt flag
#define T1STAT_ch1if 0x02 //timer1 channel 1 interrupt flag
#define T1STAT_ch2if 0x04 //timer1 channel 2 interrupt flag
#define T1STAT_ch3if 0x08 //timer1 channel 3 interrupt flag
#define T1STAT_ch4if 0x10 //timer1 channel 4 interrupt flag
#define T1STAT_ovfif 0x20 //timer1 counter overflow interrupt flag
#define T1CCTL0_cap 0x03 //channel 0 capture mode select
#define T1CCTL0_mode 0x04 //channel 0 mode select
#define T1CCTL0_cmp 0x38 //channel 0 compare mode select
#define T1CCTL0_im 0x40 //channel 0 interrupt mask
#define T1CCTL0_rfirq 0x80 //rf interrupt for capture
#define T1CCTL1_cap 0x03 //channel 1 capture mode select
#define T1CCTL1_mode 0x04 //channel 1 mode select
#define T1CCTL1_cmp 0x38 //channel 1 compare mode select
#define T1CCTL1_im 0x40 //channel 1 interrupt mask
#define T1CCTL1_rfirq 0x80 //rf interrupt for capture
#define T1CCTL2_cap 0x03 //channel 2 capture mode select
#define T1CCTL2_mode 0x04 //channel 2 mode select
#define T1CCTL2_cmp 0x38 //channel 2 compare mode select
#define T1CCTL2_im 0x40 //channel 2 interrupt mask
#define T1CCTL2_rfirq 0x80 //rf interrupt for capture
#define T1CCTL3_cap 0x03 //channel 3 capture mode select
#define T1CCTL3_mode 0x04 //channel 3 mode select
#define T1CCTL3_cmp 0x38 //channel 3 compare mode select
#define T1CCTL3_im 0x40 //channel 3 interrupt mask
#define T1CCTL3_rfirq 0x80 //rf interrupt for capture
#define T1CCTL4_cap 0x03 //channel 4 capture mode select
#define T1CCTL4_mode 0x04 //channel 4 mode select
#define T1CCTL4_cmp 0x38 //channel 4 compare mode select
#define T1CCTL4_im 0x40 //channel 4 interrupt mask
#define T1CCTL4_rfirq 0x80 //rf interrupt for capture
#define IRCTL_irgen 0x01 //ir generation control
//timer3/4
#define T3CTL_mode 0x03 //timer3 mode
#define T3CTL_clr 0x04 //clear counter
#define T3CTL_ovfim 0x08 //overflow interrupt mask
#define T3CTL_start 0x10 //start timer
#define T3CTL_div 0xe0 //prescaler divider value
#define T3CCTL0_cap 0x03 //channel 0 capture mode select
#define T3CCTL0_mode 0x04 //channel 0 mode select
#define T3CCTL0_cmp 0x38 //channel 0 compare mode select
#define T3CCTL0_im 0x40 //channel 0 interrupt mask
#define T3CCTL1_cap 0x03 //channel 1 capture mode select
#define T3CCTL1_mode 0x04 //channel 1 mode select
#define T3CCTL1_cmp 0x38 //channel 1 compare mode select
#define T3CCTL1_im 0x40 //channel 1 interrupt mask
#define T4CTL_mode 0x03 //timer4 mode
#define T4CTL_clr 0x04 //clear counter
#define T4CTL_ovfim 0x08 //overflow interrupt mask
#define T4CTL_start 0x10 //start timer
#define T4CTL_div 0xe0 //prescaler divider value
#define T4CCTL0_cap 0x03 //channel 0 capture mode select
#define T4CCTL0_mode 0x04 //channel 0 mode select
#define T4CCTL0_cmp 0x38 //channel 0 compare mode select
#define T4CCTL0_im 0x40 //channel 0 interrupt mask
#define T4CCTL1_cap 0x03 //channel 1 capture mode select
#define T4CCTL1_mode 0x04 //channel 1 mode select
#define T4CCTL1_cmp 0x38 //channel 1 compare mode select
#define T4CCTL1_im 0x40 //channel 1 interrupt mask
#define TIMIF_t3ovfif 0x01 //timer3 overflow interrupt flag
#define TIMIF_t3ch0if 0x02 //timer3 channel 0 interrupt flag
#define TIMIF_t3ch1if 0x04 //timer3 channel 1 interrupt flag
#define TIMIF_t4ovfif 0x08 //timer4 overflow interrupt flag
#define TIMIF_t4ch0if 0x10 //timer4 channel 0 interrupt flag
#define TIMIF_t4ch1if 0x20 //timer4 channel 1 interrupt flag
#define TIMIF_ovfim 0x40 //timer1 overflow interrupt mask
//sleep timer
#define STLOAD_ldrdy 0x01 //load ready
#define STCC_pin 0x07 //pin select
#define STCC_port 0x18 //port select
#define STCS_valid 0x01 //capture valid flag
//adc
#define ADCL_adc 0xfc //
#define ADCCON1_stsel 0x30 //start select
#define ADCCON1_st 0x40 //start conversion
#define ADCCON1_eoc 0x80 //end of conversion
#define ADCCON2_sch 0x0f //sequence channel select
#define ADCCON2_sdiv 0x30 //decimation rate for channel
#define ADCCON2_sref 0xc0 //select reference voltage for conversion
#define ADCCON3_ech 0x0f //single channel select
#define ADCCON3_ediv 0x30 //decimation rate for extra channel
#define ADCCON3_eref 0xc0 //select reference voltage for extra conversion
#define TR0_adctm 0x01 //connect the temperature sensor
//battery monitor
#define BATTMON_battmon_pd 0x01 //turn on the battery monitor
#define BATTMON_battmon_voltage 0x3e //control the trigger point
#define BATTMON_battmon_out 0x40 //battery monitor
#define MONMUX_battmon_input 0x01 //input to battery monitor
//rng
#define ADCCON1_rctrl 0x0c //control the 16bit rng
//aes
#define ENCCS_st 0x01 //start processing command
#define ENCCS_cmd 0x06 //command
#define ENCCS_rdy 0x08 //enc/dec ready
#define ENCCS_mode 0x70 //mode
//wdg
#define WDCTL_int 0x03 //timer interval select
#define WDCTL_mode 0x0c //mode select
#define WDCTL_clr 0xf0 //clear timer
//usart
#define U0CSR_active 0x01 //usart tx/rx active status
#define U0CSR_tx_byte 0x02 //tx byte status
#define U0CSR_rx_byte 0x04 //rx byte status
#define U0CSR_err 0x08 //parity error status
#define U0CSR_fe 0x10 //framing error status
#define U0CSR_slave 0x20 //spi master/slave mode
#define U0CSR_re 0x40 //uart rx enable
#define U0CSR_mode 0x80 //usart mode select
#define U0UCR_start 0x01 //uart start-bit level
#define U0UCR_stop 0x02 //uart stop-bit level
#define U0UCR_spb 0x04 //uart number of stop bits
#define U0UCR_parity 0x08 //uart parity enable
#define U0UCR_bit9 0x10 //enable parity bit transfer
#define U0UCR_d9 0x20 //parity enable
#define U0UCR_flow 0x40 //uart hardware flow enable
#define U0UCR_flush 0x80 //flush unit
#define U0GCR_baud_e 0x1f //baud rate exponent value
#define U0GCR_order 0x20 //bit order for transfer
#define U0GCR_cpha 0x40 //spi clk phase
#define U0GCR_cpol 0x80 //spi clk polarity
#define U1CSR_active 0x01 //usart tx/rx active status
#define U1CSR_tx_byte 0x02 //tx byte status
#define U1CSR_rx_byte 0x04 //rx byte status
#define U1CSR_err 0x08 //parity error status
#define U1CSR_fe 0x10 //framing error status
#define U1CSR_slave 0x20 //spi master/slave mode
#define U1CSR_re 0x40 //uart rx enable
#define U1CSR_mode 0x80 //usart mode select
#define U1UCR_start 0x01 //uart start-bit level
#define U1UCR_stop 0x02 //uart stop-bit level
#define U1UCR_spb 0x04 //uart number of stop bits
#define U1UCR_parity 0x08 //uart parity enable
#define U1UCR_bit9 0x10 //enable parity bit transfer
#define U1UCR_d9 0x20 //parity enable
#define U1UCR_flow 0x40 //uart hardware flow enable
#define U1UCR_flush 0x80 //flush unit
#define U1GCR_baud_e 0x1f //baud rate exponent value
#define U1GCR_order 0x20 //bit order for transfer
#define U1GCR_cpha 0x40 //spi clk phase
#define U1GCR_cpol 0x80 //spi clk polarity
//operational amplifier
#define OPAMPMC_mode 0x03 //operational amplifier mode
#define OPAMPC_en 0x01 //operational amplifier enable
#define OPAMPC_cal 0x02 //start calibration
#define OPAMPS_cal_busy 0x01 //calibration in progress
//analog comparator
#define CMPCTL_output 0x01 //comparator output
#define CMPCTL_en 0x02 //comparator enable
//usb
#define USBADDR_usbaddr 0x7f //device address
#define USBADDR_update 0x80 //address effective flag
#define USBPOW_suspend_en 0x01 //suspend enable
#define USBPOW_suspend 0x02 //suspend mode entered
#define USBPOW_resume 0x04 //drive resume signal for remote wakeup
#define USBPOW_rst 0x08 //during reset signal
#define USBPOW_iso_wait_sof 0x80 //zero-length packets is asserted
#define USBIIF_ep0if 0x01 //interrupt flag for ep0
#define USBIIF_inep1if 0x02 //interrupt flag for in ep1
#define USBIIF_inep2if 0x04 //interrupt flag for in ep2
#define USBIIF_inep3if 0x08 //interrupt flag for in ep3
#define USBIIF_inep4if 0x10 //interrupt flag for in ep4
#define USBIIF_inep5if 0x20 //interrupt flag for in ep5
#define USBOIF_outep1if 0x02 //interrupt flag for out ep1
#define USBOIF_outep2if 0x04 //interrupt flag for out ep2
#define USBOIF_outep3if 0x08 //interrupt flag for out ep3
#define USBOIF_outep4if 0x10 //interrupt flag for out ep4
#define USBOIF_outep5if 0x20 //interrupt flag for out ep5
#define USBCIF_suspendif 0x01 //suspend interrupt flag
#define USBCIF_resumeif 0x02 //resume interrupt flag
#define USBCIF_rstif 0x04 //reset interrupt flag
#define USBCIF_sofif 0x08 //sof interrupt flag
#define USBIIE_ep0ie 0x01 //ep0 interrupt enable
#define USBIIE_inep1ie 0x02 //in ep1 interrupt enable
#define USBIIE_inep2ie 0x04 //in ep2 interrupt enable
#define USBIIE_inep3ie 0x08 //in ep3 interrupt enable
#define USBIIE_inep4ie 0x10 //in ep4 interrupt enable
#define USBIIE_inep5ie 0x20 //in ep5 interrupt enable
#define USBOIE_outep1ie 0x02 //out ep1 interrupt enable
#define USBOIE_outep2ie 0x04 //out ep2 interrupt enable
#define USBOIE_outep3ie 0x08 //out ep3 interrupt enable
#define USBOIE_outep4ie 0x10 //out ep4 interrupt enable
#define USBOIE_outep5ie 0x20 //out ep5 interrupt enable
#define USBCIE_suspendif 0x01 //suspend interrupt flag
#define USBCIE_resumeif 0x02 //resume interrupt flag
#define USBCIE_rstif 0x04 //reset interrupt flag
#define USBCIE_sofif 0x08 //sof interrupt flag
#define USBINDEX_usbindex 0x0f //ep select
#define USBCTRL_usb_en 0x01 //usb enable
#define USBCTRL_pll_en 0x02 //48mhz usb pll enable
#define USBCTRL_pll_locked 0x80 //pll locked status
#define USBCS0_outpkt_rdy 0x01 //data packet received
#define USBCS0_inpkt_rdy 0x02 //data has been loaded into the ep0 fifo
#define USBCS0_sent_stall 0x04 //stall handshake has been sent
#define USBCS0_data_end 0x08 //signal the end of a data transfer
#define USBCS0_setup_end 0x10 //control transfer ends
#define USBCS0_send_stall 0x20 //terminate the current transaction
#define USBCS0_clr_outpkt_rdy 0x40 //de-assert the outpkt_rdy
#define USBCS0_clr_setup_end 0x80 //de-assert the setup_end
#define USBCSIL_inpkt_rdy 0x01 //data packet has been loaded into the in fifo
#define USBCSIL_pkt_present 0x02 //at least 1 packet in the in fifo
#define USBCSIL_underrun 0x04 //if an in token is received
#define USBCSIL_flush_packet 0x08 //flush next packet ready to transfer from in fifo
#define USBCSIL_send_stall 0x10 //make usb controller reply with a stall handshake
#define USBCSIL_sent_stall 0x20 //when a stall handshake has been sent
#define USBCSIL_clr_data_tog 0x40 //reset the data toggle
#define USBCSIH_in_dbl_buf 0x01 //double buffering enable
#define USBCSIH_force_data_tog 0x08 //force the in ep data toggle to switch
#define USBCSIH_iso 0x40 //select in ep type
#define USBCSIH_autoset 0x80 //inpkt_rdy automatically asserted
#define USBCSOL_outpkt_rdy 0x01 //packet has been received
#define USBCSOL_fifo_full 0x02 //no more packet can loaded into out fifo
#define USBCSOL_overrun 0x04 //use when out packet can't loaded into out fifo
#define USBCSOL_data_error 0x08 //crc or bit-stuff error in the packet received
#define USBCSOL_flush_packet 0x10 //flush the next packet read from out fifo
#define USBCSOL_send_stall 0x20 //make the usb controller reply with a stall handshake
#define USBCSOL_sent_stall 0x40 //when a stall handshake has been sent
#define USBCSOL_clr_data_tog 0x80 //force the next data packet to be a data0 packet
#define USBCSOH_out_dbl_buf 0x01 //double buffering enable
#define USBCSOH_iso 0x40 //select out ep type
#define USBCSOH_autoclear 0x80 //outpkt_rdy automatically cleared
#define USBCNT0_usbcnt0 0x3f //number of rx bytes into ep0 fifo
#define USBCNTH_usbcnt 0x07 //rx bytes in out fifo
//timer2(mac)
#define T2MSEL_t2msel 0x07 //value select
#define T2MSEL_t2movfsel 0x70 //
#define T2IRQF_timer2_perf 0x01 //counted to a value
#define T2IRQF_timer2_compare1f 0x02 //counts to a value 
#define T2IRQF_timer2_compare2f 0x04 //counts to a value
#define T2IRQF_timer2_ovf_perf 0x08 //counts to a value
#define T2IRQF_timer2_ovf_compare1f 0x10 //counts to a value
#define T2IRQF_timer2_ovf_compare2f 0x20 //counts to a value
#define T2IRQM_timer2_perm 0x01 //enable timer2_per interrupt
#define T2IRQM_timer2_compare1m 0x02 //enable timer2_compare1 interrupt
#define T2IRQM_timer2_compare2m 0x04 //enable timer2_compare2 interrupt
#define T2IRQM_timer2_ovf_perm 0x08 //enable timer2_ovf_per interrupt
#define T2IRQM_timer2_ovf_compare1m 0x10 //enable timer2_ovf_compare1 interrupt
#define T2IRQM_timer2_ovf_compare2m 0x20 //enable timer2_ovf_compare2 interrupt
#define T2CTRL_run 0x01 //start/stop timer
#define T2CTRL_sync 0x02 //start/stop timer trigger
#define T2CTRL_state 0x04 //timer idle/running
#define T2CTRL_latch_mode 0x08 //
#define T2ENVCFG_timer2_event1_cfg 0x07 //select event trigger t2_event1
#define T2ENVCFG_timer2_event2_cfg 0x70 //select event trigger t2_event2
//rf
#define RFSTAT_rx_active 0x01 //in rx state
#define RFSTAT_tx_active 0x02 //in tx state
#define RFSTAT_lock_status 0x04 //0-pll unlock,1-pll in lock
#define RFSTAT_cal_running 0x08 //frequency synth calibration status
#define RFSTAT_sfd 0x10 //access address has been in tx or sync has been in rx
#define RFSTAT_dem_status 0x60 //demodulator status
#define RFSTAT_mod_underflow 0x80 //modulator has underflowed
#define RFC_OBS_CTRL0_rfc_obs_mux0 0x3f //control
#define RFC_OBS_CTRL0_rfc_obs_pol0 0x40 //control
#define RFC_OBS_CTRL1_rfc_obs_mux1 0x3f //control
#define RFC_OBS_CTRL1_rfc_obs_pol1 0x40 //control
#define RFC_OBS_CTRL2_rfc_obs_mux2 0x3f //control
#define RFC_OBS_CTRL2_rfc_obs_pol2 0x40 //control
#define ATEST_atest_ctrl 0x3f //control analog test mode

#endif