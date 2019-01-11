#include "stdafx.h"
#include "layer.h"

//route insert into device->route
void route_insert(device_t *device,route_t *route)
{
	route->next=device->route;
	device->route=route;
}

//route delete from device->route
void route_delete(device_t *device)
{
	route_t *route;

	route=device->route;
	while(route)
	{
		device->route=route->next;
		delete route;
		route=device->route;
	}
}

//route find node by device_index
uint8 route_node(device_t *device,uint32 device_index)
{
	route_t *route;

	if (device->device_index==device_index)
		return device->node;
	route=device->route;
	while(route)
	{
		if (route->device_index==device_index)
			break;
		route=route->next;
	}

	return route->node;
}

//route find by device_index
route_t *route_find(device_t *device,uint32 device_index)
{
	route_t *route;

	route=device->route;
	while(route)
	{
		if (route->device_index==device_index)
			break;
		route=route->next;
	}
	
	return route;
}

//queue insert into device->queue
void queue_insert(device_t *device,queue_t *queue)
{
	queue_t *next;

	if (queue->step==STEP_CONNECT)
	{
		queue->next=device->queue;
		device->queue=queue;
	}
	else
	{
		if (!device->queue)
		{
			queue->next=device->queue;
			device->queue=queue;
		}
		else
		{
			next=device->queue;
			while(next->next) next=next->next;
			queue->next=NULL;
			next->next=queue;
		}
	}
}

//queue insert into mainchain->queue
void queue_insert(mainchain_t *mainchain,queue_t *queue)
{
	queue_t *next;

	if (queue->step==STEP_CONNECT)
	{
		queue->next=mainchain->queue;
		mainchain->queue=queue;
	}
	else
	{
		if (!mainchain->queue)
		{
			queue->next=mainchain->queue;
			mainchain->queue=queue;
		}
		else
		{
			next=mainchain->queue;
			while(next->next) next=next->next;
			queue->next=NULL;
			next->next=queue;
		}
	}
}

//queue delete from device/mainchain->queue
void queue_delete(queue_t *queue)
{
	queue_t *point;

	point=queue;
	while(point)
	{
		queue=point->next;
		if (point->data)
		{
			delete[] point->data;
			point->data=NULL;
		}
		delete point;
		point=queue;
	}
}

//list delete from mainchain->list
void list_delete(mainchain_t *mainchain)
{
	if (mainchain->list)
		delete[] mainchain->list;
}

//transaction insert into mainchain->dag
void transaction_insert(mainchain_t *mainchain,transaction_t *transaction)
{
	transaction->next=mainchain->dag;
	mainchain->dag=transaction;
}

//dag clear:clear tip's flag
void dag_tipclear(transaction_t *dag)
{
	transaction_t *transaction;

	transaction=dag;
	while(transaction)
	{
		if (transaction->flag)//correct/uncount tip
			transaction->flag=0;
		transaction=transaction->next;
	}
}

//dag clear:clear dag's flag
void dag_dagclear(transaction_t *transaction)
{
	if (!transaction->flag)
		return;
	transaction->flag=0;
	if (transaction->trunk && transaction->branch)
	{
		dag_dagclear(transaction->trunk);
		dag_dagclear(transaction->branch);
	}
	else if (transaction->trunk)
		dag_dagclear(transaction->trunk);
	else if (transaction->branch)
		dag_dagclear(transaction->branch);
}

//dag clear:clear dag+tip's flag
void dag_clear(transaction_t *dag)
{
	dag_dagclear(dag);
	dag_tipclear(dag);
}

//dag number:dag's tip number(there's tip/dag in the next)
uint32 dag_tipnum(transaction_t *dag)
{
	uint32 number;
	transaction_t *transaction;

	number=0;
	transaction=dag;
	while(transaction)
	{
		if (!transaction->flag)//correct/uncount tip
			number++;
		transaction=transaction->next;
	}

	return number;
}

//dag number:dag's dag number(maybe including some tip)
void dag_dagnum(transaction_t *transaction,uint32 &number)
{
	if (transaction->flag)
		return;
	transaction->flag=1;
	number++;
	if (transaction->trunk && transaction->branch)
	{
		dag_dagnum(transaction->trunk,number);
		dag_dagnum(transaction->branch,number);
	}
	else if (transaction->trunk)
		dag_dagnum(transaction->trunk,number);
	else if (transaction->branch)
		dag_dagnum(transaction->branch,number);
}

//dag number:dag+tip
uint32 dag_num(transaction_t *dag)
{
	uint32 number;

	number=0;
	dag_dagnum(dag,number);
	number+=dag_tipnum(dag);

	return number;
}

//dag point:store tip's point
void dag_tippoint(transaction_t *dag,uint32 &number,transaction_t **point)
{
	transaction_t *transaction;

	transaction=dag;
	while(transaction)
	{
		if (!transaction->flag)//correct/uncount tip
			point[number++]=transaction;
		transaction=transaction->next;
	}
}

//dag point:store dag's point
void dag_dagpoint(transaction_t *transaction,uint32 &number,transaction_t **point)
{
	if (transaction->flag)
		return;
	transaction->flag=1;
	point[number++]=transaction;
	if (transaction->trunk && transaction->branch)
	{
		dag_dagpoint(transaction->trunk,number,point);
		dag_dagpoint(transaction->branch,number,point);
	}
	else if (transaction->trunk)
		dag_dagpoint(transaction->trunk,number,point);
	else if (transaction->branch)
		dag_dagpoint(transaction->branch,number,point);
}

//dag point:store tip+dag's point
uint32 dag_point(transaction_t *dag,transaction_t **point)
{
	uint32 number;

	number=0;
	dag_dagpoint(dag,number,point);
	dag_tippoint(dag,number,point);

	return number;
}

//dag delete certain tip
void dag_delete(mainchain_t *mainchain,transaction_t *transaction)
{
	transaction_t *prev,*point;

	point=mainchain->dag;
	while(point)
	{
		if (point==transaction)
		{
			if (point==mainchain->dag)
				mainchain->dag=mainchain->dag->next;
			else
				prev->next=point->next;
			delete point;
			break;
		}
		prev=point;
		point=point->next;
	}
}

//dag delete:delete whole dag+tip
void dag_delete(mainchain_t *mainchain)
{
	uint32 i,number;
	transaction_t **transaction;

	if (!mainchain->dag)
		return;
	number=dag_num(mainchain->dag);
	dag_clear(mainchain->dag);
	transaction=new transaction_t*[number];
	number=dag_point(mainchain->dag,transaction);
	dag_clear(mainchain->dag);
	for (i=0;i<number;i++)
		delete[] transaction[i];
	delete[] transaction;
}

//dag copy:copy dag's tip
void dag_tipcopy(transaction_t *dst,transaction_t *src,transaction_t **transaction,uint32 &number)
{
	transaction_t *point;

	point=src;
	while(point)
	{
		if (!point->flag)//correct/uncount tip
		{
			transaction[number]=point;
			memcpy(&dst[number],point,sizeof(transaction_t));
			point->flag=1;
			number++;
		}
		point=point->next;
	}
}

//dag copy:copy dag's dag
void dag_dagcopy(transaction_t *dst,transaction_t *src,transaction_t **transaction,uint32 &number)
{
	if (src->flag)
		return;
	src->flag=1;
	transaction[number]=src;
	memcpy(&dst[number],src,sizeof(transaction_t));
	number++;
	if (src->trunk && src->branch)
	{
		dag_dagcopy(dst,src->trunk,transaction,number);
		dag_dagcopy(dst,src->branch,transaction,number);
	}
	else if (src->trunk)
		dag_dagcopy(dst,src->trunk,transaction,number);
	else if (src->branch)
		dag_dagcopy(dst,src->branch,transaction,number);
}

//dag copy:copy dag's tip+dag
uint32 dag_copy(transaction_t *dst,transaction_t *src,transaction_t **transaction)
{
	uint32 number;

	number=0;
	dag_dagcopy(dst,src,transaction,number);
	dag_tipcopy(dst,src,transaction,number);

	return number;
}

//dag copy:copy dag
void dag_copy(transaction_t *dst,transaction_t *src,uint32 number,uint16 &level)
{
	uint32 i,j,k;
	uint32 num,length;
	transaction_t *transaction,**point;

	num=number;
	point=new transaction_t*[number];
	number=dag_copy(dst,src,point);
	//modify flag/reserved/trunk/branch/next
	for (i=0;i<number;i++)
	{
		dst[i].flag=0;
		dst[i].reserved=0xffff;
		for (j=0;j<number;j++)
			if (point[j]->index==dst[i].index)
				break;
		dst[i].trunk=NULL;
		if (point[j]->trunk)
		{
			for (k=0;k<number;k++)
				if (dst[k].index==point[j]->trunk->index)
					break;
			dst[i].trunk=&dst[k];
		}
		dst[i].branch=NULL;
		if (point[j]->branch)
		{
			for (k=0;k<number;k++)
				if (dst[k].index==point[j]->branch->index)
					break;
			dst[i].branch=&dst[k];
		}
		dst[i].next=NULL;
		if (point[j]->next)
		{
			for (k=0;k<number;k++)
				if (dst[k].index==point[j]->next->index)
					break;
			dst[i].next=&dst[k];
		}
	}
	delete[] point;
	//modify reserved
	level=0;
	length=0;
	transaction=src;
	while(transaction)
	{
		length++;
		for (i=0;i<number;i++)
			if (dst[i].index==transaction->index)
				break;
		dst[i].reserved=level;
		dst[i].flag=1;
		transaction=transaction->next;
	}
	while(1)
	{
		level++;
		num-=length;
		if (!num)
			break;
		//
		length=0;
		for (i=0;i<number;i++)
		{
			if (dst[i].reserved!=level-1)
				continue;
			if (dst[i].trunk && !dst[i].trunk->flag)
			{
				length++;
				dst[i].trunk->reserved=level;
				dst[i].trunk->flag=1;
			}
			if (dst[i].branch && !dst[i].branch->flag && dst[i].trunk!=dst[i].branch)
			{
				length++;
				dst[i].branch->reserved=level;
				dst[i].branch->flag=1;
			}
		}
	}
	dag_clear(dst);
}

//dag tip:adjust tip number in runtime(initial structure,width adjust)
uint32 dag_tip(void)
{
	return TIP_NUM;
}

//dag height:longest path from genesis to current transaction in all pathes(NP-Hard)
uint32 dag_height(transaction_t *dag,transaction_t *transaction)
{
	return 0;
}

//dag depth:longest path from current transaction to certain tip
uint32 dag_depth(transaction_t *dag,transaction_t *transaction)
{
	return 0;
}

//device delete
void device_delete(device_t *device)
{
	//route
	route_delete(device);
	//queue
	queue_delete(device->queue);
	//rsa
	delete[] device->rsa.e;
	delete[] device->rsa.n;
	delete[] device->rsa.p;
	delete[] device->rsa.q;
	delete[] device->rsa.d;
	delete[] device->rsa.dp;
	delete[] device->rsa.dq;
	delete[] device->rsa.qp;
}

//mainchain delete
void mainchain_delete(mainchain_t *mainchain)
{
	//queue
	queue_delete(mainchain->queue);
	//list
	delete[] mainchain->list;
	//dag
	dag_delete(mainchain);
}

//generate device->rsa
void key_generate(device_t *device)
{
	uint8 flag;

	//malloc
	device->rsa.le=KEY_E;
	device->rsa.len=KEY_LEN;
	device->rsa.lr=KEY_MASK;
	device->rsa.e=new uint8[device->rsa.le];
	device->rsa.n=new uint8[device->rsa.len];
	device->rsa.p=new uint8[device->rsa.len>>1];
	device->rsa.q=new uint8[device->rsa.len>>1];
	device->rsa.d=new uint8[device->rsa.len];
	device->rsa.dp=new uint8[device->rsa.len>>1];
	device->rsa.dq=new uint8[device->rsa.len>>1];
	device->rsa.qp=new uint8[device->rsa.len>>1];
	//generate
	device->rsa.e[0]=0x01;
	device->rsa.e[1]=0x00;
	device->rsa.e[2]=0x01;
	device->rsa.e[3]=0x00;
	while(1)
	{
		_rand(device->rsa.p,device->rsa.len>>1);
		_rand(device->rsa.q,device->rsa.len>>1);
		flag=rsa_genkey(&device->rsa,RSA_CRT);
		if (!flag)
			break;
	}
}

//move device
void move_location(device_t *device,uint32 step,uint32 range)
{
	if (rand()%2)
	{
		device->x+=step;
		device->x=math_min(device->x,range-1);
	}
	else
	{
		device->x-=step;
		device->x=math_max(device->x,(uint32)0);
	}
	if (rand()%2)
	{
		device->y+=step;
		device->y=math_min(device->y,range-1);
	}
	else
	{
		device->y-=step;
		device->y=math_max(device->y,(uint32)0);
	}
}

void delay(void)
{
	//Sleep(300);
}