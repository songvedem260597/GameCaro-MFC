
// vebancoView.h : interface of the CvebancoView class
//

#pragma once
#include"oco.h"
#define max 100
class CvebancoView : public CView
{
protected: // create from serialization only
	CvebancoView();
	DECLARE_DYNCREATE(CvebancoView)

// Attributes
public:
	CvebancoDoc* GetDocument() const;

// Operations
public:
	oco oco[max][max];
	int arr[max][max];
	int condition;
	int size;
	int row, column;
	CPoint p1;
	int playercount;
	int winner;
	int lastRow;
	int lastColumn;

// Overrides
public:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CvebancoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void ResetGame();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG
inline CvebancoDoc* CvebancoView::GetDocument() const
   { return reinterpret_cast<CvebancoDoc*>(m_pDocument); }
#endif
