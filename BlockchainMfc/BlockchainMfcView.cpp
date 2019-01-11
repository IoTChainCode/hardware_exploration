// BlockchainMfcView.cpp : implementation of the CBlockchainMfcView class
//

#include "stdafx.h"
#include "BlockchainMfc.h"

#include "BlockchainMfcDoc.h"
#include "BlockchainMfcView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView

IMPLEMENT_DYNCREATE(CBlockchainMfcView, CView)

BEGIN_MESSAGE_MAP(CBlockchainMfcView, CView)
	//{{AFX_MSG_MAP(CBlockchainMfcView)
	//}}AFX_MSG_MAP
	// Standard printing commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView construction/destruction

CBlockchainMfcView::CBlockchainMfcView()
{
}

CBlockchainMfcView::~CBlockchainMfcView()
{
}

BOOL CBlockchainMfcView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView drawing

void CBlockchainMfcView::OnDraw(CDC* pDC)
{
	CBlockchainMfcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView diagnostics

#ifdef _DEBUG
void CBlockchainMfcView::AssertValid() const
{
	CView::AssertValid();
}

void CBlockchainMfcView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBlockchainMfcDoc* CBlockchainMfcView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBlockchainMfcDoc)));
	return (CBlockchainMfcDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView message handlers