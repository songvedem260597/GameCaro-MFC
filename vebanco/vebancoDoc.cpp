#include "stdafx.h"
#ifndef SHARED_HANDLERS
#include "vebanco.h"
#endif
#include "vebancoDoc.h"
#include <sstream>
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CvebancoDoc, CDocument)
BEGIN_MESSAGE_MAP(CvebancoDoc, CDocument)
END_MESSAGE_MAP()

CvebancoDoc::CvebancoDoc() : m_computer(true) {}
CvebancoDoc::~CvebancoDoc() {}

BOOL CvebancoDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument()) return FALSE;
    m_game.Reset(m_game.Size());
    return TRUE;
}

BOOL CvebancoDoc::StartGame(int size, bool computer)
{
    // Do not silently discard an unfinished or previously loaded game.
    if (!SaveModified() || !OnNewDocument()) return FALSE;
    m_game.Reset(size);
    m_computer = computer;
    UpdateAllViews(NULL);
    return TRUE;
}

void CvebancoDoc::PositionChanged()
{
    SetModifiedFlag(TRUE);
    UpdateAllViews(NULL);
}

void CvebancoDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring()) {
        std::ostringstream out;
        if (!m_game.Save(out, m_computer))
            AfxThrowArchiveException(CArchiveException::genericException);
        const std::string text = out.str();
        ar.Write(text.data(), static_cast<UINT>(text.size()));
    } else {
        // Even a full 30x30 game fits in 16 KiB. Cap input before parsing.
        std::string text;
        char buffer[1024];
        UINT count = 0;
        while ((count = ar.Read(buffer, sizeof(buffer))) != 0) {
            if (text.size() + count > 16384)
                AfxThrowArchiveException(CArchiveException::badSchema);
            text.append(buffer, count);
        }
        std::istringstream in(text);
        if (!m_game.Load(in, m_computer))
            AfxThrowArchiveException(CArchiveException::badSchema);
    }
}

#ifdef SHARED_HANDLERS
void CvebancoDoc::OnDrawThumbnail(CDC& dc, LPRECT bounds)
{
    dc.FillSolidRect(bounds, RGB(245, 241, 230));
    CString text = _T("CARO - Five in a row");
    dc.DrawText(text, bounds, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CvebancoDoc::InitializeSearchContent()
{
    SetSearchContent(_T("Caro;Gomoku;five in a row;board game"));
}

void CvebancoDoc::SetSearchContent(const CString& value)
{
    if (value.IsEmpty()) {
        RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
    } else {
        CMFCFilterChunkValueImpl* chunk = NULL;
        ATLTRY(chunk = new CMFCFilterChunkValueImpl);
        if (chunk != NULL) {
            chunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
            SetChunkValue(chunk);
        }
    }
}
#endif

#ifdef _DEBUG
void CvebancoDoc::AssertValid() const { CDocument::AssertValid(); }
void CvebancoDoc::Dump(CDumpContext& dc) const { CDocument::Dump(dc); }
#endif
