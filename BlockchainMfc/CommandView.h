#if !defined(AFX_COMMANDVIEW_H__04A7A69B_D43D_47A5_99BB_C9230DAF91BD__INCLUDED_)
#define AFX_COMMANDVIEW_H__04A7A69B_D43D_47A5_99BB_C9230DAF91BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommandView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCommandView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

//define
#define MAX_HEAVYNODE 10
#define MAX_LIGHTNODE 50
#define RANDOM_RANGE 10
#define RANDOM_STEP 1
#define RANDOM_ACCOUNT 8
#define MAX_TOKEN 100
#define MAX_DEAL 100
#define MIN_TOKEN 5
//include
#include "ConnectView.h"
#include "TransactionView.h"
#include "MainFrm.h"
#include "layer_mainchain.h"
#include "layer_device.h"

class CCommandView : public CFormView
{
protected:
	CCommandView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCommandView)

// Form Data
public:
	//{{AFX_DATA(CCommandView)
	enum { IDD = IDD_FORMVIEW };
	CButton	m_random;
	CButton	m_limit;
	CListCtrl	m_transaction;
	CListCtrl	m_device;
	//}}AFX_DATA
public:
	uint8 m_flag;//0-not use OnSize,1-use OnSize
public:
	CView *GetView(CRuntimeClass *pClass);
	void InitialVariable(void);
	void RandomVariable(void);
	uint8 InitialThread(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommandView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
protected:
	virtual ~CCommandView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CCommandView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonLimit();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonRandom();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMANDVIEW_H__04A7A69B_D43D_47A5_99BB_C9230DAF91BD__INCLUDED_)
