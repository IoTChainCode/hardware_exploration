// CommandView.cpp : implementation file
//

#include "stdafx.h"
#include "BlockchainMfc.h"
#include "CommandView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
//
uint8 g_run;//0-stop,1-run
uint8 **g_check;//[g_number][5].0-source,1-mc(src),2-mc(dst),3-node(src),4-node(dst)
transaction_t *g_dag;//for draw

/////////////////////////////////////////////////////////////////////////////
// CCommandView

IMPLEMENT_DYNCREATE(CCommandView, CFormView)

CCommandView::CCommandView()
	: CFormView(CCommandView::IDD)
{
	m_flag=0;
}

CCommandView::~CCommandView()
{
	uint32 i;

	g_run=0;
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
		device_delete(&g_device[i]);
	delete[] g_device;
	mainchain_delete(&g_mainchain);
}

void CCommandView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommandView)
	DDX_Control(pDX, IDC_BUTTON_RANDOM, m_random);
	DDX_Control(pDX, IDC_BUTTON_LIMIT, m_limit);
	DDX_Control(pDX, IDC_LIST_TRANSACTION, m_transaction);
	DDX_Control(pDX, IDC_LIST_DEVICE, m_device);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCommandView, CFormView)
	//{{AFX_MSG_MAP(CCommandView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_LIMIT, OnButtonLimit)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_RANDOM, OnButtonRandom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommandView diagnostics

#ifdef _DEBUG
void CCommandView::AssertValid() const
{
	CFormView::AssertValid();
}

void CCommandView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCommandView message handlers
void CCommandView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	//fill list
	m_transaction.SetExtendedStyle(LVS_EX_GRIDLINES);//set grid
	m_transaction.InsertColumn(0,"Transaction",LVCFMT_LEFT,80);
	m_transaction.InsertColumn(1,"Source",LVCFMT_LEFT,60);
	m_transaction.InsertColumn(2,"MC(Src)",LVCFMT_LEFT,60);
	m_transaction.InsertColumn(3,"MC(Dst)",LVCFMT_LEFT,60);
	m_transaction.InsertColumn(4,"Node(Src)",LVCFMT_LEFT,70);
	m_transaction.InsertColumn(5,"Node(Dst)",LVCFMT_LEFT,70);
	m_device.SetExtendedStyle(LVS_EX_GRIDLINES);//set grid
	m_device.InsertColumn(0,"Node",LVCFMT_LEFT,60);
	m_device.InsertColumn(1,"Token(Available)",LVCFMT_LEFT,170);
	m_device.InsertColumn(2,"Token(Frozen)",LVCFMT_LEFT,170);
	//initial var
	m_flag=1;
	//m_level=0;
}

void CCommandView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	
	if (m_flag)
	{
		CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
		cx=math_max((int)pMain->m_scrollsize[2].cx,cx);
		cy=math_max((int)pMain->m_scrollsize[2].cy,cy);
		//adjust list
		m_transaction.MoveWindow(0,0,cx,cy*7/10);
		m_device.MoveWindow(0,cy*7/10,cx,cy/5);
		//adjust button
		m_limit.MoveWindow(math_max(cx/4-50,0),cy*19/20-14,100,28);
		m_random.MoveWindow(math_max(cx*3/4-50,0),cy*19/20-14,100,28);
	}
}

void CCommandView::OnButtonLimit() 
{
	uint32 i;
	CString str;

	//reset device/mainchain
	g_index=0;
	g_flag=0;
	for (i=0;i<g_number;i++)
		memset(g_check[i],0,5);
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
		device_delete(&g_device[i]);
	delete[] g_device;
	mainchain_delete(&g_mainchain);
	//initial variable
	InitialVariable();
	//interface
	m_limit.SetWindowText("running...");
	m_limit.EnableWindow(FALSE);
	m_transaction.DeleteAllItems();
	for (i=0;i<g_number;i++)
	{
		str.Format("%ld->%ld:%ld",g_deal[i].device_index[0],g_deal[i].device_index[1],g_deal[i].token);
		m_transaction.InsertItem(i,str);
		memset(g_check[i],0,5);
	}
	m_device.DeleteAllItems();
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		str.Format("%ld",g_device[i].device_index);
		m_device.InsertItem(i,str);
		str.Format("%ld",g_device[i].token[0]);
		m_device.SetItemText(i,1,str);
		str.Format("%ld",g_device[i].token[1]);
		m_device.SetItemText(i,2,str);
	}
	m_random.EnableWindow(FALSE);
	//invalidate ConnectView/TransactionView
	CConnectView *pConnectView=(CConnectView *)GetView(RUNTIME_CLASS(CConnectView));
	pConnectView->Invalidate();
	CTransactionView *pTransactionView=(CTransactionView *)GetView(RUNTIME_CLASS(CTransactionView));
	pTransactionView->Invalidate();
	//initial thread
	InitialThread();
}

void CCommandView::OnButtonRandom() 
{
	uint32 i;
	CString str;

	//reset device/mainchain
	g_index=0;
	g_flag=0;
	for (i=0;i<g_number;i++)
		memset(g_check[i],0,5);
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
		device_delete(&g_device[i]);
	delete[] g_device;
	mainchain_delete(&g_mainchain);
	//random variable
	RandomVariable();
	//interface
	m_random.SetWindowText("running...");
	m_random.EnableWindow(FALSE);
	m_transaction.DeleteAllItems();
	for (i=0;i<g_number;i++)
	{
		str.Format("%ld->%ld:%ld",g_deal[i].device_index[0],g_deal[i].device_index[1],g_deal[i].token);
		m_transaction.InsertItem(i,str);
		memset(g_check[i],0,5);
	}
	m_device.DeleteAllItems();
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		str.Format("%ld",g_device[i].device_index);
		m_device.InsertItem(i,str);
		str.Format("%ld",g_device[i].token[0]);
		m_device.SetItemText(i,1,str);
		str.Format("%ld",g_device[i].token[1]);
		m_device.SetItemText(i,2,str);
	}
	m_limit.EnableWindow(FALSE);
	//invalidate ConnectView/TransactionView
	CConnectView *pConnectView=(CConnectView *)GetView(RUNTIME_CLASS(CConnectView));
	pConnectView->Invalidate();
	CTransactionView *pTransactionView=(CTransactionView *)GetView(RUNTIME_CLASS(CTransactionView));
	pTransactionView->Invalidate();
	//initial thread
	InitialThread();
}

void CCommandView::OnTimer(UINT nIDEvent) 
{
	uint32 i,j,k;
	CString str;
	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	
	//static uint32 count=0;
	//EnterCriticalSection(&g_cs);
	/*
	if (count++==100)
	{
		count=0;
		g_flag=!g_flag;
	}
	//for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	//	move_location(&g_device[i],g_devicestep,g_devicerange);
	//
	*/
	//LeaveCriticalSection(&g_cs);

	//interface
	for (i=0;i<g_number;i++)
		for (j=0;j<5;j++)
		{
			if (g_check[i][j])
				m_transaction.SetItemText(i,j+1,"¡Ì");
		}
	m_transaction.UpdateData();
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		str.Format("%ld",g_device[i].token[0]);
		m_device.SetItemText(i,1,str);
		str.Format("%ld",g_device[i].token[1]);
		m_device.SetItemText(i,2,str);
	}
	m_device.UpdateData();
	//check
	k=0;
	for (i=0;i<g_number;i++)
		for (j=0;j<5;j++)
			k+=g_check[i][j];
	if (k>=g_number*5-4)
	{
		KillTimer(0);
		g_run=0;
		m_limit.SetWindowText("limit case");
		m_limit.EnableWindow(TRUE);
		m_random.SetWindowText("random case");
		m_random.EnableWindow(TRUE);
		//
		EnterCriticalSection(&g_cs);
		if (g_mainchain.dag)
		{
			pMain->m_number=dag_num(g_mainchain.dag);
			dag_clear(g_mainchain.dag);
			if (g_dag)
				delete[] g_dag;
			g_dag=new transaction_t[pMain->m_number];
			dag_copy(g_dag,g_mainchain.dag,pMain->m_number,pMain->m_level);
			dag_clear(g_mainchain.dag);
		}
		LeaveCriticalSection(&g_cs);
		//invalidate ConnectView/TransactionView
		CConnectView *pConnectView=(CConnectView *)GetView(RUNTIME_CLASS(CConnectView));
		pConnectView->Invalidate();
		CTransactionView *pTransactionView=(CTransactionView *)GetView(RUNTIME_CLASS(CTransactionView));
		pTransactionView->Invalidate();
	}
	
	CFormView::OnTimer(nIDEvent);
}

CView *CCommandView::GetView(CRuntimeClass *pClass)
{
	uint8 flag;
	CView *pView;
	POSITION pos;
    CBlockchainMfcDoc *pDoc=(CBlockchainMfcDoc *)GetDocument();
	
	flag=0;
	pos=pDoc->GetFirstViewPosition();
	while(pos!=NULL)
	{
		pView=pDoc->GetNextView(pos);
		if (pView->IsKindOf(pClass))
		{
			flag=1;
			break;
		}
	}
    if (!flag)
		return NULL;

	return pView;
}

//mainchain thread
uint32 WINAPI thread_mainchain(PVOID pParam)
{
	uint8 flag;
	mainchain_t *mainchain;

	flag=0;
	mainchain=(mainchain_t *)pParam;
	while(1)
	{
		if (!g_run && flag)
			break;
		flag=1;
		EnterCriticalSection(&g_cs);
		process_mainchain(mainchain);
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

//device thread
uint32 WINAPI thread_device(PVOID pParam)
{
	uint8 flag;
	device_t *device;

	flag=0;
	device=(device_t *)pParam;
	while(1)
	{
		if (!g_run && flag)
			break;
		flag=1;
		EnterCriticalSection(&g_cs);
		process_device(device);
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

void CCommandView::InitialVariable(void)
{
	uint32 account;//account number
	uint32 token;//initial token
	//
	uint32 i;
	int8 buf[1000],*point[2];
	queue_t *queue;
	FILE *file;

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
	//add for mfc
	g_run=1;
	g_check=new uint8*[g_number];
	for (i=0;i<g_number;i++)
	{
		g_check[i]=new uint8[5];
		memset(g_check[i],0,5);
	}
	g_dag=NULL;
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
}

void CCommandView::RandomVariable(void)
{
	uint32 account;//account number
	uint32 token;//initial token
	//
	uint32 i;
	queue_t *queue;

	//random variable
	g_devicenum[0]=rand()%MAX_HEAVYNODE+1;//g_devicenum[0]
	g_devicenum[1]=rand()%MAX_LIGHTNODE+1;//g_devicenum[1]
	g_devicerange=RANDOM_RANGE;//g_devicerange
	g_devicestep=RANDOM_STEP;//g_devicestep
	account=RANDOM_ACCOUNT;//account
	token=rand()%MAX_TOKEN+1;//token
	g_number=rand()%MAX_DEAL+1;
	g_deal=new deal_t[g_number];
	for (i=0;i<g_number;i++)
	{
		g_deal[i].device_index[0]=rand()%(g_devicenum[0]+g_devicenum[1]);
		g_deal[i].device_index[1]=rand()%(g_devicenum[0]+g_devicenum[1]);
		g_deal[i].token=rand()%MIN_TOKEN;
	}
	//add for mfc
	g_run=1;
	g_check=new uint8*[g_number];
	for (i=0;i<g_number;i++)
	{
		g_check[i]=new uint8[5];
		memset(g_check[i],0,5);
	}
	g_dag=NULL;
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
}

uint8 CCommandView::InitialThread(void)
{
	uint32 i;
	HANDLE thread_handle;
	uint32 thread_id;

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
			//printf("initial thread_device failed\r\n");
			return 1;
		}
	}
	thread_handle=CreateThread(NULL,0,thread_mainchain,(PVOID)&g_mainchain,0,&thread_id);
	if (!thread_handle)
	{
		//printf("initial thread_mainchain failed\r\n");
		return 1;
	}
	SetTimer(0,TIMER_CONNECT*1000,NULL);//each device thread should have individual timer, use 1 timer here totally

	return 0;
}