// BlockchainMfcDoc.h : interface of the CBlockchainMfcDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BLOCKCHAINMFCDOC_H__27195901_EECC_487C_9DCA_83B70BE7533A__INCLUDED_)
#define AFX_BLOCKCHAINMFCDOC_H__27195901_EECC_487C_9DCA_83B70BE7533A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBlockchainMfcDoc : public CDocument
{
protected: // create from serialization only
	CBlockchainMfcDoc();
	DECLARE_DYNCREATE(CBlockchainMfcDoc)
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlockchainMfcDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBlockchainMfcDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
// Generated message map functions
protected:
	//{{AFX_MSG(CBlockchainMfcDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCKCHAINMFCDOC_H__27195901_EECC_487C_9DCA_83B70BE7533A__INCLUDED_)
