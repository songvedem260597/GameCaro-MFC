#pragma once

// Legacy drawing helper. The active UI uses CaroGame.h; keep this API safe for
// older callers without defining a global macro named "max".
enum { CaroLegacyCapacity = 100 };
class oco
{
private:
    int x1, y1, x2, y2;
public:
    void chucnangmoi();
    CPoint point1, point2;
    void setup(int left, int top, int right, int bottom);
    void draw(CDC* dc);
    void drawX(CDC* dc);
    void drawO(CDC* dc);
    int boxcheck(CPoint point);
    int checkToWin(const int board[][CaroLegacyCapacity], int row, int col);
    int getA();
    int getC();
    oco();
    ~oco();
};
