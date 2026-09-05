#include "stdafx.h"
#include "oco.h"

void oco::chucnangmoi()
{
	//chuc nang moi
}
void oco::setup(int mx1,int my1,int mx2,int my2)
{
	x1=mx1;y1=my1;x2=mx2;y2=my2;
}
void oco::draw(CClientDC *pDC)
{
	pDC->Rectangle(x1,y1,x2,y2);
}
void oco::drawX(CClientDC *pDC)
{
	pDC->MoveTo(x1 + 5, y1 + 5);
	pDC->LineTo(x2 - 5, y2 - 5);
	pDC->MoveTo(x2 - 5, y1 + 5);
	pDC->LineTo(x1 + 5, y2 - 5);
}
void oco::drawO(CClientDC *pDC)
{
	pDC->Ellipse(x1+5, y1+5, x2-5, y2-5);
}
int oco::boxcheck(CPoint p1)
{
	if (p1.x >= x1 && p1.x <= x2 && p1.y >= y1 && p1.y <= y2)
		return 1;
	else
		return 0;
}

int oco::checkToWin(int arr[][max], int I, int J)
{
	const int row = 30;
	const int column = 30;
	const int value = arr[I][J];
	if (value == 0)
		return 0;

	const int directions[4][2] =
	{
		{ 1, 0 },
		{ 0, 1 },
		{ 1, 1 },
		{ 1, -1 }
	};

	for (int d = 0; d < 4; ++d)
	{
		const int di = directions[d][0];
		const int dj = directions[d][1];
		int count = 1;

		int i = I + di;
		int j = J + dj;
		while (i >= 0 && i < row && j >= 0 && j < column && arr[i][j] == value)
		{
			++count;
			i += di;
			j += dj;
		}

		i = I - di;
		j = J - dj;
		while (i >= 0 && i < row && j >= 0 && j < column && arr[i][j] == value)
		{
			++count;
			i -= di;
			j -= dj;
		}

		if (count >= 5)
			return 1;
	}

	return 0;
}

int oco::getA()
{
	return(x2 - x1) / 2;
}
int oco::getC()
{
	return(y2 - y1) / 2;
}
oco::oco(void)
{

}
oco::~oco(void)
{

}
