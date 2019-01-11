// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "BlockchainMfc.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction
CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	CSize screen;
	BOOL result;

	screen.cx=GetSystemMetrics(SM_CXFULLSCREEN);//should drop the task bar
	screen.cy=GetSystemMetrics(SM_CYFULLSCREEN);//should drop the task bar
	m_scrollsize[0].cx=screen.cx/5;m_scrollsize[0].cy=screen.cy-20;
	m_scrollsize[1].cx=screen.cx/2;m_scrollsize[1].cy=screen.cy-20;
	m_scrollsize[2].cx=screen.cx*3/10;m_scrollsize[2].cy=screen.cy-20;
	result=m_wndSplitter.CreateStatic(this,1,3);
    result|=m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CConnectView),m_scrollsize[0],pContext);
	result|=m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CTransactionView),m_scrollsize[1],pContext);
	result|=m_wndSplitter.CreateView(0,2,RUNTIME_CLASS(CCommandView),m_scrollsize[2],pContext);

	return result;
}