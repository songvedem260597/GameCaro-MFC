#include "stdafx.h"
#ifndef SHARED_HANDLERS
#include "vebanco.h"
#endif
#include "vebancoDoc.h"
#include "vebancoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
enum {
    ID_CARO_NEW = 0x9000, ID_CARO_UNDO, ID_CARO_HINT,
    ID_CARO_COMPUTER, ID_CARO_TWO_PLAYERS,
    ID_CARO_15, ID_CARO_20, ID_CARO_30
};
}

IMPLEMENT_DYNCREATE(CvebancoView, CView)
BEGIN_MESSAGE_MAP(CvebancoView, CView)
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
    ON_COMMAND_RANGE(ID_CARO_NEW, ID_CARO_30, &CvebancoView::OnGameCommand)
    ON_UPDATE_COMMAND_UI_RANGE(ID_CARO_NEW, ID_CARO_30, &CvebancoView::OnUpdateGameCommand)
    ON_COMMAND(ID_EDIT_UNDO, &CvebancoView::OnUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CvebancoView::OnUpdateUndo)
    ON_WM_LBUTTONDOWN()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CvebancoView::CvebancoView() : m_menuInstalled(false) {}
CvebancoView::~CvebancoView() {}

BOOL CvebancoView::PreCreateWindow(CREATESTRUCT& cs)
{ return CView::PreCreateWindow(cs); }

void CvebancoView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
    CFrameWnd* frame = GetParentFrame();
    CMenu* menu = frame != NULL ? frame->GetMenu() : NULL;
    if (menu != NULL && !m_menuInstalled) {
        CMenu gameMenu;
        if (!gameMenu.CreatePopupMenu()) return;
        gameMenu.AppendMenu(MF_STRING, ID_CARO_NEW, _T("Van &moi\tF2"));
        gameMenu.AppendMenu(MF_STRING, ID_CARO_UNDO, _T("&Hoan tac\tCtrl+Z"));
        gameMenu.AppendMenu(MF_STRING, ID_CARO_HINT, _T("&Goi y\tH"));
        gameMenu.AppendMenu(MF_SEPARATOR);
        gameMenu.AppendMenu(MF_STRING, ID_CARO_COMPUTER, _T("Choi voi ma&y (ban: X)"));
        gameMenu.AppendMenu(MF_STRING, ID_CARO_TWO_PLAYERS, _T("&2 nguoi choi"));
        gameMenu.AppendMenu(MF_SEPARATOR);
        gameMenu.AppendMenu(MF_STRING, ID_CARO_15, _T("Ban co &15 x 15"));
        gameMenu.AppendMenu(MF_STRING, ID_CARO_20, _T("Ban co &20 x 20"));
        gameMenu.AppendMenu(MF_STRING, ID_CARO_30, _T("Ban co &30 x 30"));
        if (menu->AppendMenu(MF_POPUP,
                reinterpret_cast<UINT_PTR>(gameMenu.GetSafeHmenu()), _T("&Caro"))) {
            gameMenu.Detach(); // parent menu now owns the submenu
            m_menuInstalled = true;
            frame->DrawMenuBar();
        }
    }
}

void CvebancoView::OnUpdate(CView* /*sender*/, LPARAM /*hint*/, CObject* /*object*/)
{
    m_hint = caro::Move();
    CvebancoDoc* doc = GetDocument();
    if (doc != NULL && doc->m_computer && !doc->m_game.Finished() &&
        doc->m_game.Turn() == caro::O) {
        // Small, bounded tactical evaluation; no worker touching MFC objects.
        const caro::Move move = doc->m_game.SuggestMove();
        if (doc->m_game.Play(move.row, move.col)) doc->SetModifiedFlag(TRUE);
    }
    Invalidate(FALSE);
}

void CvebancoView::OnDraw(CDC* dc)
{
    CvebancoDoc* doc = GetDocument();
    ASSERT_VALID(doc);
    if (doc == NULL || dc == NULL) return;
    CRect bounds;
    if (dc->IsPrinting()) {
        bounds.SetRect(0, 0, dc->GetDeviceCaps(HORZRES), dc->GetDeviceCaps(VERTRES));
        PaintBoard(dc, bounds);
        return;
    }
    GetClientRect(&bounds);
    if (bounds.IsRectEmpty()) return;
    CDC backBuffer;
    CBitmap bitmap;
    if (backBuffer.CreateCompatibleDC(dc) &&
        bitmap.CreateCompatibleBitmap(dc, bounds.Width(), bounds.Height())) {
        CBitmap* previous = backBuffer.SelectObject(&bitmap);
        PaintBoard(&backBuffer, bounds);
        dc->BitBlt(0, 0, bounds.Width(), bounds.Height(), &backBuffer, 0, 0, SRCCOPY);
        backBuffer.SelectObject(previous);
    } else {
        PaintBoard(dc, bounds);
    }
}

void CvebancoView::PaintBoard(CDC* dc, const CRect& bounds)
{
    const caro::Game& game = GetDocument()->m_game;
    const caro::BoardLayout layout(bounds.Width(), bounds.Height(), game.Size());
    const int saved = dc->SaveDC();
    if (saved == 0) return;
    dc->FillSolidRect(bounds, RGB(24, 32, 46));
    dc->SetBkMode(TRANSPARENT);
    dc->SetTextColor(RGB(236, 241, 248));
    CFont titleFont, bodyFont;
    titleFont.CreatePointFont(170, _T("Segoe UI"), dc);
    bodyFont.CreatePointFont(100, _T("Segoe UI"), dc);
    if (titleFont.GetSafeHandle()) dc->SelectObject(&titleFont);
    dc->TextOut(20, 10, _T("CARO  /  FIVE IN A ROW"));
    if (bodyFont.GetSafeHandle()) dc->SelectObject(&bodyFont);
    const TCHAR* state = game.Winner() == caro::X ? _T("X thang!") :
        game.Winner() == caro::O ? _T("O thang!") :
        game.Draw() ? _T("Hoa - ban co da day") :
        game.Turn() == caro::X ? _T("Luot X") : _T("Luot O");
    CString status;
    status.Format(_T("%s   |   %s   |   %d nuoc   |   %d x %d"), state,
        GetDocument()->m_computer ? _T("Ban: X / May: O") : _T("2 nguoi choi"),
        game.MoveCount(), game.Size(), game.Size());
    dc->TextOut(20, 42, status);

    int penWidth = layout.cell / 10;
    if (penWidth < 2) penWidth = 2;
    CPen gridPen(PS_SOLID, 1, RGB(188, 179, 158));
    CPen xPen(PS_SOLID, penWidth, RGB(198, 57, 64));
    CPen oPen(PS_SOLID, penWidth, RGB(29, 100, 175));
    CPen winPen(PS_SOLID, penWidth + 1, RGB(36, 141, 92));
    if (layout.cell > 0) {
        const int side = layout.size * layout.cell;
        dc->FillSolidRect(layout.left, layout.top, side, side, RGB(250, 246, 235));
        for (int r = 0; r < game.Size(); ++r) {
            for (int c = 0; c < game.Size(); ++c) {
                const caro::Move move(r, c);
                COLORREF fill = RGB(250, 246, 235);
                if (!game.History().empty() && move == game.History().back())
                    fill = RGB(211, 228, 246);
                if (move == m_hint) fill = RGB(255, 223, 124);
                for (std::size_t i = 0; i < game.WinningLine().size(); ++i)
                    if (move == game.WinningLine()[i]) fill = RGB(192, 235, 201);
                dc->FillSolidRect(layout.left + c * layout.cell + 1,
                    layout.top + r * layout.cell + 1,
                    layout.cell - 1, layout.cell - 1, fill);
            }
        }
        dc->SelectObject(&gridPen);
        for (int i = 0; i <= game.Size(); ++i) {
            dc->MoveTo(layout.left + i * layout.cell, layout.top);
            dc->LineTo(layout.left + i * layout.cell, layout.top + side);
            dc->MoveTo(layout.left, layout.top + i * layout.cell);
            dc->LineTo(layout.left + side, layout.top + i * layout.cell);
        }
        dc->SelectStockObject(NULL_BRUSH);
        const int inset = layout.cell / 4;
        for (int r = 0; r < game.Size(); ++r) {
            for (int c = 0; c < game.Size(); ++c) {
                const int x = layout.left + c * layout.cell;
                const int y = layout.top + r * layout.cell;
                if (game.At(r, c) == caro::X) {
                    dc->SelectObject(&xPen);
                    dc->MoveTo(x + inset, y + inset);
                    dc->LineTo(x + layout.cell - inset, y + layout.cell - inset);
                    dc->MoveTo(x + layout.cell - inset, y + inset);
                    dc->LineTo(x + inset, y + layout.cell - inset);
                } else if (game.At(r, c) == caro::O) {
                    dc->SelectObject(&oPen);
                    dc->Ellipse(x + inset, y + inset,
                        x + layout.cell - inset, y + layout.cell - inset);
                }
            }
        }
        if (!game.WinningLine().empty()) {
            const caro::Move first = game.WinningLine().front();
            const caro::Move last = game.WinningLine().back();
            dc->SelectObject(&winPen);
            dc->MoveTo(layout.left + first.col * layout.cell + layout.cell / 2,
                       layout.top + first.row * layout.cell + layout.cell / 2);
            dc->LineTo(layout.left + last.col * layout.cell + layout.cell / 2,
                       layout.top + last.row * layout.cell + layout.cell / 2);
        }
    } else {
        CRect message(20, 75, bounds.right - 20, bounds.bottom - 65);
        CString text = _T("Hay phong to cua so de hien thi ban co.");
        dc->DrawText(text, &message, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    dc->SetTextColor(RGB(177, 190, 208));
    dc->TextOut(20, bounds.bottom - 56,
        _T("F2: van moi   |   Ctrl+Z: hoan tac   |   H: goi y   |   Menu Caro: che do / kich thuoc"));
    dc->TextOut(20, bounds.bottom - 32,
        _T("5 quan lien tiep tro len la thang. Ctrl+S: luu van   |   Ctrl+O: mo van"));
    // Restore all selected GDI objects before their stack destructors run.
    dc->RestoreDC(saved);
}

void CvebancoView::OnLButtonDown(UINT flags, CPoint point)
{
    SetFocus();
    CRect bounds;
    GetClientRect(&bounds);
    caro::Move move;
    CvebancoDoc* doc = GetDocument();
    const caro::BoardLayout layout(bounds.Width(), bounds.Height(), doc->m_game.Size());
    if (layout.Hit(point.x, point.y, move) &&
        (!doc->m_computer || doc->m_game.Turn() == caro::X) &&
        doc->m_game.Play(move.row, move.col)) doc->PositionChanged();
    CView::OnLButtonDown(flags, point);
}

void CvebancoView::OnSize(UINT type, int width, int height)
{
    CView::OnSize(type, width, height);
    Invalidate(FALSE);
}

BOOL CvebancoView::OnEraseBkgnd(CDC* /*dc*/) { return TRUE; }

void CvebancoView::OnUndo()
{
    CvebancoDoc* doc = GetDocument();
    if (doc->m_game.UndoTurn(doc->m_computer)) doc->PositionChanged();
}

void CvebancoView::OnUpdateUndo(CCmdUI* ui)
{ ui->Enable(GetDocument()->m_game.MoveCount() > 0); }

void CvebancoView::OnGameCommand(UINT command)
{
    CvebancoDoc* doc = GetDocument();
    switch (command) {
    case ID_CARO_NEW: doc->StartGame(doc->m_game.Size(), doc->m_computer); break;
    case ID_CARO_UNDO: OnUndo(); break;
    case ID_CARO_HINT:
        m_hint = doc->m_game.SuggestMove(); Invalidate(FALSE); break;
    case ID_CARO_COMPUTER: doc->StartGame(doc->m_game.Size(), true); break;
    case ID_CARO_TWO_PLAYERS: doc->StartGame(doc->m_game.Size(), false); break;
    case ID_CARO_15: doc->StartGame(15, doc->m_computer); break;
    case ID_CARO_20: doc->StartGame(20, doc->m_computer); break;
    case ID_CARO_30: doc->StartGame(30, doc->m_computer); break;
    }
}

void CvebancoView::OnUpdateGameCommand(CCmdUI* ui)
{
    const CvebancoDoc* doc = GetDocument();
    ui->Enable(TRUE);
    if (ui->m_nID == ID_CARO_UNDO) ui->Enable(doc->m_game.MoveCount() > 0);
    else if (ui->m_nID == ID_CARO_HINT) ui->Enable(!doc->m_game.Finished());
    else if (ui->m_nID == ID_CARO_COMPUTER) ui->SetRadio(doc->m_computer);
    else if (ui->m_nID == ID_CARO_TWO_PLAYERS) ui->SetRadio(!doc->m_computer);
    else if (ui->m_nID >= ID_CARO_15 && ui->m_nID <= ID_CARO_30) {
        const int size = ui->m_nID == ID_CARO_15 ? 15 : ui->m_nID == ID_CARO_20 ? 20 : 30;
        ui->SetRadio(doc->m_game.Size() == size);
    }
}

BOOL CvebancoView::PreTranslateMessage(MSG* message)
{
    if (message->message == WM_KEYDOWN) {
        const bool control = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
        const bool alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
        if (message->wParam == VK_F2) { OnGameCommand(ID_CARO_NEW); return TRUE; }
        if (control && message->wParam == 'Z') { OnUndo(); return TRUE; }
        if (!control && !alt && message->wParam == 'H') {
            OnGameCommand(ID_CARO_HINT); return TRUE;
        }
    }
    return CView::PreTranslateMessage(message);
}

BOOL CvebancoView::OnPreparePrinting(CPrintInfo* info) { return DoPreparePrinting(info); }
void CvebancoView::OnBeginPrinting(CDC*, CPrintInfo*) {}
void CvebancoView::OnEndPrinting(CDC*, CPrintInfo*) {}
#ifdef _DEBUG
void CvebancoView::AssertValid() const { CView::AssertValid(); }
void CvebancoView::Dump(CDumpContext& dc) const { CView::Dump(dc); }
CvebancoDoc* CvebancoView::GetDocument() const
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CvebancoDoc)));
    return static_cast<CvebancoDoc*>(m_pDocument);
}
#endif
