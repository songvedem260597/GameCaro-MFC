#pragma once
#include "CaroGame.h"

// Game state belongs to the document so repainting never loses moves and
// File/New, File/Open and File/Save all operate on the actual position.
class CvebancoDoc : public CDocument
{
protected:
    CvebancoDoc();
    DECLARE_DYNCREATE(CvebancoDoc)
public:
    caro::Game m_game;
    bool m_computer;

    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    BOOL StartGame(int size, bool computer);
    void PositionChanged();
    virtual ~CvebancoDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
#ifdef SHARED_HANDLERS
    virtual void InitializeSearchContent();
    virtual void OnDrawThumbnail(CDC& dc, LPRECT bounds);
#endif
protected:
    DECLARE_MESSAGE_MAP()
#ifdef SHARED_HANDLERS
    void SetSearchContent(const CString& value);
#endif
};
