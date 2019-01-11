#pragma once

//include
#include "layer.h"
//define
#define MAX_MASTER_SLAVE 5 //maybe 7
#define MIN_MASTER_SLAVE 3

#define MAX_METRIC 20 //max radius between bluetooth devices=10m(no adaptor)
//typedef
//struct
//function
void connect_recv(device_t *device);
void connect_seek(device_t *device);
void connect_send(device_t *device);
void connect_resend(device_t *device);
uint8 transaction_verify(device_t *device,transaction_t *transaction);
void transaction_recv(device_t *device);
void transaction_signature(transaction_t *transaction,device_t *device);
transaction_t *transaction_generate(device_t *device);
void transaction_send(device_t *device,transaction_t *transaction);
void ledger_recv(device_t *device);
void ledger_send(device_t *device);
void process_device(device_t *device);