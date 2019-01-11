// BlockchainMfcView.h : interface of the CBlockchainMfcView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BLOCKCHAINMFCVIEW_H__C8882091_1FBB_44DE_872F_35BC53E718DA__INCLUDED_)
#define AFX_BLOCKCHAINMFCVIEW_H__C8882091_1FBB_44DE_872F_35BC53E718DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CBlockchainMfcView : public CView
{
protected: // create from serialization only
	CBlockchainMfcView();
	DECLARE_DYNCREATE(CBlockchainMfcView)
public:
	CBlockchainMfcDoc* GetDocument();
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlockchainMfcView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBlockchainMfcView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBlockchainMfcView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in BlockchainMfcView.cpp
inline CBlockchainMfcDoc* CBlockchainMfcView::GetDocument()
   { return (CBlockchainMfcDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCKCHAINMFCVIEW_H__C8882091_1FBB_44DE_872F_35BC53E718DA__INCLUDED_)
