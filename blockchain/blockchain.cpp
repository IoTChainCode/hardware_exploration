#include "blockchain.h"

//global var
uint32 g_devicenum[2];//device number.0-heavy device,1-light device
uint32 g_devicerange;//device range
uint32 g_devicestep;//device step
uint32 g_number;//deal number
deal_t *g_deal;//deal list
CRITICAL_SECTION g_cs;//critical section
device_t *g_device;//device array
mainchain_t g_mainchain;//mainchain
volatile uint32 g_index;//transaction index(temporary use, start from 1, instead by hash_t later)
volatile uint8 g_flag;//for timer resend

//mainchain thread
uint32 WINAPI thread_mainchain(PVOID pParam)
{
	mainchain_t *mainchain;

	mainchain=(mainchain_t *)pParam;
	while(1)
	{
		EnterCriticalSection(&g_cs);
		process_mainchain(mainchain);
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

//device thread
uint32 WINAPI thread_device(PVOID pParam)
{
	device_t *device;

	device=(device_t *)pParam;
	while(1)
	{
		EnterCriticalSection(&g_cs);
		process_device(device);
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

void main(int argc,char* argv[])
{
	uint32 account;//account number
	uint32 token;//initial token
	//
	uint32 i;
	MSG msg;
	//uint8 flag;
	int8 buf[1000],*point[2];
	FILE *file;
	HANDLE thread_handle;
	uint32 thread_id;
	queue_t *queue;

	//read initial.ini
	file=fopen("initial.ini","r");
	fgets(buf,1000,file);//[network]
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	g_devicenum[0]=atol(&buf[13]);//g_devicenum[0]
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	g_devicenum[1]=atol(&buf[13]);//g_devicenum[1]
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	g_devicerange=atol(&buf[13]);//g_devicerange
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	g_devicestep=atol(&buf[12]);//g_devicestep
	fgets(buf,1000,file);//[blockchain]
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	account=atol(&buf[8]);//account
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	token=atol(&buf[6]);//token
	fgets(buf,1000,file);//[transaction]
	i=ftell(file);
	g_number=0;
	while(1)
	{
		if (feof(file))
			break;
		memset(buf,0,1000*sizeof(int8));
		fgets(buf,1000,file);
		if (!strcmp(buf,""))
			break;
		g_number++;
	}
	g_deal=new deal_t[g_number];
	fseek(file,i,SEEK_SET);
	g_number=0;
	while(1)
	{
		if (feof(file))
			break;
		memset(buf,0,1000*sizeof(int8));
		fgets(buf,1000,file);
		if (!strcmp(buf,""))
			break;
		buf[strlen(buf)-1]=0;
		point[0]=buf;
		point[1]=strchr(point[0],',');
		*point[1]='\0';
		g_deal[g_number].device_index[0]=atol(point[0]);
		point[0]=point[1]+1;
		point[1]=strchr(point[0],',');
		*point[1]='\0';
		g_deal[g_number].device_index[1]=atol(point[0]);
		point[0]=point[1]+1;
		g_deal[g_number].token=atol(point[0]);
		g_number++;
	}
	fclose(file);
	//0.initial device/timer/thread_device/thread_mainchain/cs
	InitializeCriticalSection(&g_cs);
	srand((unsigned)time(NULL));
	g_index=0;
	g_flag=0;
	g_device=new device_t[g_devicenum[0]+g_devicenum[1]];
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		g_device[i].x=rand()%g_devicerange;
		g_device[i].y=rand()%g_devicerange;
		g_device[i].node=i<g_devicenum[0] ? NODE_HEAVY : NODE_LIGHT;//rand()%2 ? NODE_LIGHT : NODE_HEAVY;
		g_device[i].device_index=i;
		g_device[i].route=NULL;
		g_device[i].queue=NULL;
		queue=new queue_t;
		queue->step=STEP_CONNECT;
		queue->data=new uint8[1*sizeof(uint32)];
		*(uint32 *)queue->data=0;//align problem?
		queue_insert(&g_device[i],queue);
		key_generate(&g_device[i]);
		g_device[i].token[0]=token;
		g_device[i].token[1]=0;
		//g_device[i].dag=NULL;


		//g_device[i].line=DEVICE_LINE_ON;
		//g_device[i].status=STATUS_FREE;
		//g_device[i].step=STEP_CONNECT;
		//g_device[i].dag_index=0;
		//memset((void *)g_device[i].buffer,0,BUFFER_LENGTH*sizeof(uint8));
		/*
		memset((void *)g_device[i].queue,0,QUEUE_LENGTH*sizeof(queue_t));//INFO_TX
		g_device[i].queue_index=0;
		g_device[i].tangle_index=0;
		g_device[i].transaction_index=0;
		g_device[i].key_index=0;
		g_device[i].account_id=rand()%account_num;
		g_device[i].account_money=token;*/
	}
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		//lpThreadAttributes:point to security attributes
		//dwStackSize:stack size
		//lpStartAddress:thread function.define by uint32 WINAPI xxx(PVOID pParam)
		//lpParameter:para area
		//dwCreationFlags:thread status(suspend,running)
		//lpThreadId:thread id
		thread_handle=CreateThread(NULL,0,thread_device,(PVOID)&g_device[i],0,&thread_id);
		if (!thread_handle)
		{
			printf("initial thread_device failed\r\n");
			return;
		}
	}
	g_mainchain.queue=NULL;
	g_mainchain.dag_number=0;
	g_mainchain.list_number=g_devicenum[0]+g_devicenum[1];
	g_mainchain.list=new list_t[g_mainchain.list_number];
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		g_mainchain.list[i].device_index=i;
		g_mainchain.list[i].token=token;
		g_mainchain.list[i].node=NODE_NONE;
	}
	g_mainchain.dag=NULL;
	thread_handle=CreateThread(NULL,0,thread_mainchain,(PVOID)&g_mainchain,0,&thread_id);
	if (!thread_handle)
	{
		printf("initial thread_mainchain failed\r\n");
		return;
	}
	SetTimer(NULL,1,TIMER_CONNECT*1000,NULL);//each device thread should have individual timer, use 1 timer here totally
	//msg loop
	while(1)
	{
		GetMessage(&msg,NULL,0,0);
#if 1
		EnterCriticalSection(&g_cs);
		g_flag=!g_flag;
		for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
			move_location(&g_device[i],g_devicestep,g_devicerange);
		/*
		flag=0;
		if (g_mainchain.queue && g_mainchain.queue->step==STEP_CONNECT)
			flag=1;
		for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
			if (g_device[i].queue && g_device[i].queue->step==STEP_CONNECT)
			{
				flag=1;
				break;
			}
		if (!flag)
			printf("ok");*/
		
		//flag=0;
		/*
		//if (g_mainchain.queue && g_mainchain.queue->step!=STEP_MOVE)
		//	flag=1;
		for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
			if (g_device[i].queue && g_device[i].queue->step!=STEP_MOVE)
			{
				flag=1;
				break;
			}
		if (!flag)
			printf("ok");*/
		//if (g_index==g_number)
		//	printf("ok");

		/*
		for (i=0;i<g_devicenum;i++)
			printf("%d",g_device[i].step);
		printf("\r\n");
		*/
		LeaveCriticalSection(&g_cs);
#endif
	}
	//release
	delete[] g_deal;
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
		device_delete(&g_device[i]);
	delete[] g_device;
	mainchain_delete(&g_mainchain);
	DeleteCriticalSection(&g_cs);
}
