#pragma once

//include
#include "include.h"
#include "crypt_rsa.h"
#include "crypt_sha256.h"
//define
#define NODE_NONE 0
#define NODE_HEAVY 1
#define NODE_LIGHT 2

#define STEP_CONNECT 0
#define STEP_TRANSACTION 1
#define STEP_LEDGER 2
#define STEP_MOVE 3

#define KEY_LEN 4 //ciper key byte
#define KEY_E 4 //key_e byte
#define KEY_MASK 0 //exponentially blind byte

//#define TRANSACTION_NONE 0 //not join in tips,not do address/ledger verify
#define TRANSACTION_TIP 0 //join in tips,not do address/ledger verify
#define TRANSACTION_DAG 1 //join in dags,do address/ledger verify
//#define TRANSACTION_MILESTONE 3 //

#define STATUS_DONE 0 //verify pass
#define STATUS_DEVICE 1 //address verify fail
#define STATUS_LEDGER 2 //ledger verify fail
#define STATUS_SRC 3 //src address
#define STATUS_DST 4 //dst address

#define TIMER_CONNECT 1 //connect update timer(heavy node->mainchain)

#define TIP_NUM 5 //tip width

//typedef
//struct
struct index_t
{
	uint32 number;//device index number
	uint32 *device_index;//device index array
	uint8 *key;//device key_e/n
	uint32 *token;//account token(0-available,1-frozen)
	uint8 *node;//NODE_HEAVY-heavy node,NODE_LIGHT-light node
};
struct deal_t
{
	uint32 device_index[2];//device index(single physical address).0-src,1-dst
	uint32 token;//transaction token
};
struct spv_t
{
	uint32 index;//transaction index
	deal_t deal;//transaction deal
	uint8 plain[KEY_LEN];//plaintext
	uint8 cipher[KEY_LEN];//ciphertext
};
struct ledger_t
{
	uint32 status;//transaction status(pass/fail)
	uint32 index;//transaction index
	uint32 device_index;//device index(single physical address)
	uint32 token;//transaction token
};
struct transaction_t
{
	uint32 index;//transaction index
	deal_t deal;//transaction deal
	uint8 plain[KEY_LEN];//plaintext
	uint8 cipher[KEY_LEN];//ciphertext
	uint32 pow[2];//trunk/branch's pow
	//
	uint8 transaction;//transaction status.TRANSACTION_TIP-tip,TRANSACTION_DAG-dag
	uint8 flag;//dag:0-nouse,1-use.tip:0-pass,1-fail
	uint16 reserved;
	//
	transaction_t *trunk;//trunk transaction
	transaction_t *branch;//branch transaction
	transaction_t *next;//tip link use
};
struct key_t
{
	uint8 e[KEY_E];//key_e
	uint8 n[KEY_LEN];//modular
};
struct list_t
{
	uint32 dag_index;//dag index(default 0,start from 1)
	uint32 device_index;//device index
	key_t key;//key_e/n
	uint32 token;//account token
	uint8 node;//NODE_HEAVY-heavy node,NODE_LIGHT-light node
};
struct route_t
{
	uint8 flag;//0-not connect,1-connect
	uint32 device_index;//device index(single physical address)
	//uint32 hops;//hop interval
	//uint32 *path;//route path
	key_t key;//key_e/n
	uint32 token[2];//account token(0-available,1-frozen)
	uint8 node;//NODE_HEAVY-heavy node,NODE_LIGHT-light node
	route_t *next;
};
struct queue_t
{
	volatile uint8 step;
	uint8 *data;
	queue_t *next;
};
struct mainchain_t
{
	queue_t *queue;//message queue
	uint32 list_number;//node number in list
	uint32 dag_number;//dag number
	list_t *list;//node list
	transaction_t *dag;//dag link use(full ledger)
};
struct device_t
{
	uint32 x;
	uint32 y;
	uint8 node;//NODE_HEAVY-heavy node,NODE_LIGHT-light node
	uint32 device_index;//device index(single physical address)
	route_t *route;//device route link
	queue_t *queue;//message queue
	rsa_t rsa;//rsa pair(current device)
	uint32 token[2];//account token(0-available,1-frozen)
	//transaction_t *dag;//dag link use(heavy node-dag ledger,light node-none ledger)
};
//function
void route_insert(device_t *device,route_t *route);
void route_delete(device_t *device);
uint8 route_node(device_t *device,uint32 device_index);
route_t *route_find(device_t *device,uint32 device_index);
void queue_insert(device_t *device,queue_t *queue);
void queue_insert(mainchain_t *mainchain,queue_t *queue);
void queue_delete(device_t *device);
void list_delete(mainchain_t *mainchain);
void transaction_insert(mainchain_t *mainchain,transaction_t *transaction);
void dag_tipclear(transaction_t *dag);
void dag_dagclear(transaction_t *transaction);
void dag_clear(transaction_t *dag);
uint32 dag_tipnum(transaction_t *dag);
void dag_dagnum(transaction_t *transaction,uint32 &number);
uint32 dag_num(transaction_t *dag);
void dag_tippoint(transaction_t *dag,uint32 &number,transaction_t **point);
void dag_dagpoint(transaction_t *transaction,uint32 &number,transaction_t **point);
uint32 dag_point(transaction_t *dag,transaction_t **point);
void dag_delete(mainchain_t *mainchain,transaction_t *transaction);
void dag_delete(mainchain_t *mainchain);
void dag_tipcopy(transaction_t *dst,transaction_t *src,transaction_t **transaction,uint32 &number);
void dag_dagcopy(transaction_t *dst,transaction_t *src,transaction_t **transaction,uint32 &number);
uint32 dag_copy(transaction_t *dst,transaction_t *src,transaction_t **transaction);
void dag_copy(transaction_t *dst,transaction_t *src,uint32 number,uint16 &level);
uint32 dag_tip(void);
uint32 dag_height(transaction_t *dag,transaction_t *transaction);
uint32 dag_depth(transaction_t *dag,transaction_t *transaction);
void device_delete(device_t *device);
void mainchain_delete(mainchain_t *mainchain);
void key_generate(device_t *device);
void move_location(device_t *device,uint32 step,uint32 range);