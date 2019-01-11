//NBIot driver(NEUL BC35-G)
#include <string.h>
#include "stdlib.h"
#include "at.h"
#include "los_sys.h"
#include "los_tick.h"
#include "los_task.ph"
#include "los_config.h"
#include "los_sem.h"
#include "los_dev_st_uart.h"

#define NEUL_MAX_BUF_SIZE 1064
#define NEUL_MAX_SOCKETS 1 // the max udp socket links
#define NEUL_IP_LEN 16 //ipv4 address including '\0'
#define NEUL_MANUFACT_LEN 20

static char at_rbuf[NEUL_MAX_BUF_SIZE];
static char at_wbuf[NEUL_MAX_BUF_SIZE];
static char at_tmpbuf[NEUL_MAX_BUF_SIZE];

int uart_data_read(char *buf, int maxrlen, int mode, int timeout);
int uart_data_write(char *buf, int writelen, int mode);

static neul_dev_operation_t neul_ops = 
{
    uart_data_read,
    uart_data_write
};
static remote_info udp_socket[NEUL_MAX_SOCKETS]={{0,-1,{0}}};
static neul_dev_info_t neul_dev= 
{
    at_rbuf,
    at_wbuf,
    0,
    udp_socket,
    &neul_ops,
    {0}
};

//maxlen:read max byte length
//mode:0-block,1-nonblock
//timeout:ms
int at_unit(int maxlen,int mode,int timeout)
{
	char *str;
	int ret;
	
	//send cmd
	if (!neul_dev.wbuf)
		return ERR_WRITE_EMPTY;
	ret=neul_dev.ops->dev_write(neul_dev.wbuf,strlen(neul_dev.wbuf),0);
	if (ret<0)//write data to nbiot failed
		return ERR_WRITE_FAILED;
	//recv data
	if (!neul_dev.rbuf)
		return ERR_READ_EMPTY;
	memset(neul_dev.rbuf,0,maxlen);
	ret=neul_dev.ops->dev_read(neul_dev.rbuf,maxlen,mode,timeout);
	if (ret<=0)//read data from nbiot failed
		return ERR_READ_FAILED;
	//parse
	str=strstr(neul_dev.rbuf,"OK");
	if (!str)
		return ERR_PARSE_OK;

	return 0;
}

//mode:0-block,1-nonblock
int uart_data_read(char *buf, int maxrlen, int mode, int timeout)
{
    int length;

    if (!buf || maxrlen<=0 || timeout<0)
        return 0;
    if (mode==0)
        length=los_dev_uart_read(LOS_STM32L431_UART2, buf, maxrlen, timeout);
    else
        length=los_dev_uart_read(LOS_STM32L431_UART2, buf, maxrlen, 0);

    return length;
}

int uart_data_write(char *buf, int writelen, int mode)
{
    if (!buf || writelen<=0)
        return 0;
    los_dev_uart_write(LOS_STM32L431_UART2, buf, writelen, 500);
		
    return 0;
}

int uart_data_flush(void)
{
    memset(at_rbuf,0,NEUL_MAX_BUF_SIZE);

    return 0;
}

//--3gpp commands--//
//Display Product Identification Information
//0-ok,!0=err
int at_get_pid(void)
{
    int ret;

	memcpy(neul_dev.wbuf,"ATI\r",sizeof("ATI\r"));
	ret=at_unit(20,0,200);

	return ret;
}

//Set Command Echo Mode
//0-ok,!0=err
int at_set_echo(int mode)
{
    int ret;

	memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf,"ATE%d\r",mode);
	ret=at_unit(20,0,200);

	return ret;
}

//Request Manufacturer Identification
//0-ok,!0=err
int at_get_mid(void)
{
    int ret;

	memcpy(neul_dev.wbuf,"AT+CGMI\r",sizeof("AT+CGMI\r"));
	ret=at_unit(20,0,200);
	if (ret<0)
		return ret;
    if (!strstr(neul_dev.rbuf,"Quectel"))
        return ERR_PARSE_NONE;

    return 0;
}

//Request Manufacture Model
//0-ok,!0=err
int at_get_model(void)
{
    int ret;

	memcpy(neul_dev.wbuf,"AT+CGMM\r",sizeof("AT+CGMM\r"));
	ret=at_unit(20,0,200);

    return ret;
}

//Request Manufacturer Revision
//0-ok,!0=err
int at_get_revision(void)
{
    int ret;

	memcpy(neul_dev.wbuf,"AT+CGMR\r",sizeof("AT+CGMR\r"));
	ret=at_unit(20,0,200);

    return ret;
}


//Request Product Serial Number
//0-ok,!0=err
int at_get_sn(int type)
{
    int ret;

	memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf,"AT+CGSN%d\r",type);
	ret=at_unit(64,0,200);
	if (ret<0)
		return ret;
	if (!strstr(neul_dev.rbuf,"+CGSN:") || strlen(neul_dev.rbuf)<22)
		return ERR_PARSE_NONE;
	
	return 0;
}

//Get Network Registration Status
//>0-ok,<0-err
int at_get_cereg(void)
{
	int ret;
	int i;
	
	memcpy(neul_dev.wbuf,"AT+CEREG?\r",sizeof("AT+CEREG?\r"));
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r+CEREG:%d,%d\r",&ret,&i);
		
    return i;
}

//Get Signalling Connection Status
//>0-ok,<0-err
int at_get_cscon(void)
{
	int ret;
	int i;
		
	memcpy(neul_dev.wbuf,"AT+CSCON?\r",sizeof("AT+CSCON?\r"));
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r+CSCON:%d,%d\r",&ret,&i);
		
    return i;
}

//Get Signal Strength Indicator
//>0-ok,<0-err
int at_get_csq(void)
{
	int ret;
	int i;
		
	memcpy(neul_dev.wbuf,"AT+CSQ\r",sizeof("AT+CSQ\r"));
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r+CSQ:%d,",&i);
		
    return i;
}

//Get PDP Address
//0-get ip,!0-no ip
int at_get_ip(char *buf)
{
    int ret;
    
	memcpy(neul_dev.wbuf,"AT+CGPADDR\r",sizeof("AT+CGPADDR\r"));
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r+CGPADDR:%d,%s\r",&ret,buf);
    
    return 0;
}

//Get Connect State
//0-connected,!0-not connected
int at_get_connect(void)
{
    int ret;
	int i;
    
	memcpy(neul_dev.wbuf,"AT+CGATT?\r",sizeof("AT+CGATT?\r"));
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r+CGATT:%d\r",&i);
    
    return i;
}

//Request International Mobile Subscriber Identity
//0-ok,!0-err
int at_get_imsi(void)
{
    int ret;
    
	memcpy(neul_dev.wbuf,"AT+CIMI\r",sizeof("AT+CIMI\r"));
	ret=at_unit(32,0,200);
    
    return ret;
}

//Get NBIoT Functionality
//>0-ok,<0-err
int at_get_function(void)
{
	int ret;
	int i;
		
	memcpy(neul_dev.wbuf,"AT+CFUN?\r",sizeof("AT+CFUN?\r"));
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r+CFUN:%d,",&i);
		
    return i;
}

//Set Error Report
//mode:0-disable,1-enable
int at_set_error(int mode)
{
	int ret;
	
	memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf,"AT+CMEE=%d\r",mode);
	ret=at_unit(100,0,200);
	
	return ret;
}

//Return Current Date & Time
//0-ok,!0-err
int at_get_time(char *buf)
{
	int ret;

	memcpy(neul_dev.wbuf,"AT+CCLK?\r",sizeof("AT+CCLK?\r"));
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r+CCLK:%s",buf);
	
    return 0;
}

//--general commands--//
//Reboot NBIot
//0-ok,!0-err
int at_reboot(void)
{
    int ret;
    
	memcpy(neul_dev.wbuf,"AT+NRB\r",sizeof("AT+NRB\r"));
	ret=at_unit(64,0,200);
	if (ret<0)
		return ret;
    if (!strstr(neul_dev.rbuf,"REBOOTING"))
      return ERR_PARSE_NONE;
    
    return 0;
}

//Query UE Statistics
//0-ok,!0-err
int at_get_statistics(void)
{
	int ret;	

	memcpy(neul_dev.wbuf,"AT+NUESTATS\r",sizeof("AT+NUESTATS\r"));
	ret=at_unit(1000,0,200);
	if (ret<0)
		return ret;
	sscanf(neul_dev.rbuf,"\rSignal power:%s\rTotal power:%s\rTX power:%s\rTX time:%s\rRX time:%s\rCell ID:%s\rECL:%s\rSNR:%s\rEARFCN:%s\rPCI:%s\rRSRQ:%s\r",
							g_netdata.signal_power,
							g_netdata.total_power,
							g_netdata.tx_power,
							g_netdata.tx_time,
							g_netdata.rx_time,
							g_netdata.scell_id,
							g_netdata.ecl,
							g_netdata.snr,
							g_netdata.earfcn,
							g_netdata.pci,
							g_netdata.rsrq);
	
    return 0;
}

//create udp/tcp socket
//protocol:PROTOCOL_UDP-udp,PROTOCOL_TCP-tcp
//port:local port socket used,0 for random port
//>=0-socket handle,<0-err
int at_open_udptcp(int protocol,uint16 port)
{
    int ret;
    int i;
	
    memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf,"AT+NSOCR=%s,%d,1\r",protocol==PROTOCOL_UDP ? "DGRAM,17" : "STREAM,6",port);
	ret=at_unit(32,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"%d\r",&i);
    if (i<0)
		return ERR_PARAM;
		
	return i;
}

//close udp/tcp socket
//socket:the socket handle that need close
//0-ok,!0-err
int at_close_udptcp(int socket)
{
    int ret;
    
	if (socket<0)
        return ERR_PARAM;
    memset(neul_dev.wbuf,0,64);
    sprintf(neul_dev.wbuf,"AT+NSOCL=%d\r",socket);
	ret=at_unit(32,0,200);
	if (ret<0)
		return ret;
    neul_dev.remotecount--;
    (neul_dev.addrinfo+socket)->port=0;
    (neul_dev.addrinfo+socket)->socket=-1;
    memset((neul_dev.addrinfo+socket)->ip,0,NEUL_IP_LEN);
    
    return 0;
}

//udp send data
//socket:input param , the data will send to this socket
//buf:input param, the data buf
//length:input param, the send data length
//>=0-recv data length,!0-err
int at_send_udp(int socket,const char *buf,int length)
{
    int ret;
	int i;

	if (socket<0 || !buf || !length)
		return ERR_PARAM;
    memset(at_tmpbuf, 0, NEUL_MAX_BUF_SIZE);
    ascii2string((const char *)buf,length,at_tmpbuf);
    memset(neul_dev.wbuf,0,NEUL_MAX_BUF_SIZE);
    sprintf(neul_dev.wbuf,"AT+NSOST=%d,%s,%d,%d,%s\r",socket,(neul_dev.addrinfo+socket)->ip,(neul_dev.addrinfo+socket)->port,length,at_tmpbuf);
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
	sscanf(neul_dev.rbuf,"\r+NSONMI:%d,%d\r",&socket,&i);
		
    return i;
}

//tcp connect
//socket:input param , the data will send to this socket
//ip:input param, the remote ip address
//port:input param, remote ip's port
//0-ok,!0-err
int at_conn_tcp(int socket,const char *ip,uint16 port)
{
	int ret;
    
	if (!ip || strlen(ip)>=NEUL_IP_LEN || port==0 || socket<0)
        return ERR_PARAM;
    memset(neul_dev.wbuf,0,64);
    sprintf(neul_dev.wbuf,"AT+NSOCO=%d,%s,%d\r",socket,ip,port);
	ret=at_unit(32,0,200);
    neul_dev.remotecount++;
    (neul_dev.addrinfo+socket)->port=port;
    (neul_dev.addrinfo+socket)->socket=socket;
    memcpy((neul_dev.addrinfo+socket)->ip,ip,strlen(ip));
	
	return ret;
}

//tcp send data
//socket:input param , the data will send to this socket
//buf:input param, the data buf
//length:input param, the send data length
//>=0-recv data length,!0-err
int at_send_tcp(int socket,const char *buf,int length)
{
	int ret;
	int sequence,i;
    
    memset(neul_dev.wbuf,0,64);
    sprintf(neul_dev.wbuf,"AT+NSOSD=%d,%d,%s,0x100,101\r",socket,length,buf);//priority,sequence
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
	sscanf(neul_dev.rbuf,"\r+NSOSTR:%d,%d,%d\r",&socket,&sequence,&i);
	if (!i)
		return ERR_PARAM;
	
	return ret;
}

//recv data from socket(udp/tcp)
//socket:input param , the data will read from this socket
//buf:out param, store the data read
//maxlen:input param, the max read data length
//>0-data length,<0-err
int at_recv_udptcp(int socket,char *buf,int maxlen)
{
    int ret;
    int port,length,len;

    if (socket<0 || !buf || maxlen<=0)
        return ERR_PARAM;
	memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf, "AT+NSORF=%d,%d\r",socket,maxlen);
	ret=at_unit(200,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r%d,%s,%d,%d,%s,%d",&socket,
                            at_tmpbuf,
                            &port,
                            &length,
                            at_tmpbuf+22,
                            &len);
    if (length>0)
        string2ascii((const uint8*)(at_tmpbuf+22),length*2,buf);

    return length;
}

//Connectivity Test to Remote Host
int at_ping(const char *ip)
{
	int ret;
	
	if (!ip || strlen(ip)>=NEUL_IP_LEN)
		return ERR_PARAM;
	memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf, "AT+NPING=%s\r",ip);
	ret=at_unit(200,0,200);
	
	return ret;
}

//band:input param, nbiot work band BC95-CM set to 8, BC95-SL set to 5, BC95-VF set to 20
//0-ok,!0-err
int at_set_band(int band)
{
    int ret;
	
	if (band!=8 && band!=5 && band!=20)
		return ERR_PARAM;
    memset(neul_dev.wbuf,0,16);
	sprintf(neul_dev.wbuf,"AT+NBAND=%d\r",band);
	ret=at_unit(16,0,200);

    return ret;
}

//Set NBIoT's Behaviour
//flag.bit0:0-manual connect,1-auto connect
//0-ok,!0-err
int at_set_config(int flag)
{
    int ret;

	memset(neul_dev.wbuf,0,100);
	sprintf(neul_dev.wbuf,"AT+NCONFIG=AUTOCONNECT,%s\r",(flag & 0x01) ? "TRUE" : "FALSE");
	ret=at_unit(32,0,300);

	return ret;
}

//Get NBIoT's Behaviour
//0-ok,!0-err
int at_get_config(void)
{
    int ret;
	
	memcpy(neul_dev.wbuf,"AT+NCONFIG?\r",sizeof("AT+NCONFIG?\r"));
	ret=at_unit(106,0,500);
	if (ret<0)
		return ret;
    if (strstr(neul_dev.rbuf,"AUTOCONNECT,TRUE"))
		return 1;
    if (strstr(neul_dev.rbuf,"AUTOCONNECT,FALSE"))
		return 2;
		
    return ERR_PARAM;
}

//Configure UART Port Baud Rate
int at_set_uart(uint16 baud,int timeout,int store,int sync,int stop,int parity)
{
	int ret;

	memset(neul_dev.wbuf,0,100);
	sprintf(neul_dev.wbuf,"AT+NATSPEED=%d,%d,%d,%d,%d,%d\r",baud,timeout,store,sync,stop,parity);
	ret=at_unit(32,0,300);

	return ret;
}

//USIM Card Identification
int at_get_iccid(void)
{
	int ret;
	
	memcpy(neul_dev.wbuf,"AT+NCCID\r",sizeof("AT+NCCID\r"));
	ret=at_unit(50,0,500);
	
	return ret;
}

//Set Power Saving Mode Status
//>0-ok,<0-err
int at_set_npsmr(int mode)
{
	int ret;

    memset(neul_dev.wbuf,0,64);
    sprintf(neul_dev.wbuf,"AT+NPSMR=%d\r",mode);
	ret=at_unit(100,0,200);

    return ret;
}

//Request Power Saving Mode Status
//>0-ok,<0-err
int at_get_npsmr(void)
{
	int ret;
	int i;
	
	memcpy(neul_dev.wbuf,"AT+NPSMR?\r",sizeof("AT+NPSMR?\r"));
	ret=at_unit(100,0,200);
	if (ret<0)
		return ret;
    sscanf(neul_dev.rbuf,"\r+NPSMR:%d,%d\r",&ret,&i);
		
    return i;
}

//--huawei commands--//
//Set CDP Setting
//ip:input param cdp server ip address(huawei)
//0-ok,!0-err
int at_set_cdp(const char *ip)
{
    int ret;
    
    if (!ip || strlen(ip)>=NEUL_IP_LEN)
        return ERR_PARAM;
    memset(neul_dev.wbuf,0,64);
    sprintf(neul_dev.wbuf,"AT+NCDP=%s\r",ip);
	ret=at_unit(32,0,200);
		
	return ret;
}

//Get CDP Setting
//0-ok,!0-err
int at_get_cdp(void)
{
    int ret;

	memcpy(neul_dev.wbuf,"AT+NCDP?\r",sizeof("AT+NCDP?\r"));
	ret=at_unit(32,0,200);
    
    return ret;
}

//Data Encryption Mode
//type.bit0:0-none,1-dtls
//type.bit1:0-at every time,1-at poweron or ip modify
//0-ok,!0-err
int at_set_encryption(int type)
{
    int ret;

    memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf,"AT+QSECSWT=%d\r",type);
	ret=at_unit(32,0,200);
    
    return ret;
}

//Set PSK ID and PSK
int at_set_psk(const char *id,const char *psk)
{
    int ret;

    memset(neul_dev.wbuf,0,200);
	sprintf(neul_dev.wbuf,"AT+QSETPSK=%s,%s\r",id,psk);
	ret=at_unit(100,0,200);
    
    return ret;
}

//Register Control
//type:0-register,1-deregister
//0-ok,!0-err
int at_register(int type)
{
    int ret;

    memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf,"AT+QLWSREGIND=%d\r",type);
	ret=at_unit(32,0,200);
    
    return ret;
}

//Set Register Mode
//type:0-manual,1-automatic
//0-ok,!0-err
int at_set_register(int type)
{
    int ret;

    memset(neul_dev.wbuf,0,64);
	sprintf(neul_dev.wbuf,"AT+QREGSWT=%d\r",type);
	ret=at_unit(32,0,200);
    
    return ret;
}

//send lwm2m message(hex data) to cdp server
//buf:input param, the data that send to cdp
//length:input param, data length
//0-ok,!0-err
int at_send_lwm2m(const char *buf,int length)
{
    int ret;
    
    if (!buf || length>=512)
        return ERR_PARAM;
    memset(at_tmpbuf,0,NEUL_MAX_BUF_SIZE);
    ret = string2ascii((unsigned char *)buf, length, at_tmpbuf);
    memset(neul_dev.wbuf,0,200);
    sprintf(neul_dev.wbuf, "AT+QLWULDATA=%d,%s\r", length, at_tmpbuf);
	ret=at_unit(32,0,200);
		
	return ret;
}

//send coap message(hex data) to cdp server
//buf:input param, the data that send to cdp
//length:input param, data length
//0-ok,!0-err
int at_send_coap(const char *buf,int length)
{
    int ret;
    
    if (!buf || length>=512)
        return ERR_PARAM;
    memset(at_tmpbuf,0,NEUL_MAX_BUF_SIZE);
    ret = string2ascii((unsigned char *)buf, length, at_tmpbuf);
    memset(neul_dev.wbuf,0,64);
    sprintf(neul_dev.wbuf, "AT+NMGS=%d,%s\r", length, at_tmpbuf);
	ret=at_unit(100,0,200);
		
	return ret;
}

//recv coap message(hex data) from cdp server
//buf:input param, the data that recv from cdp
//maxlen:input param, data length
//>=0-recv length,<0-err
int at_recv_coap(char *buf,int maxlen)
{
    int ret;
	int i;

    if (!buf || maxlen==0)
		return ERR_PARAM;
	memcpy(neul_dev.wbuf,"AT+NMGR\r",sizeof("AT+NMGR\r"));
	ret=at_unit(NEUL_MAX_BUF_SIZE,0,540);
	if (ret<0)
		return ret;
    if (!strstr(neul_dev.rbuf,"+NNMI:"))
		return ERR_PARAM;
    sscanf(neul_dev.rbuf,"+NNMI:%d,%s",&i,at_tmpbuf);
    if (i<=0)
		return 0;
	ascii2string(at_tmpbuf,i*2,buf);
		
    return i;
}

//sent number of coap message
//>=0-recv buffered length,<0-err
int at_get_send(void)
{
	static int count=0;
	int i;
	int ret;

	memcpy(neul_dev.wbuf,"AT+NQMGS\r",sizeof("AT+NQMGS\r"));
	ret=at_unit(64,0,200);
	if (ret<0)
		return ret;
    if (!strstr(neul_dev.rbuf,"SENT="))
		return ERR_PARAM;
    sscanf(neul_dev.rbuf,"SENT=%d,%s",&i,neul_dev.wbuf);
	if (i==count)
		return ERR_PARAM;
	count=i;
		
    return 0;
}

//buffered number of coap message
//>=0-recv buffered length,<0-err
int at_get_recv(void)
{
    int ret;
	int i;

	memcpy(neul_dev.wbuf,"AT+NQMGR\r",sizeof("AT+NQMGR\r"));
	ret=at_unit(128,0,100);
	if (ret<0)
		return ret;
    if (!strstr(neul_dev.rbuf,"BUFFERED"))
		return 0;
    sscanf(neul_dev.rbuf,"BUFFERED=%d,%s",&i,neul_dev.wbuf);
    if (i<=0)
		return 0;

    return i;
}

//Message Registration Status
//>=0-ok,<0-err
int at_get_status(void)
{
    int ret;

	memcpy(neul_dev.wbuf,"AT+NMSTATUS?\r",sizeof("AT+NMSTATUS?\r"));
	ret=at_unit(32,0,200);
	if (ret<0)
		return ret;
    if (strstr(neul_dev.rbuf,"+NMSTATUS:UNINITIALISED"))
		return STATUS_UNINITIALISED;
    if (strstr(neul_dev.rbuf,"+NMSTATUS:INITIALISED"))
		return STATUS_INITIALISED;
    if (strstr(neul_dev.rbuf,"+NMSTATUS:REGISTERING"))
		return STATUS_REGISTERING;
    if (strstr(neul_dev.rbuf,"+NMSTATUS:MO_DATA_ENABLED"))
		return STATUS_MO_DATA_ENABLED;
    
    return 0;
}
