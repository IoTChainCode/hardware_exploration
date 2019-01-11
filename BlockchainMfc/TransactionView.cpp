// TransactionView.cpp : implementation file
//

#include "stdafx.h"
#include "BlockchainMfc.h"
#include "TransactionView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern
extern mainchain_t g_mainchain;//mainchain
extern transaction_t *g_dag;

/////////////////////////////////////////////////////////////////////////////
// CTransactionView

IMPLEMENT_DYNCREATE(CTransactionView, CScrollView)

CTransactionView::CTransactionView()
{
}

CTransactionView::~CTransactionView()
{
}

BEGIN_MESSAGE_MAP(CTransactionView, CScrollView)
	//{{AFX_MSG_MAP(CTransactionView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransactionView drawing

void CTransactionView::OnInitialUpdate()
{
	CSize size;
	CScrollView::OnInitialUpdate();

	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	size.cx=pMain->m_scrollsize[1].cx;
	size.cy=0;
	SetScrollSizes(MM_TEXT,size);
}

void CTransactionView::OnDraw(CDC* pDC)
{
	CSize size;
	CBrush brush,*p_brush;
	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;

	//draw dag
	brush.CreateSolidBrush(RGB(0,0,0));
	p_brush=pDC->SelectObject(&brush);
	if (g_dag)
		DagDraw(g_dag,pDC,&brush);
	pDC->SelectObject(p_brush);
	//expand scrollsize
	//SetScrollPos(SB_HORZ,pMain->m_scrollsize[1].cx);
	//SetScrollPos(SB_VERT,pMain->m_scrollsize[1].cy);
	size.cx=pMain->m_scrollsize[1].cx;
	size.cy=0;
	SetScrollSizes(MM_TEXT,size);
}

/////////////////////////////////////////////////////////////////////////////
// CTransactionView diagnostics

#ifdef _DEBUG
void CTransactionView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CTransactionView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTransactionView message handlers

void CTransactionView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	if (pMain)
	{
		//pMain->m_scrollsize[1].cx=cx;
		pMain->m_scrollsize[1].cy=cy;
	}
}

void CTransactionView::DagDraw(transaction_t *dag,CDC* pDC,CBrush *brush)
{
	uint32 i,j,k,ii,jj,kk;
	int32 x,y;
	double angle;
	RECT rect;
	CString str,string;
	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;

	//draw
	for (i=0;i<pMain->m_level;i++)
	{
		k=0;
		for (j=0;j<pMain->m_number;j++)
		{
			if (dag[j].reserved!=i)
				continue;
			//draw transaction
			rect.left=k*100;
			rect.right=rect.left+20;
			rect.top=i*100;
			rect.bottom=rect.top+20;
			if (dag[j].transaction==TRANSACTION_TIP)
				pDC->Rectangle(&rect);
			else
				pDC->FillRect(&rect,brush);
			str.Format("%ld",dag[j].index);
			pDC->TextOut(rect.left,rect.top,str);
			//draw arrow
			if (dag[j].trunk)
			{
				for (ii=0;ii<pMain->m_number;ii++)
					if (dag[ii].index==dag[j].trunk->index)
					{
						kk=ii;
						break;
					}
				jj=0;
				for (ii=0;ii<kk;ii++)
					if (dag[ii].reserved==dag[kk].reserved)
					{
						jj++;
						break;
					}
				pDC->MoveTo(rect.left+10,rect.top+10);
				if (dag[ii].reserved==i)//same level
				{
					x=jj*100+(jj>k ? 0 : 20);
					y=dag[kk].reserved*100+10;
					pDC->LineTo(x,y);
					if (jj>k)
					{
						pDC->LineTo(x-(uint32)(cos(PI/4)*8),y+(uint32)(sin(PI/4)*8));
						pDC->MoveTo(x,y);
						pDC->LineTo(x-(uint32)(cos(PI/4)*8),y-(uint32)(sin(PI/4)*8));
					}
					else
					{
						pDC->LineTo(x+(uint32)(cos(PI/4)*8),y+(uint32)(sin(PI/4)*8));
						pDC->MoveTo(x,y);
						pDC->LineTo(x+(uint32)(cos(PI/4)*8),y-(uint32)(sin(PI/4)*8));
					}
				}
				else//diff level
				{
					x=jj*100+10;
					y=dag[ii].reserved*100+(dag[kk].reserved>i ? 0 : 20);
					pDC->LineTo(x,y);
					angle=x==rect.left+10 ? 0 : -atan((rect.top+10-y)/(double)(rect.left+10-x));
					if (dag[kk].reserved>i)
					{
						pDC->LineTo(x-(uint32)(cos(angle+PI/4)*8),y-(uint32)(sin(angle+PI/4)*8));
						pDC->MoveTo(x,y);
						pDC->LineTo(x+(uint32)(cos(angle-PI/4)*8),y+(uint32)(sin(angle-PI/4)*8));
					}
					else
					{
						pDC->LineTo(x+(uint32)(cos(angle+PI/4)*8),y+(uint32)(sin(angle+PI/4)*8));
						pDC->MoveTo(x,y);
						pDC->LineTo(x-(uint32)(cos(angle-PI/4)*8),y+(uint32)(sin(angle-PI/4)*8));
					}
				}
			}
			if (dag[j].branch)
			{
				for (ii=0;ii<pMain->m_number;ii++)
					if (dag[ii].index==dag[j].branch->index)
					{
						kk=ii;
						break;
					}
				jj=0;
				for (ii=0;ii<kk;ii++)
					if (dag[ii].reserved==dag[kk].reserved)
					{
						jj++;
						break;
					}
				pDC->MoveTo(rect.left+10,rect.top+10);
				if (dag[ii].reserved==i)//same level
				{
					x=jj*100+(jj>k ? 0 : 20);
					y=dag[kk].reserved*100+10;
					pDC->LineTo(x,y);
					if (jj>k)
					{
						pDC->LineTo(x-(uint32)(cos(PI/4)*8),y+(uint32)(sin(PI/4)*8));
						pDC->MoveTo(x,y);
						pDC->LineTo(x-(uint32)(cos(PI/4)*8),y-(uint32)(sin(PI/4)*8));
					}
					else
					{
						pDC->LineTo(x+(uint32)(cos(PI/4)*8),y+(uint32)(sin(PI/4)*8));
						pDC->MoveTo(x,y);
						pDC->LineTo(x+(uint32)(cos(PI/4)*8),y-(uint32)(sin(PI/4)*8));
					}
				}
				else//diff level
				{
					x=jj*100+10;
					y=dag[ii].reserved*100+(dag[kk].reserved>i ? 0 : 20);
					pDC->LineTo(x,y);
					angle=x==rect.left+10 ? 0 : -atan((rect.top+10-y)/(double)(rect.left+10-x));
					if (dag[kk].reserved>i)
					{
						pDC->LineTo(x-(uint32)(cos(angle+PI/4)*8),y-(uint32)(sin(angle+PI/4)*8));
						pDC->MoveTo(x,y);
						pDC->LineTo(x+(uint32)(cos(angle-PI/4)*8),y+(uint32)(sin(angle-PI/4)*8));
					}
					else
					{
						pDC->LineTo(x+(uint32)(cos(angle+PI/4)*8),y+(uint32)(sin(angle+PI/4)*8));
						pDC->MoveTo(x,y);
						pDC->LineTo(x-(uint32)(cos(angle-PI/4)*8),y+(uint32)(sin(angle-PI/4)*8));
					}
				}
			}
			//
			k++;
		}
	}
	//print
	pDC->TextOut(0,pMain->m_level*100,"Transaction:    Trunk    Branch");
	for (i=0;i<pMain->m_number;i++)
	{
		//draw transaction/trunk/branch
		string="";
		str.Format("%ld:                     ",dag[i].index);
		string+=str;
		if (dag[i].trunk)
			str.Format("%ld                  ",dag[i].trunk->index);
		else
			str="NULL      ";
		string+=str;
		if (dag[i].branch)
			str.Format("%ld",dag[i].branch->index);
		else
			str="NULL";
		string+=str;
		pDC->TextOut(0,pMain->m_level*100+(i+1)*20,string);
	}
	pMain->m_scrollsize[1].cy=pMain->m_level*100+(pMain->m_number+1)*20;
}