#pragma once
#include "vebancoDoc.h"

class CvebancoView : public CView
{
protected:
    CvebancoView();
    DECLARE_DYNCREATE(CvebancoView)
public:
    CvebancoDoc* GetDocument() const;
    virtual ~CvebancoView();
    virtual void OnDraw(CDC* dc);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* message);
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
protected:
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* sender, LPARAM hint, CObject* object);
    virtual BOOL OnPreparePrinting(CPrintInfo* info);
    virtual void OnBeginPrinting(CDC* dc, CPrintInfo* info);
    virtual void OnEndPrinting(CDC* dc, CPrintInfo* info);

    afx_msg void OnLButtonDown(UINT flags, CPoint point);
    afx_msg void OnSize(UINT type, int width, int height);
    afx_msg BOOL OnEraseBkgnd(CDC* dc);
    afx_msg void OnGameCommand(UINT command);
    afx_msg void OnUpdateGameCommand(CCmdUI* ui);
    afx_msg void OnUndo();
    afx_msg void OnUpdateUndo(CCmdUI* ui);
    DECLARE_MESSAGE_MAP()
private:
    caro::Move m_hint;
    bool m_menuInstalled;
    void PaintBoard(CDC* dc, const CRect& bounds);
};

#ifndef _DEBUG
inline CvebancoDoc* CvebancoView::GetDocument() const
{ return static_cast<CvebancoDoc*>(m_pDocument); }
#endif
