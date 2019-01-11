#if !defined(AFX_TRANSACTIONVIEW_H__9C9D8A00_8BEE_40DD_A865_585C03731F07__INCLUDED_)
#define AFX_TRANSACTIONVIEW_H__9C9D8A00_8BEE_40DD_A865_585C03731F07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransactionView.h : header file
//

//include
#include "BlockchainMfcDoc.h"
#include "MainFrm.h"
#include "layer_device.h"
#include "layer_mainchain.h"

/////////////////////////////////////////////////////////////////////////////
// CTransactionView view
class CTransactionView : public CScrollView
{
protected:
	CTransactionView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTransactionView)
public:
	void DagDraw(transaction_t *dag,CDC* pDC,CBrush *brush);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransactionView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTransactionView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CTransactionView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSACTIONVIEW_H__9C9D8A00_8BEE_40DD_A865_585C03731F07__INCLUDED_)
