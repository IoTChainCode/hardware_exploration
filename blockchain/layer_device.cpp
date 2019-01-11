#include "layer_device.h"

extern uint32 g_devicenum[2];//device number.0-heavy device,1-light device
extern uint32 g_devicerange;//device range
extern uint32 g_devicestep;//device step
extern uint32 g_number;//deal number
extern deal_t *g_deal;//deal list
extern device_t *g_device;//device array
extern mainchain_t g_mainchain;//mainchain
extern volatile uint32 g_index;//transaction index(temporary use, start from 1, instead by hash_t later)
extern volatile uint8 g_flag;//for timer resend

//STEP_CONNECT
void connect_recv(device_t *device)
{
	//queue->route
	uint32 i;
	uint8 flag;
	route_t *route;
	queue_t *queue,*prev;
	index_t index;

	queue=device->queue;
	while(queue)
	{
		if (queue->step==STEP_CONNECT)
		{
			//queue process
			index.number=*(uint32 *)queue->data;
			index.device_index=(uint32 *)(queue->data+1*sizeof(uint32));
			index.key=queue->data+(1+index.number)*sizeof(uint32);
			index.token=(uint32 *)(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN));
			index.node=queue->data+(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN);
			for (i=0;i<index.number;i++)
			{
				if (index.device_index[i]==device->device_index)
					continue;
				flag=0;
				route=device->route;
				while(route)
				{
					if (route->device_index==index.device_index[i])
					{
						flag=1;
						break;
					}
					route=route->next;
				}
				if (!flag)
				{
					route=new route_t;
					route->flag=0;
					route->device_index=index.device_index[i];
					memcpy(route->key.e,&index.key[i*(KEY_E+KEY_LEN)],KEY_E);
					memcpy(route->key.n,&index.key[i*(KEY_E+KEY_LEN)+KEY_E],KEY_LEN);
					memcpy(route->token,&index.token[2*i],2*sizeof(uint32));
					route->node=index.node[i];
					route->next=NULL;
					route_insert(device,route);
					//printf("connect(%ld):%ld\r\n",device->device_index,route->device_index);
				}
			}
			//queue delete
			if (queue==device->queue)
			{
				device->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=device->queue;
			}
			else
			{
				prev->next=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=prev->next;
			}
		}
		else
		{
			prev=queue;
			queue=queue->next;
		}
	}
}

void connect_seek(device_t *device)
{
	//broadcasting to seek/malloc device->route
	uint32 i;
	uint8 flag;
	route_t *route;

	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		if (i==device->device_index)
			continue;
		if (math_distance(device->x,device->y,g_device[i].x,g_device[i].y)<=MAX_METRIC)
		{
			flag=0;
			route=device->route;
			while(route)
			{
				if (route->device_index==g_device[i].device_index)
				{
					flag=1;
					break;
				}
				route=route->next;
			}
			if (!flag)
			{
				route=new route_t;
				route->flag=0;
				route->device_index=g_device[i].device_index;
				memcpy(route->key.e,g_device[i].rsa.e,KEY_E);
				memcpy(route->key.n,g_device[i].rsa.n,KEY_LEN);
				memcpy(route->token,g_device[i].token,2*sizeof(uint32));
				route->node=g_device[i].node;
				route->next=NULL;
				route_insert(device,route);
				//printf("connect(%ld):%ld\r\n",device->device_index,route->device_index);
			}
		}
	}
}

void connect_send(device_t *device)
{
	//route->queue(device/mainchain)
	uint8 flag;
	route_t *route;
	queue_t *queue;
	index_t index;

	//compute number(should handling)
	flag=0;
	index.number=1;
	route=device->route;
	while(route)
	{
		if (!route->flag)
		{
			route->flag=1;
			flag=1;
		}
		index.number++;
		route=route->next;
	}
	if (!flag)
		return;
	index.device_index=new uint32[index.number];
	index.key=new uint8[index.number*(KEY_E+KEY_LEN)];
	index.token=new uint32[2*index.number];
	index.node=new uint8[index.number];
	index.number=0;
	index.device_index[index.number]=device->device_index;
	memcpy(&index.key[index.number*(KEY_E+KEY_LEN)],device->rsa.e,KEY_E);
	memcpy(&index.key[index.number*(KEY_E+KEY_LEN)+KEY_E],device->rsa.n,KEY_LEN);
	memcpy(&index.token[2*index.number],device->token,2*sizeof(uint32));
	index.node[index.number]=device->node;
	index.number++;
	route=device->route;
	while(route)
	{
		index.device_index[index.number]=route->device_index;
		memcpy(&index.key[index.number*(KEY_E+KEY_LEN)],route->key.e,KEY_E);
		memcpy(&index.key[index.number*(KEY_E+KEY_LEN)+KEY_E],route->key.n,KEY_LEN);
		memcpy(&index.token[2*index.number],route->token,2*sizeof(uint32));
		index.node[index.number]=route->node;
		index.number++;
		route=route->next;
	}
	//fill route
	route=device->route;
	while(route)
	{
		queue=new queue_t;
		queue->step=STEP_CONNECT;
		queue->data=new uint8[(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN)+index.number*sizeof(uint8)];
		*(uint32 *)queue->data=index.number;//align problem?
		memcpy(queue->data+1*sizeof(uint32),index.device_index,index.number*sizeof(uint32));
		memcpy(queue->data+(1+index.number)*sizeof(uint32),index.key,index.number*(KEY_E+KEY_LEN));
		memcpy(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.token,2*index.number*sizeof(uint32));
		memcpy(queue->data+(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.node,index.number*sizeof(uint8));
		queue_insert(&g_device[route->device_index],queue);
		route=route->next;
	}
	//fill mainchain
	if (device->node==NODE_HEAVY)//if heavy node then send to server
	{
		queue=new queue_t;
		queue->step=STEP_CONNECT;
		queue->data=new uint8[(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN)+index.number*sizeof(uint8)];
		*(uint32 *)queue->data=index.number;//align problem?
		memcpy(queue->data+1*sizeof(uint32),index.device_index,index.number*sizeof(uint32));
		memcpy(queue->data+(1+index.number)*sizeof(uint32),index.key,index.number*(KEY_E+KEY_LEN));
		memcpy(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.token,2*index.number*sizeof(uint32));
		memcpy(queue->data+(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.node,index.number*sizeof(uint8));
		queue_insert(&g_mainchain,queue);
		//printf("connect(%ld):mainchain\r\n",device->device_index);
	}
	//release
	delete[] index.device_index;
	delete[] index.key;
	//update queue
	queue=new queue_t;
	queue->step=STEP_TRANSACTION;
	queue->data=NULL;
	queue_insert(device,queue);
}

void connect_resend(device_t *device)
{
	//route->queue(device/mainchain)
	route_t *route;
	queue_t *queue;
	index_t index;

	if (device->node==NODE_LIGHT)
		return;
	//compute number(should handling)
	index.number=1;
	route=device->route;
	while(route)
	{
		index.number++;
		route=route->next;
	}
	index.device_index=new uint32[index.number];
	index.key=new uint8[index.number*(KEY_E+KEY_LEN)];
	index.token=new uint32[2*index.number];
	index.node=new uint8[index.number];
	index.number=0;
	index.device_index[index.number]=device->device_index;
	memcpy(&index.key[index.number*(KEY_E+KEY_LEN)],device->rsa.e,KEY_E);
	memcpy(&index.key[index.number*(KEY_E+KEY_LEN)+KEY_E],device->rsa.n,KEY_LEN);
	memcpy(&index.token[2*index.number],device->token,2*sizeof(uint32));
	index.node[index.number]=device->node;
	index.number++;
	route=device->route;
	while(route)
	{
		index.device_index[index.number]=route->device_index;
		memcpy(&index.key[index.number*(KEY_E+KEY_LEN)],route->key.e,KEY_E);
		memcpy(&index.key[index.number*(KEY_E+KEY_LEN)+KEY_E],route->key.n,KEY_LEN);
		memcpy(&index.token[2*index.number],route->token,2*sizeof(uint32));
		index.node[index.number]=route->node;
		index.number++;
		route=route->next;
	}
	//fill mainchain
	queue=new queue_t;
	queue->step=STEP_CONNECT;
	queue->data=new uint8[(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN)+index.number*sizeof(uint8)];
	*(uint32 *)queue->data=index.number;//align problem?
	memcpy(queue->data+1*sizeof(uint32),index.device_index,index.number*sizeof(uint32));
	memcpy(queue->data+(1+index.number)*sizeof(uint32),index.key,index.number*(KEY_E+KEY_LEN));
	memcpy(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.token,2*index.number*sizeof(uint32));
	memcpy(queue->data+(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.node,index.number*sizeof(uint8));
	queue_insert(&g_mainchain,queue);
	//printf("connect(%ld):mainchain\r\n",device->device_index);
	//release
	delete[] index.device_index;
	delete[] index.key;
	//update queue
	queue=new queue_t;
	queue->step=STEP_TRANSACTION;
	queue->data=NULL;
	queue_insert(device,queue);
}

//STEP_TRANSACTION
uint8 transaction_verify(device_t *device,transaction_t *transaction)
{
	//transaction verify:verify device address by rsa, verify device ledger by value compare.0-pass,1-address fail,2-ledger fail
	uint32 i,j;
	uint8 flag;
	rsa_t rsa;
	route_t *route;
	uint8 *e,*n;
	uint8 result[KEY_LEN];

	//get device
	rsa.le=KEY_E;
	rsa.len=KEY_LEN;
	if (device->device_index==transaction->deal.device_index[0])
	{
		e=device->rsa.e;
		n=device->rsa.n;
		j=device->token[0];
		flag=0;
	}
	else
	{
		route=device->route;
		while(route)
		{
			if (route->device_index==transaction->deal.device_index[0])
				break;
			route=route->next;
		}
		e=route->key.e;
		n=route->key.n;
		j=route->token[0];
		flag=1;
	}
	//address verify
	rsa.e=new uint8[rsa.le];
	rsa.n=new uint8[rsa.len];
	memcpy(rsa.e,e,KEY_E);
	memcpy(rsa.n,n,KEY_LEN);
	i=rsa_enc(result,transaction->cipher,rsa.len,&rsa);
	memset(&result[i],0,rsa.len-i);
	if (memcmp(result,transaction->plain,rsa.len))
		return STATUS_DEVICE;
	//ledger verify
	if (flag && transaction->deal.token>j)//verify if source=light node
		return STATUS_LEDGER;

	return STATUS_DONE;
}

void transaction_recv(device_t *device)
{
	//queue->delete queue
	uint8 flag,node;
	route_t *route;
	queue_t *queue,*prev,*insert;
	transaction_t transaction;

	queue=device->queue;
	while(queue)
	{
		if (queue->step==STEP_TRANSACTION)
		{
			//queue process
			if (queue->data && device->node==NODE_HEAVY)//heavy node & transaction
			{
				//transaction verify
				transaction.index=*(uint32 *)queue->data;
				memcpy(&transaction.deal,queue->data+1*sizeof(uint32),sizeof(deal_t));
				memcpy(transaction.plain,queue->data+1*sizeof(uint32)+sizeof(deal_t),KEY_LEN);
				memcpy(transaction.cipher,queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,KEY_LEN);
				flag=transaction_verify(device,&transaction);
				node=route_node(device,transaction.deal.device_index[0]);
				if (flag)//error
				{
					if (node==NODE_LIGHT)//if source=light node,send back to source(update ledger)
					{
						insert=new queue_t;
						insert->step=STEP_LEDGER;
						insert->data=new uint8[sizeof(ledger_t)];
						*(uint32 *)insert->data=flag;
						*(uint32 *)(insert->data+1*sizeof(uint32))=transaction.index;
						*(uint32 *)(insert->data+2*sizeof(uint32))=transaction.deal.device_index[0];
						*(uint32 *)(insert->data+3*sizeof(uint32))=transaction.deal.token;
						queue_insert(&g_device[transaction.deal.device_index[0]],insert);
					}
					else//if source=heavy node(self),update ledger
					{
						device->token[0]+=transaction.deal.token;
						device->token[1]-=transaction.deal.token;
						printf("device(%ld):%ld-%ld\r\n",device->device_index,device->token[0],device->token[1]);
					}
				}
				else//correct then send to server
				{
					insert=new queue_t;
					insert->step=STEP_TRANSACTION;
					insert->data=new uint8[sizeof(spv_t)];
					*(uint32 *)insert->data=*(uint32 *)queue->data;
					memcpy(insert->data+1*sizeof(uint32),queue->data+1*sizeof(uint32),sizeof(deal_t));
					memcpy(insert->data+1*sizeof(uint32)+sizeof(deal_t),queue->data+1*sizeof(uint32)+sizeof(deal_t),KEY_LEN);
					memcpy(insert->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,KEY_LEN);
					queue_insert(&g_mainchain,insert);
					//update token
					if (node==NODE_LIGHT)//if source=light node,update route ledger
					{
						route=route_find(device,transaction.deal.device_index[0]);
						route->token[0]-=transaction.deal.token;
						route->token[1]+=transaction.deal.token;
						//printf("device(%ld):%ld-%ld\r\n",device->device_index,device->token[0],device->token[1]);
					}
				}
			}
			//queue delete
			if (queue==device->queue)
			{
				device->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=device->queue;
			}
			else
			{
				prev->next=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=prev->next;
			}
		}
		else
		{
			prev=queue;
			queue=queue->next;
		}
	}
}

void transaction_signature(transaction_t *transaction,device_t *device)
{
	//encrypt transaction with rsa private key
	uint32 i;

	i=rsa_dec(transaction->cipher,transaction->plain,device->rsa.len,&device->rsa,RSA_CRT);
	memset(&transaction->cipher[i],0,device->rsa.len-i);
}

transaction_t *transaction_generate(device_t *device)
{
	//generate transaction
	uint32 i;
	transaction_t *transaction;

	if (g_index==g_number)//all transaction done
		return NULL;
	if (device->device_index!=g_deal[g_index].device_index[0])//not current deal index
		return NULL;
	if (g_deal[g_index].token>device->token[0])//ledger verify
	{
		printf("transaction(%ld-%ld):%ld,error\r\n",g_deal[g_index].device_index[0],g_deal[g_index].device_index[1],g_deal[g_index].token);
		g_index++;
		return NULL;
	}
	transaction=new transaction_t;
	memcpy(&transaction->deal,&g_deal[g_index],sizeof(transaction_t));
	_rand(transaction->plain,KEY_LEN);
	i=_mod(transaction->plain,transaction->plain,device->rsa.n,KEY_LEN,KEY_LEN);
	memset(&transaction->plain[i],0,KEY_LEN-i);
	transaction_signature(transaction,device);
	transaction->index=++g_index;
	printf("transaction(%ld-%ld):%ld\r\n",transaction->deal.device_index[0],transaction->deal.device_index[1],transaction->deal.token);

	return transaction;
}

void transaction_send(device_t *device,transaction_t *transaction)
{
	//transaction->queue
	route_t *route;
	queue_t *queue;

	//update queue(self)
	if (!transaction)
	{
		queue=new queue_t;
		queue->step=STEP_TRANSACTION;
		queue->data=NULL;
		queue_insert(device,queue);
		return;
	}
	if (device->node==NODE_LIGHT)
	{
		//update queue
		queue=new queue_t;
		queue->step=STEP_TRANSACTION;
		queue->data=NULL;
		queue_insert(device,queue);
		//update ledger
		route=device->route;
		while(route)
		{
			if (route->node==NODE_HEAVY)
				break;
			route=route->next;
		}
		if (!route)//if not connect heavy node,update self ledger
		{
			device->token[0]-=transaction->deal.token;
			device->token[1]+=transaction->deal.token;
			printf("device(%ld):%ld-%ld\r\n",device->device_index,device->token[0],device->token[1]);
			return;
		}
	}
	//update queue(other)
	queue=new queue_t;
	queue->step=STEP_TRANSACTION;
	queue->data=new uint8[sizeof(spv_t)];
	*(uint32 *)queue->data=transaction->index;
	memcpy(queue->data+1*sizeof(uint32),&transaction->deal,sizeof(deal_t));
	memcpy(queue->data+1*sizeof(uint32)+sizeof(deal_t),transaction->plain,KEY_LEN);
	memcpy(queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,transaction->cipher,KEY_LEN);
	if (device->node==NODE_LIGHT)//if light node,send transaction to 1st heavy node
		queue_insert(&g_device[route->device_index],queue);
	else//if heavy node,send transaction to self
		queue_insert(device,queue);
	//update ledger
	device->token[0]-=transaction->deal.token;
	device->token[1]+=transaction->deal.token;
	printf("device(%ld):%ld-%ld\r\n",device->device_index,device->token[0],device->token[1]);
}

//STEP_LEDGER
void ledger_recv(device_t *device)
{
	//queue->delete queue
	uint8 node;
	queue_t *queue,*prev,*insert;

	queue=device->queue;
	while(queue)
	{
		if (queue->step==STEP_LEDGER)
		{
			//queue process
			if (device->node==NODE_HEAVY)//if heavy node
			{
				node=route_node(device,*(uint32 *)(queue->data+2*sizeof(uint32)));
				if (node==NODE_LIGHT)//if light node,send back to light node(update ledger)
				{
					insert=new queue_t;
					insert->step=STEP_LEDGER;
					insert->data=new uint8[sizeof(ledger_t)];
					memcpy(insert->data,queue->data,sizeof(ledger_t));
					queue_insert(&g_device[*(uint32 *)(queue->data+2*sizeof(uint32))],insert);
				}
				else//if heavy node,update ledger
				{
					switch(*(uint32 *)queue->data)
					{
					case STATUS_SRC:
						device->token[1]-=*(uint32 *)(queue->data+3*sizeof(uint32));
						break;
					case STATUS_DST:
						device->token[0]+=*(uint32 *)(queue->data+3*sizeof(uint32));
						break;
					case STATUS_DEVICE:
					case STATUS_LEDGER:
						device->token[0]+=*(uint32 *)(queue->data+3*sizeof(uint32));
						device->token[1]-=*(uint32 *)(queue->data+3*sizeof(uint32));
						break;
					}
					printf("device(%ld):%ld-%ld\r\n",device->device_index,device->token[0],device->token[1]);
				}
			}
			else//if light node,update ledger
			{
				switch(*(uint32 *)queue->data)
				{
				case STATUS_SRC:
					device->token[1]-=*(uint32 *)(queue->data+3*sizeof(uint32));
					break;
				case STATUS_DST:
					device->token[0]+=*(uint32 *)(queue->data+3*sizeof(uint32));
					break;
				case STATUS_DEVICE:
				case STATUS_LEDGER:
					device->token[0]+=*(uint32 *)(queue->data+3*sizeof(uint32));
					device->token[1]-=*(uint32 *)(queue->data+3*sizeof(uint32));
					break;
				}
				printf("device(%ld):%ld-%ld\r\n",device->device_index,device->token[0],device->token[1]);
			}
			//queue delete
			if (queue==device->queue)
			{
				device->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=device->queue;
			}
			else
			{
				prev->next=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=prev->next;
			}
		}
		else
		{
			prev=queue;
			queue=queue->next;
		}
	}
}

void ledger_send(device_t *device)
{
	//ledger->queue
	queue_t *queue;

	queue=new queue_t;
	queue->step=STEP_CONNECT;
	queue->data=new uint8[1*sizeof(uint32)];
	*(uint32 *)queue->data=0;//align problem?
	//queue->step=STEP_MOVE;//STEP_TRANSACTION
	//queue->data=NULL;
	queue_insert(device,queue);
}

void process_device(device_t *device)
{
	transaction_t *transaction;
	static __declspec(thread) uint8 flag=0;

	if (!device->queue)
		return;
	if (flag!=g_flag)
	{
		flag=g_flag;
		connect_resend(device);//resend connect device->mainchain
	}
	switch(device->queue->step)
	{
	case STEP_CONNECT:
		//recv
		connect_recv(device);//recv & process device's queue->route
		//process
		connect_seek(device);//search around nearby->route
		//send
		connect_send(device);//pack & send device's route->queue
		break;
	case STEP_TRANSACTION:
		//recv
		transaction_recv(device);//recv & process device's queue
		//process
		transaction=transaction_generate(device);//generate transaction
		//send
		transaction_send(device,transaction);//pack & send device's route->queue
		break;
	case STEP_LEDGER:
		//recv
		ledger_recv(device);//recv & process device's queue
		//process
		//send
		ledger_send(device);//pack & send device's route->queue
		break;
	//case STEP_MOVE:
	//	break;
	}
}