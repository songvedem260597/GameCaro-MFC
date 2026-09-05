
// vebancoView.cpp : implementation of the CvebancoView class
//

#include "stdafx.h"
#ifndef SHARED_HANDLERS
#include "vebanco.h"
#endif

#include "vebancoDoc.h"
#include "vebancoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	const COLORREF kBackground = RGB(240, 244, 248);
	const COLORREF kPanel = RGB(255, 255, 255);
	const COLORREF kBoard = RGB(250, 252, 255);
	const COLORREF kGrid = RGB(210, 218, 228);
	const COLORREF kGridStrong = RGB(170, 181, 196);
	const COLORREF kText = RGB(34, 42, 55);
	const COLORREF kMuted = RGB(111, 121, 137);
	const COLORREF kBlue = RGB(37, 99, 235);
	const COLORREF kRed = RGB(239, 68, 68);
	const COLORREF kGreen = RGB(16, 185, 129);
	const COLORREF kHighlight = RGB(219, 234, 254);
	const COLORREF kShadow = RGB(220, 226, 234);

	int IntMin(int a, int b)
	{
		return (a < b) ? a : b;
	}

	int IntMax(int a, int b)
	{
		return (a > b) ? a : b;
	}

	void GetLayout(CWnd* view, CRect& boardRect, CRect& sideRect, int& cellSize)
	{
		CRect client;
		view->GetClientRect(&client);

		const int margin = 28;
		const int gap = 28;
		const int sideWidth = 320;
		const int maxBoard = IntMin(client.Height() - margin * 2, client.Width() - sideWidth - gap - margin * 2);
		cellSize = IntMax(18, maxBoard / 30);
		const int boardPixels = cellSize * 30;

		int top = IntMax(margin, (client.Height() - boardPixels) / 2);
		int left = margin;
		boardRect = CRect(left, top, left + boardPixels, top + boardPixels);
		sideRect = CRect(boardRect.right + gap, top, IntMin(client.right - margin, boardRect.right + gap + sideWidth), top + boardPixels);
	}

	void FillRoundRect(CDC* pDC, const CRect& rect, COLORREF color, int radius)
	{
		CBrush brush(color);
		CPen pen(PS_SOLID, 1, color);
		CBrush* oldBrush = pDC->SelectObject(&brush);
		CPen* oldPen = pDC->SelectObject(&pen);
		pDC->RoundRect(rect, CPoint(radius, radius));
		pDC->SelectObject(oldPen);
		pDC->SelectObject(oldBrush);
	}
}

IMPLEMENT_DYNCREATE(CvebancoView, CView)

BEGIN_MESSAGE_MAP(CvebancoView, CView)
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

CvebancoView::CvebancoView()
{
	size = 30;
	row = 30;
	column = 30;
	ResetGame();
}

CvebancoView::~CvebancoView()
{
}

void CvebancoView::ResetGame()
{
	playercount = 0;
	condition = 1;
	winner = 0;
	lastRow = -1;
	lastColumn = -1;

	for (int i = 0; i < row; i++)
		for (int j = 0; j < column; j++)
			arr[i][j] = 0;

	if (GetSafeHwnd())
		Invalidate(FALSE);
}

BOOL CvebancoView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~WS_BORDER;
	return CView::PreCreateWindow(cs);
}

void CvebancoView::OnDraw(CDC* pDC)
{
	CvebancoDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect client;
	GetClientRect(&client);
	pDC->FillSolidRect(client, kBackground);
	pDC->SetBkMode(TRANSPARENT);

	CRect boardRect, sideRect;
	int cellSize = 0;
	GetLayout(this, boardRect, sideRect, cellSize);
	size = cellSize;

	CRect boardShadow = boardRect;
	boardShadow.OffsetRect(6, 8);
	FillRoundRect(pDC, boardShadow, kShadow, 18);
	FillRoundRect(pDC, boardRect, kBoard, 18);

	CRect panelShadow = sideRect;
	panelShadow.OffsetRect(6, 8);
	FillRoundRect(pDC, panelShadow, kShadow, 18);
	FillRoundRect(pDC, sideRect, kPanel, 18);

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			int left = boardRect.left + j * cellSize;
			int top = boardRect.top + i * cellSize;
			oco[i][j].setup(left, top, left + cellSize, top + cellSize);
		}
	}

	if (lastRow >= 0 && lastColumn >= 0)
	{
		CRect highlight(
			boardRect.left + lastColumn * cellSize + 1,
			boardRect.top + lastRow * cellSize + 1,
			boardRect.left + (lastColumn + 1) * cellSize,
			boardRect.top + (lastRow + 1) * cellSize);
		pDC->FillSolidRect(highlight, kHighlight);
	}

	for (int n = 0; n <= row; n++)
	{
		COLORREF lineColor = (n % 5 == 0) ? kGridStrong : kGrid;
		CPen pen(PS_SOLID, 1, lineColor);
		CPen* oldPen = pDC->SelectObject(&pen);
		int y = boardRect.top + n * cellSize;
		pDC->MoveTo(boardRect.left, y);
		pDC->LineTo(boardRect.right, y);
		pDC->SelectObject(oldPen);
	}

	for (int n = 0; n <= column; n++)
	{
		COLORREF lineColor = (n % 5 == 0) ? kGridStrong : kGrid;
		CPen pen(PS_SOLID, 1, lineColor);
		CPen* oldPen = pDC->SelectObject(&pen);
		int x = boardRect.left + n * cellSize;
		pDC->MoveTo(x, boardRect.top);
		pDC->LineTo(x, boardRect.bottom);
		pDC->SelectObject(oldPen);
	}

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			if (arr[i][j] == 0)
				continue;

			CRect cell(
				boardRect.left + j * cellSize,
				boardRect.top + i * cellSize,
				boardRect.left + (j + 1) * cellSize,
				boardRect.top + (i + 1) * cellSize);
			cell.DeflateRect(IntMax(4, cellSize / 5), IntMax(4, cellSize / 5));

			if (arr[i][j] == 1)
			{
				CPen pen(PS_SOLID, IntMax(2, cellSize / 8), kRed);
				CPen* oldPen = pDC->SelectObject(&pen);
				pDC->MoveTo(cell.left, cell.top);
				pDC->LineTo(cell.right, cell.bottom);
				pDC->MoveTo(cell.right, cell.top);
				pDC->LineTo(cell.left, cell.bottom);
				pDC->SelectObject(oldPen);
			}
			else
			{
				CPen pen(PS_SOLID, IntMax(2, cellSize / 8), kGreen);
				CPen* oldPen = pDC->SelectObject(&pen);
				CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
				pDC->Ellipse(cell);
				pDC->SelectObject(oldBrush);
				pDC->SelectObject(oldPen);
			}
		}
	}

	int x = sideRect.left + 28;
	int y = sideRect.top + 30;
	int contentWidth = IntMax(120, sideRect.Width() - 56);

	CFont titleFont;
	titleFont.CreatePointFont(250, _T("Segoe UI Semibold"));
	CFont labelFont;
	labelFont.CreatePointFont(105, _T("Segoe UI"));
	CFont valueFont;
	valueFont.CreatePointFont(135, _T("Segoe UI Semibold"));
	CFont buttonFont;
	buttonFont.CreatePointFont(110, _T("Segoe UI Semibold"));

	CFont* oldFont = pDC->SelectObject(&titleFont);
	pDC->SetTextColor(kText);
	pDC->TextOutW(x, y, _T("CARO"));
	y += 48;

	pDC->SelectObject(&labelFont);
	pDC->SetTextColor(kMuted);
	pDC->TextOutW(x, y, _T("Classic 5-in-a-row"));
	y += 62;

	CRect turnCard(x, y, x + contentWidth, y + 112);
	FillRoundRect(pDC, turnCard, RGB(247, 249, 252), 14);
	pDC->SetTextColor(kMuted);
	pDC->TextOutW(turnCard.left + 18, turnCard.top + 16, _T("STATUS"));

	pDC->SelectObject(&valueFont);
	CString status;
	COLORREF statusColor = kBlue;
	if (winner == 1)
	{
		status = _T("Player X wins");
		statusColor = kRed;
	}
	else if (winner == 2)
	{
		status = _T("Player O wins");
		statusColor = kGreen;
	}
	else if (playercount % 2 == 0)
	{
		status = _T("Player X turn");
		statusColor = kRed;
	}
	else
	{
		status = _T("Player O turn");
		statusColor = kGreen;
	}
	pDC->SetTextColor(statusColor);
	pDC->TextOutW(turnCard.left + 18, turnCard.top + 50, status);
	y += 138;

	pDC->SelectObject(&labelFont);
	pDC->SetTextColor(kMuted);
	pDC->TextOutW(x, y, _T("Players"));
	y += 34;

	CRect xCard(x, y, x + contentWidth, y + 56);
	FillRoundRect(pDC, xCard, RGB(254, 242, 242), 12);
	pDC->SetTextColor(kRed);
	pDC->SelectObject(&valueFont);
	pDC->TextOutW(xCard.left + 18, xCard.top + 15, _T("X   Red"));
	y += 68;

	CRect oCard(x, y, x + contentWidth, y + 56);
	FillRoundRect(pDC, oCard, RGB(236, 253, 245), 12);
	pDC->SetTextColor(kGreen);
	pDC->TextOutW(oCard.left + 18, oCard.top + 15, _T("O   Green"));
	y += 88;

	CRect newGameRect(x, y, x + contentWidth, y + 54);
	FillRoundRect(pDC, newGameRect, kBlue, 13);
	pDC->SelectObject(&buttonFont);
	pDC->SetTextColor(RGB(255, 255, 255));
	CString buttonText = _T("NEW GAME");
	CSize buttonSize = pDC->GetTextExtent(buttonText);
	pDC->TextOutW(newGameRect.left + (newGameRect.Width() - buttonSize.cx) / 2,
		newGameRect.top + (newGameRect.Height() - buttonSize.cy) / 2, buttonText);

	pDC->SelectObject(&labelFont);
	pDC->SetTextColor(kMuted);
	CString moves;
	moves.Format(_T("Moves played: %d"), playercount);
	pDC->TextOutW(x, sideRect.bottom - 44, moves);

	pDC->SelectObject(oldFont);
}

BOOL CvebancoView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void CvebancoView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CvebancoView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

#ifdef _DEBUG
void CvebancoView::AssertValid() const
{
	CView::AssertValid();
}

void CvebancoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CvebancoDoc* CvebancoView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CvebancoDoc)));
	return (CvebancoDoc*)m_pDocument;
}
#endif //_DEBUG

void CvebancoView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect boardRect, sideRect;
	int cellSize = 0;
	GetLayout(this, boardRect, sideRect, cellSize);

	int panelX = sideRect.left + 28;
	int panelY = sideRect.top + 30 + 48 + 62 + 138 + 34 + 68 + 88;
	CRect newGameRect(panelX, panelY, panelX + IntMax(120, sideRect.Width() - 56), panelY + 54);
	if (newGameRect.PtInRect(point))
	{
		ResetGame();
		CView::OnLButtonDown(nFlags, point);
		return;
	}

	if (!condition || !boardRect.PtInRect(point))
	{
		CView::OnLButtonDown(nFlags, point);
		return;
	}

	int col = (point.x - boardRect.left) / cellSize;
	int r = (point.y - boardRect.top) / cellSize;
	if (r < 0 || r >= row || col < 0 || col >= column || arr[r][col] != 0)
	{
		CView::OnLButtonDown(nFlags, point);
		return;
	}

	int currentPlayer = (playercount % 2 == 0) ? 1 : 2;
	arr[r][col] = currentPlayer;
	lastRow = r;
	lastColumn = col;
	playercount++;

	if (oco[r][col].checkToWin(arr, r, col) == 1)
	{
		winner = currentPlayer;
		condition = 0;
	}

	Invalidate(FALSE);
	CView::OnLButtonDown(nFlags, point);
}
