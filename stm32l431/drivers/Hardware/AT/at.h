#ifndef _at_H
#define _at_H

#include "include.h"

//define
#define PROTOCOL_UDP 0
#define PROTOCOL_TCP 1

#define ERR_WRITE_EMPTY -1
#define ERR_WRITE_FAILED -2
#define ERR_READ_EMPTY -3
#define ERR_READ_FAILED -4
#define ERR_PARSE_OK -5
#define ERR_PARSE_NONE -6
#define ERR_PARAM -7

#define STATUS_UNINITIALISED 1
#define STATUS_INITIALISED 2
#define STATUS_REGISTERING 3
#define STATUS_MO_DATA_ENABLED 4

typedef int (*neul_read)(char *buf, int maxlen, int mode, int timeout);
typedef int (*neul_write)(char *buf, int writelen, int mode);

typedef struct _neul_dev_operation_t 
{
    neul_read dev_read;
    neul_write dev_write;
} neul_dev_operation_t;

typedef struct _remote_info_t
{
    unsigned short port;
    int socket;
    char ip[16];
}remote_info;

typedef struct _neul_dev_info_t 
{
    char *rbuf;
    char *wbuf;
    int remotecount;
    remote_info *addrinfo;
    neul_dev_operation_t *ops;
    char cdpip[16];
} neul_dev_info_t;

int uart_data_flush(void);
//3gpp commands
int at_get_pid(void);
int at_set_echo(int mode);
int at_get_mid(void);
int at_get_model(void);
int at_get_revision(void);
int at_get_sn(int type);
int at_get_cereg(void);
int at_get_cscon(void);
int at_get_csq(void);
int at_get_ip(char *buf);
int at_get_connect(void);
int at_get_imsi(void);
int at_get_function(void);
int at_set_error(int mode);
int at_get_time(char *buf);
//general commands
int at_reboot(void);
int at_get_statistics(void);
int at_open_udptcp(int protocol,uint16 port);
int at_close_udptcp(int socket);
int at_send_udp(int socket,const char *buf,int length);
int at_conn_tcp(int socket,const char *ip,uint16 port);
int at_send_tcp(int socket,const char *buf,int length);
int at_recv_udptcp(int socket,char *buf,int maxlen);
int at_ping(const char *ip);
int at_set_band(int band);
int at_set_config(int flag);
int at_get_config(void);
int at_set_uart(uint16 baud,int timeout,int store,int sync,int stop,int parity);
int at_get_iccid(void);
int at_set_npsmr(int mode);
int at_get_npsmr(void);
//huawei commands
int at_set_cdp(const char *ip);
int at_get_cdp(void);
int at_set_encryption(int type);
int at_set_psk(const char *id,const char *psk);
int at_register(int type);
int at_set_register(int type);
int at_send_lwm2m(const char *buf,int length);
int at_send_coap(const char *buf,int length);
int at_recv_coap(char *buf,int maxlen);
int at_get_send(void);
int at_get_recv(void);
int at_get_status(void);

#endif
