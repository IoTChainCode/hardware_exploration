#if !defined(AFX_CONNECTVIEW_H__FF2E373A_C202_4724_8EBC_F3ABD9EED24F__INCLUDED_)
#define AFX_CONNECTVIEW_H__FF2E373A_C202_4724_8EBC_F3ABD9EED24F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectView.h : header file
//

//include
#include "MainFrm.h"
#include "layer_mainchain.h"
#include "layer_device.h"

/////////////////////////////////////////////////////////////////////////////
// CConnectView view
class CConnectView : public CScrollView
{
protected:
	CConnectView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CConnectView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CConnectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CConnectView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTVIEW_H__FF2E373A_C202_4724_8EBC_F3ABD9EED24F__INCLUDED_)
