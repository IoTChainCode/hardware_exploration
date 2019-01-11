// BlockchainMfcDoc.cpp : implementation of the CBlockchainMfcDoc class
//

#include "stdafx.h"
#include "BlockchainMfc.h"

#include "BlockchainMfcDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc

IMPLEMENT_DYNCREATE(CBlockchainMfcDoc, CDocument)

BEGIN_MESSAGE_MAP(CBlockchainMfcDoc, CDocument)
	//{{AFX_MSG_MAP(CBlockchainMfcDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc construction/destruction
CBlockchainMfcDoc::CBlockchainMfcDoc()
{
}

CBlockchainMfcDoc::~CBlockchainMfcDoc()
{
}

BOOL CBlockchainMfcDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc serialization

void CBlockchainMfcDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc diagnostics

#ifdef _DEBUG
void CBlockchainMfcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBlockchainMfcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc commands
