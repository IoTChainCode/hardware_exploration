#include "stdafx.h"
#include "layer_mainchain.h"

extern uint32 g_devicenum[2];//device number.0-heavy device,1-light device
extern uint32 g_devicerange;//device range
extern uint32 g_devicestep;//device step
extern uint32 g_number;//deal number
extern deal_t *g_deal;//deal list
extern device_t *g_device;//device array
extern mainchain_t g_mainchain;//mainchain
extern volatile uint32 g_index;//transaction index(temporary use, start from 1, instead by hash_t later)
//
extern uint8 **g_check;//[g_number][5].0-source,1-mc(src),2-mc(dst),3-node(src),4-node(dst)

//STEP_CONNECT
void connect_recv(mainchain_t *mainchain)
{
	//queue->list
	uint32 i,j;
	uint32 *dag;
	uint8 flag;
	queue_t *queue,*remove,*prev;
	index_t index;

	flag=0;
	while(1)
	{
		queue=mainchain->queue;
		while(queue)
		{
			if (queue->step==STEP_CONNECT)
				break;
			queue=queue->next;
		}
		if (!queue)
			break;
		flag=1;
		//find latest/first queue
		index.number=*(uint32 *)queue->data;
		index.device_index=(uint32 *)(queue->data+1*sizeof(uint32));
		index.key=queue->data+(1+index.number)*sizeof(uint32);
		index.token=(uint32 *)(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN));
		index.node=queue->data+(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN);
		//update list
		for (i=0;i<index.number;i++)
		{
			for (j=0;j<mainchain->list_number;j++)
				if (index.device_index[i]==mainchain->list[j].device_index)//must find it
					break;
			mainchain->list[j].dag_index=mainchain->dag_number+1;
			memcpy(&mainchain->list[j].key,&index.key[i*(KEY_E+KEY_LEN)],KEY_E+KEY_LEN);
			mainchain->list[j].node=index.node[i];
			//printf("connect(mainchain):%ld\r\n",index.device_index[i]);
		}
		//compute dag_number
		mainchain->dag_number=0;
		for (i=0;i<mainchain->list_number;i++)
		{
			if (!mainchain->list[i].dag_index)
				continue;
			for (j=0;j<mainchain->dag_number;j++)
				if (mainchain->list[i].dag_index==dag[j])
					break;
			if (j==mainchain->dag_number)
			{
				dag=mainchain->dag_number ? (uint32 *)realloc(dag,(mainchain->dag_number+1)*sizeof(uint32)) : (uint32 *)malloc((mainchain->dag_number+1)*sizeof(uint32));
				dag[mainchain->dag_number++]=mainchain->list[i].dag_index;
			}
		}
		//modify dag_index
		for (i=0;i<mainchain->list_number;i++)
		{
			if (!mainchain->list[i].dag_index)
				continue;
			for (j=0;j<mainchain->dag_number;j++)
				if (mainchain->list[i].dag_index==dag[j])
					break;
			mainchain->list[i].dag_index=j+1;
		}
		if (mainchain->dag_number)
			free(dag);
		//delete all queues with same device_index
		remove=queue;
		while(remove)
		{
			if (remove->step==STEP_CONNECT && *(uint32 *)(remove->data+1*sizeof(uint32))==index.device_index[0])
			{
				if (remove==mainchain->queue)
				{
					mainchain->queue=remove->next;
					if (remove->data)
					{
						delete[] remove->data;
						remove->data=NULL;
					}
					delete remove;
					remove=mainchain->queue;
				}
				else
				{
					prev->next=remove->next;
					if (remove->data)
					{
						delete[] remove->data;
						remove->data=NULL;
					}
					delete remove;
					remove=prev->next;
				}
			}
			else
			{
				prev=remove;
				remove=remove->next;
			}
		}
	}
	//update queue
	if (flag)
	{
		queue=new queue_t;
		queue->step=STEP_TRANSACTION;
		queue->data=NULL;
		queue_insert(mainchain,queue);
	}
}

//STEP_TRANSACTION
uint8 transaction_verify(mainchain_t *mainchain,transaction_t *transaction)
{
	//transaction verify:verify device address by rsa, verify device ledger by value compare.0-pass,1-address fail,2-ledger fail
	uint32 i;
	rsa_t rsa;
	uint8 result[KEY_LEN];

	//get device
	rsa.le=KEY_E;
	rsa.len=KEY_LEN;
	for (i=0;i<mainchain->list_number;i++)
	{
		if (mainchain->list[i].device_index==transaction->deal.device_index[0])
			break;
	}
	//address verify
	rsa.e=new uint8[rsa.le];
	rsa.n=new uint8[rsa.len];
	memcpy(rsa.e,mainchain->list[i].key.e,KEY_E);
	memcpy(rsa.n,mainchain->list[i].key.n,KEY_LEN);
	i=rsa_enc(result,transaction->cipher,rsa.len,&rsa);
	memset(&result[i],0,rsa.len-i);
	if (memcmp(result,transaction->plain,rsa.len))
		return STATUS_DEVICE;
	//ledger verify
	if (transaction->deal.token>mainchain->list[i].token)
		return STATUS_LEDGER;

	return STATUS_DONE;
}

uint8 transaction_seek(transaction_t **trunk,transaction_t **branch,mainchain_t *mainchain)
{
	//search 2 tips(random algorithm):0-dag empty,1-dag not empty.Actually we should create wider tips manual, I use dag_tip to control automaticlly.
	uint32 i,j,k;
	transaction_t *transaction;

	if (!mainchain->dag)//no genesis
		return 1;
	i=dag_tipnum(mainchain->dag);
	if (!i)//no correct tip or no genesis
		return 1;
	if (i==1)
	{
		j=0;
		k=0;
	}
	else
	{
		//find tip's index
		while(1)
		{
			j=rand()%i;
			k=rand()%i;
			if (j!=k)
				break;
		}
	}
	i=0;
	transaction=mainchain->dag;
	while(transaction)
	{
		if (!transaction->flag)//correct tip
		{
			if (j==i)
				*trunk=transaction;
			if (k==i)
				*branch=transaction;
			i++;
		}
		transaction=transaction->next;
	}

	return 0;
}

uint32 transaction_pow(transaction_t *transaction)
{
	//compute hash pow by sha256
	uint64 i;
	uint32 length;
	crypt_sha256 *sha256;
	uint8 content[KEY_LEN+1],result[HASH_LEN];

	length=KEY_LEN;
	memcpy(content,transaction->plain,length);
	sha256=new crypt_sha256;
	for (i=1;i<0x100000000;i++)
	{
		length=_add(content,content,1,length);
		sha256->sha256_init();
		sha256->sha256_update(content,length);
		sha256->sha256_final(result);
		if (_bitlen(result,HASH_LEN)<=HASH_LEN*8-COMPARE_LEN)
			break;
	}
	if (i==0x100000000)
		i=0;
	delete sha256;

	return (uint32)i;
}

uint8 transaction_device(uint32 &index,mainchain_t *mainchain,uint32 device_index)
{
	uint32 i,j;

	for (i=0;i<mainchain->list_number;i++)
		if (device_index==mainchain->list[i].device_index)//must find it
			break;
	if (mainchain->list[i].node==NODE_HEAVY)//use if self=heavy node
	{
		index=mainchain->list[i].device_index;
		return 0;
	}
	for (j=0;j<mainchain->list_number;j++)
		if (mainchain->list[i].dag_index==mainchain->list[j].dag_index && mainchain->list[j].node==NODE_HEAVY)//find heavy node in same dag
			break;
	if (j==mainchain->list_number)//not find heavy node
		return 1;
	index=mainchain->list[j].device_index;

	return 0;
}

void transaction_recv(mainchain_t *mainchain)
{
	//queue->dag
	uint32 i,j;
	uint32 pow[2];
	uint8 flag;
	transaction_t *trunk,*branch,*transaction,*point;
	queue_t *queue,*insert,*prev;

	queue=mainchain->queue;
	while(queue)
	{
		if (queue->step==STEP_TRANSACTION)
		{
			if (queue->data)
			{
				//search error tip
				transaction=mainchain->dag;
				while(transaction)
				{
					if (transaction->flag)
					{
						transaction->index=*(uint32 *)queue->data;
						memcpy(&transaction->deal,(uint32 *)(queue->data+1*sizeof(uint32)),sizeof(deal_t));
						memcpy(transaction->plain,(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)),KEY_LEN);
						memcpy(transaction->cipher,(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN),KEY_LEN);
						transaction->transaction=TRANSACTION_TIP;
						transaction->flag=0;
						goto next;
					}
					transaction=transaction->next;
				}
				//find 2 tips & verify them(address/ledger), calculate pow
				do
				{
					trunk=branch=NULL;
					flag=transaction_seek(&trunk,&branch,mainchain);
					if (!flag)//dag exist
					{
						i=transaction_device(j,mainchain,trunk->deal.device_index[0]);
						if (i)//not find heavy node
							goto loop;
						flag=transaction_verify(mainchain,trunk);
						if (flag)
						{
							trunk->flag=1;
							//notify device
							insert=new queue_t;
							insert->step=STEP_LEDGER;
							insert->data=new uint8[sizeof(ledger_t)];
							*(uint32 *)insert->data=flag;
							*(uint32 *)(insert->data+1*sizeof(uint32))=trunk->index;
							*(uint32 *)(insert->data+2*sizeof(uint32))=trunk->deal.device_index[0];
							*(uint32 *)(insert->data+3*sizeof(uint32))=trunk->deal.token;
							queue_insert(&g_device[j],insert);
							//printf("mainchain:%ld\r\n",trunk->deal.device_index[0]);
							delay();
							break;
						}
						i=transaction_device(j,mainchain,branch->deal.device_index[0]);
						if (i)//not find heavy node
							goto loop;
						flag=transaction_verify(mainchain,branch);
						if (flag)
						{
							branch->flag=1;
							//notify device
							insert=new queue_t;
							insert->step=STEP_LEDGER;
							insert->data=new uint8[sizeof(ledger_t)];
							*(uint32 *)insert->data=flag;
							*(uint32 *)(insert->data+1*sizeof(uint32))=branch->index;
							*(uint32 *)(insert->data+2*sizeof(uint32))=branch->deal.device_index[0];
							*(uint32 *)(insert->data+3*sizeof(uint32))=branch->deal.token;
							queue_insert(&g_device[j],insert);
							//printf("mainchain:%ld\r\n",branch->deal.device_index[0]);
							delay();
							break;
						}
						pow[0]=transaction_pow(trunk);
						pow[1]=transaction_pow(branch);
					}
					else//no genesis
					{
						flag=0;
						pow[0]=pow[1]=0;
					}
				}while(0);
				if (!flag)//correct
				{
					//add in tip
					transaction=new transaction_t;
					transaction->index=*(uint32 *)queue->data;
					memcpy(&transaction->deal,(uint32 *)(queue->data+1*sizeof(uint32)),sizeof(deal_t));
					memcpy(transaction->plain,(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)),KEY_LEN);
					memcpy(transaction->cipher,(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN),KEY_LEN);
					memcpy(transaction->pow,pow,2*sizeof(uint32));
					transaction->transaction=TRANSACTION_TIP;
					transaction->flag=0;
					transaction->trunk=trunk;
					transaction->branch=branch;
					transaction_insert(mainchain,transaction);
					//modify trunk/branch with dag
					if (trunk && trunk->transaction!=TRANSACTION_DAG)
					{
						trunk->transaction=TRANSACTION_DAG;
						//update ledger
						for (i=0;i<mainchain->list_number;i++)
						{
							if (trunk->deal.device_index[0]==mainchain->list[i].device_index)
							{
								mainchain->list[i].token-=trunk->deal.token;
								//printf("mainchain(%ld-%ld):%ld\r\n",mainchain->list[i].dag_index,mainchain->list[i].device_index,mainchain->list[i].token);
							}
							if (trunk->deal.device_index[1]==mainchain->list[i].device_index)
							{
								mainchain->list[i].token+=trunk->deal.token;
								//printf("mainchain(%ld-%ld):%ld\r\n",mainchain->list[i].dag_index,mainchain->list[i].device_index,mainchain->list[i].token);
							}
						}
						//notify device
						for (i=0;i<2;i++)
						{
							insert=new queue_t;
							insert->step=STEP_LEDGER;
							insert->data=new uint8[sizeof(ledger_t)];
							*(uint32 *)insert->data=i ? STATUS_DST : STATUS_SRC;
							*(uint32 *)(insert->data+1*sizeof(uint32))=trunk->index;
							*(uint32 *)(insert->data+2*sizeof(uint32))=trunk->deal.device_index[i];
							*(uint32 *)(insert->data+3*sizeof(uint32))=trunk->deal.token;
							flag=transaction_device(j,mainchain,trunk->deal.device_index[i]);
							if (flag)//not find heavy node
								queue_insert(mainchain,insert);
							else
							{
								queue_insert(&g_device[j],insert);
								//printf("mainchain:%ld\r\n",j);
								g_check[trunk->index-1][1+i]=1;delay();
							}
						}
					}
					if (branch && branch->transaction!=TRANSACTION_DAG && trunk!=branch)
					{
						branch->transaction=TRANSACTION_DAG;
						//update ledger
						for (i=0;i<mainchain->list_number;i++)
						{
							if (branch->deal.device_index[0]==mainchain->list[i].device_index)
							{
								mainchain->list[i].token-=branch->deal.token;
								//printf("mainchain(%ld-%ld):%ld\r\n",mainchain->list[i].dag_index,mainchain->list[i].device_index,mainchain->list[i].token);
							}
							if (branch->deal.device_index[1]==mainchain->list[i].device_index)
							{
								mainchain->list[i].token+=branch->deal.token;
								//printf("mainchain(%ld-%ld):%ld\r\n",mainchain->list[i].dag_index,mainchain->list[i].device_index,mainchain->list[i].token);
							}
						}
						//notify device
						for (i=0;i<2;i++)
						{
							insert=new queue_t;
							insert->step=STEP_LEDGER;
							insert->data=new uint8[sizeof(ledger_t)];
							*(uint32 *)insert->data=i ? STATUS_DST : STATUS_SRC;
							*(uint32 *)(insert->data+1*sizeof(uint32))=branch->index;
							*(uint32 *)(insert->data+2*sizeof(uint32))=branch->deal.device_index[i];
							*(uint32 *)(insert->data+3*sizeof(uint32))=branch->deal.token;
							flag=transaction_device(j,mainchain,branch->deal.device_index[i]);
							if (flag)//not find heavy node
								queue_insert(mainchain,insert);
							else
							{
								queue_insert(&g_device[j],insert);
								//printf("mainchain:%ld\r\n",j);
								g_check[branch->index-1][1+i]=1;delay();
							}
						}
					}
					//add into dag if width enough
					if (dag_tipnum(mainchain->dag)==dag_tip())
					{
						transaction=mainchain->dag;
						while(transaction)
						{
							if (transaction->transaction==TRANSACTION_DAG)
							{
								if (transaction==mainchain->dag)
									mainchain->dag=mainchain->dag->next;
								else
									point->next=transaction->next;
							}
							else
								point=transaction;
							transaction=transaction->next;
						}
					}
				}
			}
next:
			//queue delete
			if (queue==mainchain->queue)
			{
				mainchain->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=mainchain->queue;
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
loop:
		{
			prev=queue;
			queue=queue->next;
		}
	}
}

void transaction_send(mainchain_t *mainchain)
{
	//transaction->queue
	queue_t *queue;

	queue=new queue_t;
	queue->step=STEP_TRANSACTION;
	queue->data=NULL;
	queue_insert(mainchain,queue);
}

//STEP_LEDGER
void ledger_recv(mainchain_t *mainchain)
{
	//queue->device
	uint32 i,j;
	queue_t *queue,*insert,*prev;

	queue=mainchain->queue;
	while(queue)
	{
		if (queue->step==STEP_LEDGER)
		{
			//queue process
			i=transaction_device(j,mainchain,*(uint32 *)(queue->data+1*sizeof(uint32)));
			if (i)//not find heavy node
			{
				prev=queue;
				queue=queue->next;
				continue;
			}
			//queue send
			insert=new queue_t;
			insert->step=STEP_LEDGER;
			insert->data=new uint8[sizeof(ledger_t)];
			memcpy(insert->data,queue->data,sizeof(ledger_t));
			queue_insert(&g_device[j],queue);
			//printf("mainchain:%ld\r\n",j);
			delay();
			//queue delete
			if (queue==mainchain->queue)
			{
				mainchain->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=mainchain->queue;
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

void process_mainchain(mainchain_t *mainchain)
{
	if (!mainchain->queue)
		return;
	switch(mainchain->queue->step)
	{
	case STEP_CONNECT:
		//recv
		connect_recv(mainchain);//recv queue & process mainchain's list
		break;
	case STEP_TRANSACTION:
		//recv
		transaction_recv(mainchain);//recv queue & process mainchain's
		//send
		transaction_send(mainchain);//pack & send mainchain's route->queue
		break;
	case STEP_LEDGER:
		//recv
		ledger_recv(mainchain);//recv queue & process mainchain's queue
		break;
	}
}