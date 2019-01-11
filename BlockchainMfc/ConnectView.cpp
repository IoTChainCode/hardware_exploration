// ConnectView.cpp : implementation file
//

#include "stdafx.h"
#include "BlockchainMfc.h"
#include "ConnectView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern
extern uint32 g_devicenum[2];//device number.0-heavy device,1-light device
extern uint32 g_devicerange;//device range
extern device_t *g_device;//device array
extern mainchain_t g_mainchain;

/////////////////////////////////////////////////////////////////////////////
// CConnectView

IMPLEMENT_DYNCREATE(CConnectView, CScrollView)

CConnectView::CConnectView()
{
}

CConnectView::~CConnectView()
{
}

BEGIN_MESSAGE_MAP(CConnectView, CScrollView)
	//{{AFX_MSG_MAP(CConnectView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectView drawing

void CConnectView::OnInitialUpdate()
{
	CSize size;
	CScrollView::OnInitialUpdate();

	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	size.cx=pMain->m_scrollsize[0].cx;
	size.cy=0;
	SetScrollSizes(MM_TEXT,size);
}

void CConnectView::OnDraw(CDC* pDC)
{
	uint32 i,x,y;
	CBrush brush,*p_brush;
	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;

	//draw device
	brush.CreateSolidBrush(RGB(0,0,0));
	pDC->Rectangle(0,(pMain->m_scrollsize[0].cy-pMain->m_scrollsize[0].cx)/2,pMain->m_scrollsize[0].cx,(pMain->m_scrollsize[0].cy+pMain->m_scrollsize[0].cx)/2);
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		x=pMain->m_scrollsize[0].cx*g_device[i].x/g_devicerange;
		y=pMain->m_scrollsize[0].cx*g_device[i].y/g_devicerange+(pMain->m_scrollsize[0].cy-pMain->m_scrollsize[0].cx)/2;
		pDC->Ellipse(x,y,x+5,y+5);
		if (g_device[i].node==NODE_HEAVY)
		{
			p_brush=pDC->SelectObject(&brush);
			pDC->FloodFill(x+2,y+2,RGB(0,0,0));
			pDC->SelectObject(p_brush);
		}
	}
	//draw mainchain
	x=pMain->m_scrollsize[0].cx/2;
	y=(pMain->m_scrollsize[0].cy-pMain->m_scrollsize[0].cx)/4;
	//pDC->Ellipse(x,y,x+20,y+20);
	pDC->Rectangle(x-50,y-10,x+50,y+10);
	p_brush=pDC->SelectObject(&brush);
	//pDC->FloodFill(x+10,y+10,RGB(0,0,0));
	pDC->FloodFill(x,y,RGB(0,0,0));
	pDC->SelectObject(p_brush);
	pDC->TextOut(x-33,y-8,"mainchain");
	//draw arrow
	if (g_mainchain.dag)
	{
	}
	//expand scrollsize
	//SetScrollPos(SB_HORZ,pMain->m_scrollsize[0].cx);
	//SetScrollPos(SB_VERT,pMain->m_scrollsize[0].cy);
}

/////////////////////////////////////////////////////////////////////////////
// CConnectView diagnostics

#ifdef _DEBUG
void CConnectView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CConnectView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CConnectView message handlers

void CConnectView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	if (pMain)
	{
//		pMain->m_scrollsize[0].cx=cx;
		pMain->m_scrollsize[0].cy=cy;
	}
}
//m_scrollsize[0].cx=screen.cx/5;m_scrollsize[0].cy=screen.cy-20;
