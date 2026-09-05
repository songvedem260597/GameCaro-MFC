#include "stdafx.h"
#include "oco.h"

oco::oco() : x1(0), y1(0), x2(0), y2(0) {}
oco::~oco() {}
void oco::chucnangmoi() {}
void oco::setup(int left, int top, int right, int bottom)
{ x1 = left; y1 = top; x2 = right; y2 = bottom; }
void oco::draw(CDC* dc) { dc->Rectangle(x1, y1, x2, y2); }
void oco::drawX(CDC* dc)
{
    dc->MoveTo(x1 + 5, y1 + 5); dc->LineTo(x2 - 5, y2 - 5);
    dc->MoveTo(x2 - 5, y1 + 5); dc->LineTo(x1 + 5, y2 - 5);
}
void oco::drawO(CDC* dc) { dc->Ellipse(x1 + 5, y1 + 5, x2 - 5, y2 - 5); }
int oco::boxcheck(CPoint point)
{ return point.x >= x1 && point.x < x2 && point.y >= y1 && point.y < y2; }
int oco::getA() { return (x2 - x1) / 2; }
int oco::getC() { return (y2 - y1) / 2; }

int oco::checkToWin(const int board[][CaroLegacyCapacity], int row, int col)
{
    // This legacy helper retains its original 30x30 board contract.
    const int size = 30;
    if (row < 0 || col < 0 || row >= size || col >= size) return 0;
    const int player = board[row][col];
    if (player != 1 && player != 2) return 0;
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    for (int d = 0; d < 4; ++d) {
        int count = 1;
        for (int sign = -1; sign <= 1; sign += 2) {
            const int dr = directions[d][0] * sign, dc = directions[d][1] * sign;
            int r = row + dr, c = col + dc;
            while (r >= 0 && c >= 0 && r < size && c < size && board[r][c] == player) {
                ++count; r += dr; c += dc;
            }
        }
        if (count >= 5) return 1;
    }
    return 0;
}
